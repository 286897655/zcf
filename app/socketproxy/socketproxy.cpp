#include <iostream>
#include <asio.hpp>
#include <zlog/log.h>
#include <zcf/zcf_flags.hpp>
#include <zcf/net/zcf_net.hpp>

using io_worker = asio::executor_work_guard<asio::io_context::executor_type>;

class iocontextservice{
public:
iocontextservice(int pool_size):pool_size_(pool_size){

    for(size_t i = 0; i < pool_size_; i++){
        auto ioc = std::make_shared<asio::io_context>();
        auto work = asio::make_work_guard(*ioc);

        iows_.emplace_back(std::move(work));
        iocs_.emplace_back(std::move(ioc));
    }
    for(size_t i = 0; i < pool_size_; i++){
        ioc_threads_.emplace_back(std::make_shared<std::thread>([this,i](){
            // set thread name
            char buffer[16]={0};
            sprintf(buffer,"iocontext_%d",i);
            pthread_setname_np(pthread_self(), buffer);
            iocs_[i]->run();
        }));
    }
}

public:

std::shared_ptr<asio::io_context> NextIoContext(){
    static size_t now_io_context = 0;
    return iocs_.at((++now_io_context) % pool_size_);
}

private:
std::vector<std::shared_ptr<asio::io_context>> iocs_;
std::vector<io_worker> iows_;
std::vector<std::shared_ptr<std::thread>> ioc_threads_;
size_t pool_size_;
};

static iocontextservice* ioc_service = nullptr;
class proxypair{
public:
    proxypair(const std::shared_ptr<asio::io_context>& ioc,const std::string& ip,int port,int proxy_id)
        :ioc_(ioc),upstream_socket_(*ioc_),downstream_socket_(*ioc_),
            proxy_ip_(ip),proxy_port_(port),proxy_id_(proxy_id){

    }

    void setOnClose(std::function<void(int)>&& on_close){
        on_close_ = on_close;
    }

    asio::ip::tcp::socket& upstream_socket(){
        return upstream_socket_;
    }

    void start(){
        // handle async connect downstream
        asio::ip::tcp::endpoint downstream(asio::ip::address::from_string(proxy_ip_),proxy_port_);
        downstream_socket_.async_connect(downstream,[this](const asio::error_code& ec){
            if(!ec){
                zlog("!!!connect {}:{} success",proxy_ip_,proxy_port_);
                startproxy();
            }else{
                zlog("!!!connect {}:{} fail",proxy_ip_,proxy_port_);

                on_close_(proxy_id_);
            }
        });
    }

private:
    void startproxy(){
        // handle upstream_socket read
        readupstream();
        readdownstream();
    }

    void readdownstream(){
        downstream_socket_.async_read_some(asio::buffer(down_buffer_),[this](const asio::error_code& ec,std::size_t len){
            if(!ec){
                writeupstream(len);
            }else{
                zlog("!!!read downstream error:{}",ec.message());
                
                on_close_(proxy_id_);
            }
        });
        // asio::async_read(downstream_socket_,asio::buffer(down_buffer_),[this](const asio::error_code& ec,std::size_t len){
            
        // });
    };

    void readupstream(){
        upstream_socket_.async_read_some(asio::buffer(up_buffer_),[this](const asio::error_code& ec,std::size_t len){
            if(!ec){
                writedownstream(len);
            }else{
                zlog("!!!read upstream error:{}",ec.message());

                on_close_(proxy_id_);
            }
        });
        // asio::async_read(upstream_socket_,asio::buffer(up_buffer_),[this](const asio::error_code& ec,std::size_t len){
            
        // });
    }

    void writedownstream(int length){
        asio::async_write(downstream_socket_,asio::buffer(up_buffer_,length),[this,length](const asio::error_code& ec,std::size_t len){
            if(!ec){
                assert(length == len);
                // write end do read
                readupstream();
            }else{
                zlog("!!! async write downstream error:{}",ec.message());

                on_close_(proxy_id_);
            }
        });
    }

    void writeupstream(int length){
        asio::async_write(upstream_socket_,asio::buffer(down_buffer_,length),[this,length](const asio::error_code& ec,std::size_t len){
            if(!ec){
                assert(length == len);
                // write end and do read
                readdownstream();
            }else{
                zlog("!!! async write upstream error:{}",ec.message());

                on_close_(proxy_id_);
            }
        });
    };

private:
    std::string proxy_ip_;
    int proxy_port_;
    int proxy_id_;
    std::shared_ptr<asio::io_context> ioc_;
    asio::ip::tcp::socket upstream_socket_;
    asio::ip::tcp::socket downstream_socket_;
    std::array<uint8_t,4096> up_buffer_;
    std::array<uint8_t,4096> down_buffer_;
    std::function<void(int)> on_close_;
};


class proxy_server{
public:
proxy_server(asio::io_context& io_context,int port,const std::string& proxy_ip,int proxy_port)
    :acceptor_(io_context,asio::ip::tcp::endpoint(asio::ip::tcp::v6(), port)),
        proxy_ip_(proxy_ip),proxy_port_(proxy_port)
{
    do_accept();
}

private:
void do_accept(){
    std::shared_ptr<asio::io_context> next_ioc = ioc_service->NextIoContext();
    static int proxy_count = 1;
    proxypair* pair = new proxypair(next_ioc,proxy_ip_,proxy_port_,proxy_count);
    proxy_socket_pairs_.insert(std::make_pair(proxy_count,pair));
    pair->setOnClose([this](int proxy_id){
        auto proxy_pair = proxy_socket_pairs_.find(proxy_id);
        Z_ASSERT(proxy_pair != proxy_socket_pairs_.end());
        delete proxy_pair->second;
        proxy_pair->second = nullptr;
        proxy_socket_pairs_.erase(proxy_pair);
    });
    proxy_count++;

    acceptor_.async_accept(pair->upstream_socket(),[this,pair](std::error_code ec){
        if(!ec){
            // no error
            std::cout << "got connect,peer:"<<pair->upstream_socket().remote_endpoint().address() << std::endl;
            pair->start();
        }
        do_accept();
    });
}

private:
asio::ip::tcp::acceptor acceptor_;
std::unordered_map<int,proxypair*> proxy_socket_pairs_;
std::string proxy_ip_;
int proxy_port_;
};


int main(int argc,char** argv){
    zlog::logger::create_defaultLogger();
    zcf::OptionParser option_parser("socketproxy argument:");
    auto option_help = option_parser.add<zcf::Switch>("h","help","print socketproxy help");
    auto option_port = option_parser.add<zcf::Value<int>>("l","listen","proxy listen of port");
    auto option_proxy = option_parser.add<zcf::Value<std::string>>("p","proxy","proxy address");

    option_parser.parse(argc,argv);
    if(option_help->is_set()){
        std::cout << option_parser << std::endl;
        return 0;
    }
    if(!option_port->is_set()){
        zlog("socketproxy has no listen port");
        return 0;
    }
    if(!option_proxy->is_set()){
        zlog("socketproxy has no proxy address");
        return 0;
    }

    int listen_port = option_port->value();
    std::string proxy_addr = option_proxy->value();
    auto parsed_addr = zcf::socket::parse_ip_colon_port(proxy_addr);
    zlog("socketproxy will listen at {},and proxy to {}:{}",listen_port,parsed_addr.first,parsed_addr.second);
    ioc_service = new iocontextservice(std::thread::hardware_concurrency() >> 1);
    asio::io_context main_iocontext;
    proxy_server server(main_iocontext,listen_port,parsed_addr.first,parsed_addr.second);
    main_iocontext.run();
    return 0;
}
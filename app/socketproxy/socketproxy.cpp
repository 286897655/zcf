#include <iostream>
#include <asio.hpp>

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
    proxypair(const std::shared_ptr<asio::io_context>& ioc):ioc_(ioc),upstream_socket_(*ioc_),downstream_socket_(*ioc_){

    }

    asio::ip::tcp::socket& upstream_socket(){
        return upstream_socket_;
    }

    void start(){
        // handle async connect downstream
        asio::ip::tcp::endpoint downstream(asio::ip::address::from_string("127.0.0.1"),8020);
        downstream_socket_.async_connect(downstream,[this](const asio::error_code& ec){
            if(!ec){
                std::cout<< "!!!connect 127.0.0.1:8020 success"<<std::endl;
                startproxy();
            }else{
                std::cout<< "!!!connect 127.0.0.1:8020 fail"<<std::endl;
            }
        });
    }

private:
    void startproxy(){
        // handle upstream_socket read
        readupstream();
    }
    void readupstream(){
        asio::async_read(upstream_socket_,asio::buffer(read_buffer_),[this](const asio::error_code& ec,std::size_t len){
            if(!ec){
                writedownstream(len);
            }else{
                std::cout << "!!!read error:" << ec.message() <<std::endl;
            }
        });
    }

    void writedownstream(int length){
        asio::async_write(downstream_socket_,asio::buffer(read_buffer_,length),[this,length](const asio::error_code& ec,std::size_t len){
            if(!ec){
                assert(length == len);
                // write end do read
                readupstream();
            }else{
                std::cout << "!!! async write error" << std::endl;
            }
        });
    }

private:
    std::shared_ptr<asio::io_context> ioc_;
    asio::ip::tcp::socket upstream_socket_;
    asio::ip::tcp::socket downstream_socket_;
    std::array<uint8_t,4096> read_buffer_;
};


class server{
public:
server(asio::io_context& io_context,int port)
    :acceptor_(io_context,asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 9020))
{
    do_accept();
}

private:
void do_accept(){
    std::shared_ptr<asio::io_context> next_ioc = ioc_service->NextIoContext();
    proxypair* pair = new proxypair(next_ioc);
    proxy_socket_pairs_.emplace_back(pair);

    acceptor_.async_accept(pair->upstream_socket(),[this,pair](std::error_code ec){
        if(!ec){
            // no error
            std::cout << "got connect,peer:"<<pair->upstream_socket().remote_endpoint().address() << std::endl;
            pair->start();
        }
        do_accept();
    });
}

asio::ip::tcp::acceptor acceptor_;
std::vector<proxypair*> proxy_socket_pairs_;
};


int main(int argc,char** argv){
    ioc_service = new iocontextservice(4);
    asio::io_context main_iocontext;
    server server(main_iocontext,9020);
    main_iocontext.run();
    return 0;
}
#include <zcf/fw/framework.hpp>
#include <zcf/log/zcf_log.h>

class ChannelEvent : public std::enable_shared_from_this<ChannelEvent>{
public:
    std::string ID;
    int Event;
};

class Service : public fw::EventListener<ChannelEvent>{

private:
    void OnEvent(ChannelEvent&& event) override{
        zlog("Service On Event:{}--->{}",event.ID,event.Event);
    }
};

class Channel : public fw::EventDispatch<ChannelEvent>{
public:
    void DoEvent(){
        ChannelEvent event;
        event.ID = "test";
        event.Event = 2;
        postEventW(std::move(event));
    }

};

int main(int argc,char** argv){
    std::shared_ptr<Service> service = std::make_shared<Service>();
    Channel channel;
    channel.RegisterW(service);
    channel.DoEvent();
};
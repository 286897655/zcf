#include <string>

struct Http
{
    int port;
};
struct Rtsp
{
    int port;
};
struct Golbal
{
    std::string log;
    std::string conf;
};

template<typename ...Base>
struct AConfig : public Base...{};

template<typename ...Base>
struct BConfig : public Base...{};

int main(int argc,char** argv){
    AConfig<Http,Rtsp> aconfig;
    BConfig<Rtsp,Golbal> bconfig;
    

}
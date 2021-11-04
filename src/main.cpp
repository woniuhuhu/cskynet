#include "Sunnet.h"
#include <unistd.h>
void TestSocketCtrl()
{
    int fd = Sunnet::inst->Listen(8001, 1);
    usleep(15 * 100000);
    Sunnet::inst->CloseConn(fd);
}
void TestEcho()
{
    auto t = make_shared<string>("gateway");
    uint32_t geteway = Sunnet::inst->NewService(t);
}
void test()
{
    auto pingType = make_shared<string>("ping");
    uint32_t ping1 = Sunnet::inst->NewService(pingType);
    uint32_t ping2 = Sunnet::inst->NewService(pingType);
    uint32_t pong = Sunnet::inst->NewService(pingType);
    auto msg1 = Sunnet::inst->MakeMsg(ping1, new char[3]{'h', 'i', '\0'}, 3);
    auto msg2 = Sunnet::inst->MakeMsg(ping2, new char[6]{'h', 'e', 'l', 'l', 'o', '\0'}, 6);
    Sunnet::inst->Send(pong, msg1);
    Sunnet::inst->Send(pong, msg2);
}
int main()
{
    new Sunnet();
    Sunnet::inst->Start();
    //启动main服务
    auto t = make_shared<string>("main");
	Sunnet::inst->NewService(t);

    //TestEcho();
    //TestSocketCtrl();
    // test();
    //开启系统后的一些逻辑
    Sunnet::inst->Wait();
    return 0;
}

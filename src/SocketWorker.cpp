#include <assert.h>
#include "SocketWorker.h"
#include <iostream>
#include <unistd.h>
//初始化
void SocketWorker::Init()
{
    cout << "SocketWorker Init" << endl;
    //创建epoll对象
    epollFD = epoll_create(520); //返回值：非负数表示成功创建的epoll对象的描述符，-1
    assert(epollFd>0);
}
void SocketWorker::operator()()
{
    while (true)
    {
        cout << "SocketWorker working" << endl;
        usleep(1000);
    }
}
#include <assert.h>
#include "SocketWorker.h"
#include <iostream>
#include <unistd.h>
#include <string.h>
//初始化
void SocketWorker::Init()
{
    cout << "SocketWorker Init" << endl;
    //创建epoll对象
    epollFd = epoll_create(520); //返回值：非负数表示成功创建的epoll对象的描述符，-1
    assert(epollFd>0);
}
void SocketWorker::operator()()
{
    while (true)
    {
        //阻塞等待
        const int EVENT_SIZE = 64;
        struct epoll_event events[EVENT_SIZE];
        int eventCount = epoll_wait(epollFd, events, EVENT_SIZE, -1);
        //取得事件
        for (int i = 0; i < eventCount;i++){
            epoll_event ev = events[i];
            OnEvent(ev);
        }
    }
}
void SocketWorker::OnEvent(epoll_event ev){
    cout << "Onevent" << endl;
}
//注意跨线程调用
void SocketWorker::AddEvent(int fd){
    cout << "AddEvent fd " << fd << endl;
    //添加到epoll对象
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = fd;
    //第一个参数代表要操作的epoll对象，第二个代表要执行的操作，第三个代表要操作的文件描述符
    //第四个参数是epoll_event的结构体，它分为events和data两部分，主要用于控制监听的行为。event包括“监听什么”和“触发模式”
    //两种属性。EPOLLIN（监听可读事件）|EPOLLET(边缘触发模式)|EPOLLOUT(监听可写事件)
    if(epoll_ctl(epollFd,EPOLL_CTL_ADD,fd,&ev)==-1){   //epoll_ctl是修改epoll对象监听列表的方法
        cout << "AddEvent epoll_ctl Fail:" << strerror(errno) << endl;
    }
}
//跨线程调用
void SocketWorker::ModifyEvent(int fd,bool epollOut){
    cout << "ModifyEvent fd " << fd << " " << epollOut << endl;
    struct epoll_event ev;
    ev.data.fd = fd;
    if(epollOut){
        ev.events = EPOLLIN | EPOLLET | EPOLLOUT;
    }
    else{
        ev.events = EPOLLIN | EPOLLET;
    }
    epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ev);
}
//跨线程调用
void SocketWorker::RemoveEvent(int fd){
    cout << "RemoveEvent fd" << fd << endl;
    epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, NULL);
}

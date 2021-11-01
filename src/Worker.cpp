#include <iostream>
#include <unistd.h>
#include "Worker.h"
#include "Service.h"
using namespace std;
//线程函数
void Worker::operator()() //重载括号操作符，对应线程执行代码
{
    while (true)
    {
        shared_ptr<Service> srv = Sunnet::inst->PopGlobalQueue(); //从全局服务队列里弹出一个服务来执行
        if (!srv)
        {
            cout << "Worker operator线程在等待，因为GlobalQueue是空的" << endl;
            Sunnet::inst->WorkerWait();
        }
        else
        {
            cout << "服务队列里有消息了ProcessMsgs处理" << endl;
            srv->ProcessMsgs(eachNum);
            CheckAndPutGlobal(srv);
        }
    }
}
//那些调Sunnet的通过传参数解决
//状态是不在队列中，global = true
void Worker::CheckAndPutGlobal(shared_ptr<Service> srv)
{
    //退出中（服务的退出方式只能它自己调用，这样isExiting才不会产生线程冲突
    if (srv->isExiting)
    {
        return;
    }
    pthread_spin_lock(&srv->queueLock);
    {
        //重新放回全局队列
        if (!srv->msgQueue.empty())
        {
            //srv->inGlobal true
            Sunnet::inst->PushGlobalQueue(srv);
        }
        ///不在队列中，重设inGlobal
        else
        {
            srv->SetInGlobal(false);
        }
    }
    pthread_spin_unlock(&srv->queueLock);
}

#include <iostream>
#include "Sunnet.h"
using namespace std;
//单例
Sunnet *Sunnet::inst;
Sunnet::Sunnet()
{
    inst = this;
}
//开启系统
void Sunnet::Start()
{
    cout << "hello Sunnet~!" << endl;
    //锁
    pthread_rwlock_init(&ServicesLock, NULL);
    //开启worker
    StartWorker();
}
//新建服务
uint32_t Sunnet::NewService(shared_ptr<string> type)
{
    auto srv = make_shared<Service>();
    srv->type = type;
    pthread_rwlock_wrlock(&servicesLock);
    {
        srv->id = maxId;
        maxId++;
        services.emplace(srv->id, srv);
    }
    pthread_rwlock_unlock(&servicesLock);
    srv->OnInit(); //初始化
    return srv->id;
}
//等待
void Sunnet::Wait()
{
    if (workerThreads[0])
    {
        workerThreads[0]->join();
    }
}
//开启worker线程
void Sunnet::StartWorker()
{
    for (int i = 0; i < WORKER_NUM; i++)
    {
        cout << "start worker thread:" << i << endl;
        Worker *worker = new Worker();
        worker->id = i;
        worker->eachNum = 2 << i;
        //创建线程
        thread *wt = new thread(*worker);
        //添加到数组
        workers.push_back(worker);
        workerThreads.push_back(wt);
    }
}

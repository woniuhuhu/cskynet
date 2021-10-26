#pragma once
#include "Service.h"
#include <unordered>
#include <vector>
#include "Worker.h"
class Sunnet{
    public :
        //单例
        static Sunnet* inst;
    public:
        //构造函数
        Sunnet();
        //初始化并开始
        void Start();
        //等待运行
        void Wait();
    public:
        //服务列表
        unordered_map <uint32_t,shared_ptr <Service>> Services;
        uint32_t maxId = 0;  //最大ID
        pthread_rwlock_t serviceLock;//读写锁
    public：
        //增删服务
        uint32_t NewService(shared_ptr<string> type);
        void KillService(uint32_t id);//仅限服务自己调用
    private:
        //获取服务
        shared_ptr <Service> GetService(uint32_t id);
    private:
        //工作线程
        int WORKER_NUM = 3;//工作线程数
        vector <Worker*> workers;//worker对象
        vector <thread*> workerThreads;//线程
    private:
        //开启工作线程
        void StartWorker();

};
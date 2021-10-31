#pragma once
#include "Service.h"
#include <unordered_map>
#include <vector>
#include "Worker.h"
#include "SocketWorker.h"
#include "Conn.h"
class Worker;
class Sunnet{
    public:
        //单例
        static Sunnet* inst;
        //构造函数
        Sunnet();
        //初始化并开始
        void Start();
        //等待运行
        void Wait();
        //服务列表
        unordered_map <uint32_t,shared_ptr <Service>> services;
        uint32_t maxId = 0;  //最大ID
        pthread_rwlock_t servicesLock;     //读写锁
        //增删服务
        uint32_t NewService(shared_ptr<string> type);
        void KillService(uint32_t id);//仅限服务自己调用
        //发送消息
        void Send(uint32_t toId,shared_ptr<BaseMsg> msg);
        //全局队列操作
        shared_ptr<Service> PopGlobalQueue();
        void PushGlobalQueue(shared_ptr<Service> srv);
        //5-42
        shared_ptr<BaseMsg> MakeMsg(uint32_t source,char* buff,int len);
        //唤醒工作进程
        void CheckAndWeakUp();
        //让工作线程等待（仅工作线程调用）
        void WorkerWait();
        //增删查Conn
        int AddConn(int fd,uint32_t id,Conn::TYPE type);
        shared_ptr<Conn> GetConn(int fd);
        bool RemoveConn(int fd);
        //网络连接操作接口
        int Listen(uint32_t port,uint32_t serviceId);
        void CloseConn(uint32_t fd);
    private:
        //获取服务
        shared_ptr <Service> GetService(uint32_t id);
        //工作线程
        int WORKER_NUM = 3;//工作线程数
        vector <Worker*> workers;//创建一个worker对象
        vector <thread*> workerThreads;//创建一条线程
        //开启工作线程
        void StartWorker();
        //全局队列
        queue<shared_ptr<Service>> globalQueue;
        int globalLen = 0; //队列长度
        pthread_spinlock_t globalLock;//锁
        //休眠和唤醒
        pthread_mutex_t sleepMtx;//互斥锁
        pthread_cond_t sleepCond;//条件变量
        int sleepCount;//休眠工作线程数
        //Socket线程
        SocketWorker* socketWorker;
        thread* socketThread;
        //开启Socket线程
        void StartSocket();
        //Conn列表
        unordered_map<uint32_t, shared_ptr<Conn>> conns;
        pthread_rwlock_t connsLock;//读写锁

};
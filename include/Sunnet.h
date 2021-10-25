#pragma once
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
    private:
        //工作线程
        int WORKER_NUM = 3;//工作线程数
        vector<Worker*> workers;//worker对象
        vector<thread*> workerThreads;//线程
    private:
        //开启工作线程
        void StartWorker();

};
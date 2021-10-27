#pragma once
#include <thread>
#include "Sunnet.h"
class Sunnet;
using namespace std;


class Worker {
    public:
        int id;//编号
        int eachNum;//每次处理多少条消息
        void operator()();//线程函数
        void CheckAndPutGlobal(shared_ptr<Service> srv);
};
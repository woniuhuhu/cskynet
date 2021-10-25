#include <iostream>
#include <unistd.h>
#include "Worker.h"
using namespace std;
//线程函数
void Worker::operator()(){
    while(true){
        cout<<"working id:"<<id<<endl;
        usleep(100000);//0.1s
    }
}

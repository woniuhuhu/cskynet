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
    pthread_rwlock_init(&servicesLock, NULL);
    pthread_spin_init(&globalLock,PTHREAD_PROCESS_PRIVATE);
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
//由ID查找服务
shared_ptr<Service> Sunnet::GetService(uint32_t id)
{
    shared_ptr<Service> srv = NULL;
	pthread_rwlock_rdlock(&servicesLock);
	{
		unordered_map<uint32_t,shared_ptr<Service>>::iterator iter = services.find(id);
		if(iter != services.end()){ //.end()哈希表的结束位置
			srv = iter->second;//first是键，second是值
		}
	}
	pthread_rwlock_unlock(&servicesLock);
	return srv;
}
//删除服务
//只能service自己调自己，调用不加锁的srv->OnExit和srv->isExiting
void Sunnet::KillService(uint32_t id){
    shared_ptr<Service> srv = GetService(id);
    if(!srv){
        return;
    }
    //退出前
    srv->OnExit();
    srv->isExiting = true;
    //删列表
    pthread_rwlock_wrlock(&servicesLock);
    {
        services.erase(id);
    }
    pthread_rwlock_unlock(&servicesLock);
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
//弹出全局队列
shared_ptr<Service> Sunnet::PopGlobalQueue(){
    shared_ptr<Service> srv = NULL;
    pthread_spin_lock(&globalLock);
    {
        if(!globalQueue.empty()){
            srv = globalQueue.front();
            globalQueue.pop();
            globalLen--;
        }
    }
    pthread_spin_unlock(&globalLock);
    return srv;
}
//插入全局队列
void Sunnet::PushGlobalQueue(shared_ptr<Service> srv){
    pthread_spin_lock(&globalLock);
    {
        globalQueue.push(srv);
        globalLen++;
    }
    pthread_spin_unlock(&globalLock);
}
//5-42
shared_ptr<BaseMsg> Sunnet::MakeMsg(uint32_t source,char* buff,int len){
    auto msg = make_shared<ServiceMsg>();
    msg->type = BaseMsg::TYPE::SERVICE;
    msg->source = source;
    //.....懒啊
    msg->buff = shared_ptr<char>(buff);
    msg->size = len;
    return msg;
}
//发送消息
void Sunnet::Send(uint32_t toId,shared_ptr<BaseMsg> msg){
	shared_ptr<Service> toSrv = GetService(toId);
	if(!toSrv){
		cout<<"Send fail,toSrv not exist toId:"<<toId<<endl;
		return;
	}
	//插入目标服务的消息队列
	toSrv->PushMsg(msg);
	//检查并放入全局队列
	bool hasPush = false;
	pthread_spin_lock(&toSrv->inGlobalLock);
	{
		if(!toSrv->inGlobal){
			PushGlobalQueue(toSrv);
			toSrv->inGlobal = true;
			hasPush = true;
		}
	}
	pthread_spin_unlock(&toSrv->inGlobalLock);
	//唤起进程（后面实现）
}
	

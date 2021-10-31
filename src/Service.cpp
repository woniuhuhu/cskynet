#include "Service.h"
#include "Sunnet.h"
#include <iostream>
#include <unistd.h>
#include <string.h>

//构造函数
Service::Service(){
	//初始化锁
	pthread_spin_init(&queueLock,PTHREAD_PROCESS_PRIVATE);
	pthread_spin_init(&inGlobalLock,PTHREAD_PROCESS_PRIVATE);
}
	//析构函数
Service::~Service(){
	pthread_spin_destroy(&queueLock);
	pthread_spin_destroy(&inGlobalLock);
}
//插入消息
void Service::PushMsg(shared_ptr <BaseMsg> msg){
	pthread_spin_lock(&queueLock);
	{
		msgQueue.push(msg);
	}
	pthread_spin_unlock(&queueLock);
}
//取出消息
shared_ptr <BaseMsg> Service::PopMsg(){
	shared_ptr <BaseMsg> msg = NULL;
	//取一条消息
	pthread_spin_lock(&queueLock);
	{
		if(!msgQueue.empty()){
			msg = msgQueue.front();
			msgQueue.pop();
		}
	}
	pthread_spin_unlock(&queueLock);
	return msg;
}
//创建服务后触发
void Service::OnInit(){
	cout<<"["<<id<<"] OnInit"<<endl;
	//开启监听
	Sunnet::inst->Sunnet::Listen(8002,id);
}
/* //发送消息
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
	//唤起进程,不放在临界区里面
    if(hasPush){
        CheckAndWeakUp();
    }
} */
//收到消息时触发
void Service::OnMsg(shared_ptr<BaseMsg> msg){
	//SOCKET_ACCEPT
	if(msg->type == BaseMsg::TYPE::SOCKET_ACCEPT){
		auto m = dynamic_pointer_cast<SocketAcceptMsg>(msg);
		cout<<"new conn"<<m->clientFd<<endl;
	}
	//SOCKET_RW
	if(msg->type == BaseMsg::TYPE::SOCKET_RW){
		auto m = dynamic_pointer_cast<SocketRWMsg>(msg);
		if(m->isRead){
			char buff[512];
			int len = read(m->fd,&buff,512);
			if(len>0){
				char writeBuff[3] = {'l','p','y'};
				write(m->fd,&writeBuff,3);
			}
			else{
				cout<<"close"<<m->fd<<strerror(errno)<<endl;
				Sunnet::inst->CloseConn(m->fd);
			}
		}
	}	
}	
	//测试用
//	if(msg->type == BaseMsg::TYPE::SERVICE){
//		auto m = dynamic_pointer_cast<ServiceMsg>(msg);
//		cout<<"["<<id<<"]  OnMsg"<<m->buff<<endl;
//		auto msgRet = Sunnet::inst->MakeMsg(id,new char[9999999]{'p','i','n','g','\0'},9999999);
//		Sunnet::inst->Send(m->source,msgRet);
//	}
//	else{
//		cout<<"["<<id<<"] OnMsg"<<endl;
//	}

//推出消息时触发
void Service::OnExit(){
	cout<<"["<<id<<"] onExit"<<endl;
}
//处理一条消息，返回值代表是否处理
bool Service::ProcessMsg(){
	shared_ptr<BaseMsg> msg = PopMsg();
	if(msg){
		OnMsg(msg);
		return true;
	}
	else{
		return false;//返回值预示队列是否为空
	}
}
//处理N条信息，返回值代表是否处理
void Service::ProcessMsgs(int max){
	for(int i=0;i<max;i++){
		bool succ = ProcessMsg();
		if(!succ){
			break;
		}
	}	
}
//SetInGlobal
void Service::SetInGlobal(bool isIn){
	pthread_spin_lock(&inGlobalLock);
	{
		inGlobal = isIn;
	}
	pthread_spin_unlock(&inGlobalLock);
}

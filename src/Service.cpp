#include "Service.h"
#include "Sunnet.h"
#include <iostream>

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
}
//收到消息时触发
void Service::OnMsg(shared_ptr<BaseMsg> msg){
	//测试用
	if(msg->type == BaseMsg::TYPE::SERVICE){
		auto m = dynamic_pointer_cast<ServiceMsg>(msg);
		cout<<"["<<id<<"]  OnMsg"<<m->buff<<endl;
		auto msgRet = Sunnet::inst->MakeMsg(id,new char[9999999]{'p','i','n','g','\0'},9999999);
		Sunnet::inst->Send(m->source,msgRet);
	}
	else{
		cout<<"["<<id<<"] OnMsg"<<endl;
	}
}
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


#include "Service.h"
#include "Sunnet.h"
#include <iostream>
#include <unistd.h>
#include <string.h>
#include "LuaAPI.h"
//构造函数
Service::Service()
{
	//初始化锁
	pthread_spin_init(&queueLock, PTHREAD_PROCESS_PRIVATE);
	pthread_spin_init(&inGlobalLock, PTHREAD_PROCESS_PRIVATE);
}
//析构函数
Service::~Service()
{
	pthread_spin_destroy(&queueLock);
	pthread_spin_destroy(&inGlobalLock);
}
//插入消息
void Service::PushMsg(shared_ptr<BaseMsg> msg)
{
	pthread_spin_lock(&queueLock);
	{
		msgQueue.push(msg);
	}
	pthread_spin_unlock(&queueLock);
}
//取出消息
shared_ptr<BaseMsg> Service::PopMsg()
{
	shared_ptr<BaseMsg> msg = NULL;
	//取一条消息
	pthread_spin_lock(&queueLock);
	{
		if (!msgQueue.empty())
		{
			msg = msgQueue.front();
			msgQueue.pop();
		}
	}
	pthread_spin_unlock(&queueLock);
	return msg;
}
//创建服务后触发
void Service::OnInit()
{
	cout << "[" << id << "] OnInit" << endl;
	//新建lua虚拟机
	//创建lua_State对象
	luaState = luaL_newstate();
	//开启标准库
	luaL_openlibs(luaState);
	//注册Sunnet系统API
	LuaAPI::Register(luaState);
	//执行Lua文件
	string filename = "../service/"+*type+"/init.lua";
	int isok = luaL_dofile(luaState,filename.data());
	if(isok == 1){//若成功则返回值为0，失败为1
		cout<<"run lua fail: "<<lua_tostring(luaState,-1)<<endl;
	}
	//调用lua函数
	lua_getglobal(luaState,"OnInit");
	lua_pushinteger(luaState,id);
	isok = lua_pcall(luaState,1,0,0);
	if(isok != 0){//若返回值为0则代表成功，否侧代表失败
		cout<<"call lua OnInit fail "<<lua_tostring(luaState,-1)<<endl;
	}

	//开启监听
	Sunnet::inst->Sunnet::Listen(8002, id);
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
void Service::OnMsg(shared_ptr<BaseMsg> msg)
{
	//SERVICE
	if (msg->type == BaseMsg::TYPE::SERVICE)
	{
		auto m = dynamic_pointer_cast<ServiceMsg>(msg);
		OnServiceMsg(m);
	}
	//SOCKET_ACCEPT
	else if (msg->type == BaseMsg::TYPE::SOCKET_ACCEPT)
	{
		auto m = dynamic_pointer_cast<SocketAcceptMsg>(msg);
		cout << "new conn ACCEPT" << m->clientFd << endl;
		OnAcceptMsg(m);
	}
	//SOCKET_RW
	else if (msg->type == BaseMsg::TYPE::SOCKET_RW)
	{
		cout << "SOCKET_RW m->isread" << endl;
		auto m = dynamic_pointer_cast<SocketRWMsg>(msg);
		OnRWMsg(m);
		/*	if (m->isRead)
		{
			cout << "SOCKET_RW m->isread" << endl;
			char buff[512];
			int len = read(m->fd, &buff, 512);
			if (len > 0)
			{
				char writeBuff[3] = {'l', 'p', 'y'};
				write(m->fd, &writeBuff, 3);
			}
			else
			{
				cout << "close" << m->fd << strerror(errno) << endl;
				Sunnet::inst->CloseConn(m->fd);
			}
		}*/
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
void Service::OnExit()
{
	cout << "[" << id << "] onExit" << endl;
	//调用lua函数
	lua_getglobal(luaState,"OnExit");
	int isok = lua_pcall(luaState,0,0,0);
	if(isok != 0){
		cout<<"call lua OnExit fail"<<lua_tostring(luaState,-1)<<endl;
	}
	//关闭虚拟机
	lua_close(luaState);
}
//处理一条消息，返回值代表是否处理
bool Service::ProcessMsg()
{
	shared_ptr<BaseMsg> msg = PopMsg();
	if (msg)
	{
		cout << "ProcessMsg里的OnMsg" << endl;
		OnMsg(msg);
		cout << "ProcessMsg里的OnMsg later" << endl;
		return true;
	}
	else
	{
		return false; //返回值预示队列是否为空
	}
}
//处理N条信息，返回值代表是否处理
void Service::ProcessMsgs(int max)
{
	for (int i = 0; i < max; i++)
	{
		bool succ = ProcessMsg();
		if (!succ)
		{
			break;
		}
	}
}
//SetInGlobal
void Service::SetInGlobal(bool isIn)
{
	pthread_spin_lock(&inGlobalLock);
	{
		inGlobal = isIn;
	}
	pthread_spin_unlock(&inGlobalLock);
}
//收到其他服务发来的消息
void Service::OnServiceMsg(shared_ptr<ServiceMsg> msg)
{
	//调用Lua函数
	lua_getglobal(luaState,"OnServiceMsg");//将lua OnServiceMsg函数压栈
	lua_pushinteger(luaState,msg->source);
	lua_pushlstring(luaState,msg->buff.get(),msg->size);
	int isok = lua_pcall(luaState,2,0,0);
	if(isok != 0){//若返回值为0，则代表成功，否则失败
		cout<<"call lua OnServiceMsg fail"<<lua_tostring(luaState,-1)<<endl;
	}
}
//新连接
void Service::OnAcceptMsg(shared_ptr<SocketAcceptMsg> msg)
{
	cout << "OnAcceptMsg " << msg->clientFd << endl;
	auto w = make_shared<ConnWriter>();
	w->fd = msg->clientFd;
	writers.emplace(msg->clientFd, w);
}
//套接字可读可写
void Service::OnRWMsg(shared_ptr<SocketRWMsg> msg)
{
	int fd = msg->fd;
	//可读
	if (msg->isRead)
	{
		const int BUFFSIZE = 512;
		char buff[BUFFSIZE];
		int len = 0;
		do
		{
			len = read(fd, &buff, BUFFSIZE);
			if (len > 0)
			{
				OnSocketData(fd, buff, len);
			}
		} while (len == BUFFSIZE);
		if (len <= 0 && errno != EAGAIN)
		{
			if (Sunnet::inst->GetConn(fd))
			{ //加入判断是为了防止OnSocketClose执行了两次
				OnSocketClose(fd);
				Sunnet::inst->CloseConn(fd);
			}
		}
	}
	//可写
	if (msg->isWrite)
	{
		if (Sunnet::inst->GetConn(fd))
		{ //保证OnSocketWritable有效
			OnSocketWritable(fd);
		}
	}
}

//收到客户端数据
void Service::OnSocketData(int fd, const char *buff, int len)
{
	cout << "OnSocketData " << fd << "buff: " << buff << endl;
	//用ConnWriter发送大量数据
	char *writeBuff = new char[4200000];
	writeBuff[4200000 - 1] = 'e';
	int r = write(fd, writeBuff, 4200000);
	cout << "write r: " << r << " " << strerror(errno) << endl;
	auto w = writers[fd];
	w->EntireWrite(shared_ptr<char>(writeBuff), 4200000);
	w->LingerClose();
	//echo
	//char writeBuff[3] = {'l','p','y'};
	//write(fd,&writeBuff,3};
}
//套接字可写
void Service ::OnSocketWritable(int fd)
{
	cout << "OnSocketWritable:" << fd << endl;
	auto w = writers[fd];
	w->OnWriteable();
}
//关闭连接前
void Service::OnSocketClose(int fd)
{
	cout << "OnSocketClose:" << fd << endl;
	writers.erase(fd);
}

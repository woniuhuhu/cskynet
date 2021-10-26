#pragma once
#include <queue>
#include <thread>
#include "Msg.h"
using namespace std;
class Service {
	public:
		//唯一ID
		unit32_t id;
		//类型
		shared_ptr<string> type;
		//是否正在退出
		bool isExiting = false;
		//消息列表和锁
		queue <shared_ptr<BaseMsg>> msgQueue;
		pthread_spinlock_t queueLock;
	public:
		//构造和析构函数
		Service();
		~Service();
		//回调函数（编写服务逻辑)
		void OnInit();
		void OnMsg(shared_ptr<BaseMsg> msg);
		void OnExit();
		//插入消息
		void PushMsg(shared_ptr<BaseMsg> msg);
		//执行消息
		bool ProcessMsg();
		void ProcessMsgs(int max);
	private:
		//取出一条消息
		shared_ptr <BaseMsg> PopMsg();
};

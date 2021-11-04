#pragma once
#include <list>
#include <stdint.h>
#include <memory>
using namespace std;

class WriteObject{
	public:
		streamsize start;//代表已经写入套接字写缓冲区的字节数
		streamsize len;//代表某次发送的总字节数
		shared_ptr<char> buff;//代表某次发送的内容
};

class ConnWriter{
	public:
		int fd;
	private:
		//是否在关闭
		bool isClosing = false;
		list<shared_ptr<WriteObject>> objs;//双向连接
	public:
		void EntireWrite(shared_ptr<char> buff,streamsize len);//发送数据
		void LingerClose();//全部发完后再关闭
		void OnWriteable();
	private:
	    void EntireWriteWhenEmpty(shared_ptr<char> buff, streamsize len);
   	    void EntireWriteWhenNotEmpty(shared_ptr<char> buff, streamsize len);
   	    bool WriteFrontObj();
};

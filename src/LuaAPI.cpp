#include "LuaAPI.h"
#include "stdint.h"
#include "Sunnet.h"
#include <unistd.h>
#include <cstring>
#include <iostream>
//开启新服务
int LuaAPI::NewService(lua_State *luaState){
	//参数个数
	int num = lua_gettop(luaState);//获取栈顶元素的索引，也就是栈内元素的个数
	//参数1：服务类型
	if(lua_isstring(luaState,1)==0){//  判断栈中指定位置的元素是否为字符串 1:是 0:不是
		lua_pushinteger(luaState,-1);
		return 1;
	}
	size_t len = 0;
	const char *type = lua_tolstring(luaState,1,&len);//把指定索引处的值转化为字符串，并把字符串的长度存入*len中
	char *newstr = new char[len+1];//后面加\0
	newstr[len] = '\0';
	memcpy(newstr,type,len);
	auto t = make_shared<string>(newstr);
	//处理
	uint32_t id = Sunnet::inst->NewService(t);
	//返回值
	lua_pushinteger(luaState,id);
	return 1;
}
//关闭服务
int LuaAPI::KillService(lua_State *luaState){
	//参数
	int num = lua_gettop(luaState);//获取参数的个数
	if(lua_isinteger(luaState,1)==0){
		return 0;
	}
	int id = lua_tointeger(luaState,1);
	//处理
	Sunnet::inst->KillService(id);
	//返回值
	//无
	return 0;
}
//发送消息
int LuaAPI::Send(lua_State *luaState){
	//参数总数
	int num = lua_gettop(luaState);
	if(num !=3){
		cout<<"Send fail,num err"<<endl;
		return 0;
	}
	//参数1：我是谁
	if(lua_isinteger(luaState,1)==0){
		cout<<"Send fail,arg1 er"<<endl;
		return 0;
	}
	int source = lua_tointeger(luaState,1);
	//参数2：发给谁
	if(lua_isinteger(luaState,2)==0){
		cout<<"Send fail arg2 er"<<endl;
		return 0;
	}
	int toId = lua_tointeger(luaState,2);
	//参数3：发送的内容
	if(lua_isstring(luaState,3)==0){
		cout<<"Send fail arg3 er"<<endl;
		return 0;
	}
	size_t len = 0;
	const char *buff = lua_tolstring(luaState,3,&len);
	char* newstr = new char[len];
	memcpy(newstr,buff,len);
	//处理
	auto msg = make_shared<ServiceMsg>();
	msg->type = BaseMsg::TYPE::SERVICE;
	msg->source = source;
	msg->buff = shared_ptr<char>(newstr);
	msg->size = len;
	Sunnet::inst->Send(toId,msg);
	//返回值
	//无
	return 0;
}
//写套接字
int LuaAPI::Write(lua_State *luaState){
	//参数个数
	int num = lua_gettop(luaState);
	//参数1：fd
	if(lua_isinteger(luaState,1)==0){
		lua_pushinteger(luaState,-1);
		return 1;
	}
	int fd = lua_tointeger(luaState,1);
	//参数2：buff
	if(lua_isstring(luaState,2)==0){
		lua_pushinteger(luaState,-1);
		return 1;
	}
	size_t len = 0;
	const char *buff = lua_tolstring(luaState,2,&len);
	//处理
	int r = write(fd,buff,len);
	//返回值
	lua_pushinteger(luaState,r);
	return 1;
}
//开启网络监听
int LuaAPI::Listen(lua_State *luaState){
    //参数个数
    int num = lua_gettop(luaState);
    //参数1：端口
    if(lua_isinteger(luaState, 1) == 0) {
        lua_pushinteger(luaState, -1);
        return 1;
    }
    int port = lua_tointeger(luaState, 1);
    //参数2：服务Id
    if(lua_isinteger(luaState, 2) == 0) {
        lua_pushinteger(luaState, -1);
        return 1;
    }
    int id = lua_tointeger(luaState, 2);
    //处理
    int fd = Sunnet::inst->Listen(port, id);
    //返回值
    lua_pushinteger(luaState, fd);
    return 1;
}
//关闭连接
int LuaAPI::CloseConn(lua_State *luaState){
    //参数个数
    int num = lua_gettop(luaState);
    //参数1：fd
    if(lua_isinteger(luaState, 1) == 0) {
        return 0;
    }
    int fd = lua_tointeger(luaState, 1);
    //处理
    Sunnet::inst->CloseConn(fd);
    //返回值
    //（无）
    return 0;
}
//注册lua模块
void LuaAPI::Register(lua_State *luaState){
	static luaL_Reg lualibs[] = {//用于注册函数的数组类型，每一项有两个参数，第一个代表lua中方法的名字，第二个对应c++方法
		{"NewService",NewService},
		{"KillService",KillService},
		{"Send",Send},
		{"Listen",Listen},
		{"CloseConn",CloseConn},
		{"Write",Write},
		{NULL,NULL}//表示结束
	};
	luaL_newlib(luaState,lualibs);//在栈中创建一张表，把数组lualibs中的函数注册到表中
	lua_setglobal(luaState,"sunnet");//将栈顶元素(也就是那张表）放入全局空间，并重新命名为sunnet
}


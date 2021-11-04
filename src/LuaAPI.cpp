#include "LuaAPI.h"
#include "stdint.h"
#include "Sunnet.h"
#include <unistd.h>
#include <cstring>
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
//注册lua模块
void LuaAPI::Register(lua_State *luaState){
	static luaL_Reg lualibs[] = {//用于注册函数的数组类型，每一项有两个参数，第一个代表lua中方法的名字，第二个对应c++方法
		{"NewService",NewService},
		//{"KillService",KillService},
		//{"Send",Send},
		//{"Listen",Listen},
		//{"CloseConn",CloseConn},
		//{"Write",Write},
		//{NULL,NULL}//表示结束
	};
	luaL_newlib(luaState,lualibs);//在栈中创建一张表，把数组lualibs中的函数注册到表中
	lua_setglobal(luaState,"sunnet");//将栈顶元素放入全局空间，并重新命名
}

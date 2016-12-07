// luajittest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <atlstr.h>
#include <lua.hpp>


#pragma comment(lib,"../bin/lua51.lib")
#pragma comment(lib,"../bin/luajit.lib")


bool RunLuaScript(lua_State* L, const char* strScript,int nScriptLen)
{
	if ( L==NULL || strScript==NULL || nScriptLen<=0 ) {
		return false;
	}

	if ( luaL_loadbuffer(L, strScript, nScriptLen, strScript) || lua_pcall(L, 0, 0, 0) ){
		const char* a = lua_tostring(L, -1);
		lua_pop(L, 1);
		return true;
	}
	return false;
}

int _tmain(int argc, _TCHAR* argv[])
{
	lua_State* L = lua_open();
	luaL_openlibs(L);

	CStringA str;
	str = "local ffi = require('ffi')"
		"ffi.cdef[["
		"typedef void* HWND;"
		"typedef unsigned int UINT;"
		"	int printf(const char *fmt, ...);"
		"	int MessageBoxA(  HWND hWnd,   const char * lpText,   const char * lpCaption,   UINT uType ); "
		"]]"
		"ffi.C.printf('Hello %s!', 'world')"
		"ffi.C.MessageBoxA(nil,'a','b',0)";
	RunLuaScript(L,str,str.GetLength());


	lua_close(L);

	return 0;
}


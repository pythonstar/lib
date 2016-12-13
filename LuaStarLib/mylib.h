#pragma once

extern "C" {
	#include <lua/lua.h>
}

extern "C" __declspec(dllexport) int luaopen_star(lua_State *L);

int msgbox(lua_State *L);
int log(lua_State *L);
int gethtml(lua_State *L);
int gethtmlex(lua_State *L);
int urldownload2str(lua_State *L);
int utf8s2ms(lua_State *L);
int openurl(lua_State *L);
int md5(lua_State *L);
int sleep(lua_State *L);
int unescapeunicode(lua_State *L);
int filterinvalidfilename(lua_State *L);
int unescapexml(lua_State *L);
int encodeurlutf8(lua_State *L);
int encodeurlgbk(lua_State *L);
int sendhttpdata(lua_State *L);
int getcurrenttime(lua_State *L);
int trim(lua_State* L);
int	run(lua_State* L);
int runbat(lua_State *L);
int reversefind(lua_State* L);
int getimagesize(lua_State* L);
int getfilesize(lua_State* L);


#ifdef _USRDLL

int _crc32(lua_State *L);

int test(lua_State *L);
int help(lua_State *L);
int dbgprint(lua_State *L);
int getstartpath(lua_State *L);
int getluapath(lua_State *L);
int gettemppath(lua_State *L);
int gettempfilepath(lua_State *L);
int getdesktoppath(lua_State *L);
int createdirectory(lua_State *L);
int deletedirectory(lua_State *L);
int downloadfile(lua_State *L);
int copytoclipboard(lua_State *L);
int getclipboardtext(lua_State *L);
int md5file(lua_State *L);

int postmessage(lua_State *L);
int sendmessage(lua_State *L);
int loadlibrary(lua_State *L);
int getprocaddress(lua_State* L);

int keypress(lua_State* L);
int writeunicodestring(lua_State* L);
int renamefile(lua_State* L);
int locatefile(lua_State *L);
int readtextfile(lua_State *L);

#endif

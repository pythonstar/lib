#include "stdafx.h"
#include "mylib.h"
#include <StarLib/File/File.h>
#include <StarLib/Common/common.h>
#include <StarLib/Common/zipHelper.h>
#include <StarLib/net/htmldown.h>
#include <starlib/net/httppost.h>
#include <StarLib/MD5/Md5.h>
#include <atlimage.h>
#include <string>
using namespace std;



#pragma warning(disable:4311)

#ifdef __cplusplus
extern "C" {
#endif

#include <zlib/zlib.h>
#include <zlib/zconf.h>
#include <lua/lauxlib.h>
#include <lua/lualib.h>

#ifdef __cplusplus
}
#endif


#ifdef _DEBUG
#pragma comment(lib,"luaD.lib")
#ifdef _USRDLL
#pragma comment(lib,"zlibD.lib")
#pragma comment(lib,"zipD.lib")
#endif
//#pragma comment(lib,"User32.lib")
#else
#pragma comment(lib,"lua.lib")
#ifdef _USRDLL
#pragma comment(lib,"zlib.lib")
#pragma comment(lib,"zip.lib")
#endif
#endif



#define MODULE_NAME_STAR	"star"

static const struct luaL_reg starlib [] = { 

	{"msgbox", msgbox},
	{"log", log},
	{"md5", md5},
	{"trim", trim},
	{"utf8s2ms", utf8s2ms},
	{"reversefind", reversefind},
	{"unescapeunicode", unescapeunicode},
	{"filterinvalidfilename", filterinvalidfilename},
	{"unescapexml", unescapexml},
	{"encodeurlutf8", encodeurlutf8},
	{"encodeurlgbk", encodeurlgbk},
	{"getfilesize", getfilesize},
	{"getimagesize", getimagesize},
	{"gethtml", gethtml},
	{"gethtmlex", gethtmlex},
	{"urldownload2str", urldownload2str},
	{"openurl", openurl},
	{"sendhttpdata", sendhttpdata},
	{"exec", run},
	{"run", run},
	{"runbat", runbat},
	{"createprocess", run},
	{"open", run},
	{"sleep", sleep},
	{"getcurrenttime", getcurrenttime},

#ifdef _USRDLL
	{"help", help},
	{"test", test},
	{"dbgprint", dbgprint},
	{"md5file", md5file},
	{"getstartpath", getstartpath},
	{"getluapath", getluapath},
	{"gettemppath", gettemppath},
	{"gettempfilepath", gettempfilepath},
	{"getdesktoppath", getdesktoppath},
	{"createdirectory", createdirectory},
	{"deletedirectory", deletedirectory},
	{"renamefile", renamefile},
	{"writeunicodestring", writeunicodestring},
	{"locatefile", locatefile},
	{"selectfile", locatefile},
	{"readtextfile", readtextfile},
	{"downloadfile", downloadfile},

	{"copytoclipboard", copytoclipboard},
	{"getclipboardtext", getclipboardtext},
	{"keypress", keypress},
	{"postmessage", postmessage},
	{"sendmessage", sendmessage},
	{"loadlibrary", loadlibrary},
	{"getprocaddress", getprocaddress},

	{"crc32", _crc32},
#endif

	{NULL, NULL} /* sentinel */ 
};

//////////////////////////////////////////////////////////////////////////

#ifdef _MANAGED
#pragma managed(push, off)
#endif


extern "C" __declspec(dllexport) int luaopen_star(lua_State *L) { 
	CoInitialize(NULL);
	luaL_register(L, MODULE_NAME_STAR, starlib);	

	return 1; 
}

#ifdef _MANAGED
#pragma managed(pop)
#endif
//////////////////////////////////////////////////////////////////////////



CString GetLuaFilePath(lua_State *L)
{
	CString strFilePath;
	int level= 0;
	lua_Debug dbg;
	memset(&dbg, 0, sizeof(dbg));

	while ( lua_getstack(L,level++,&dbg) );
	lua_getstack(L,--level,&dbg);
	if (lua_getinfo(L, "S", &dbg) != 0){
		if ( dbg.source!=NULL ) {
			strFilePath.Append(dbg.source + 1);	//第一个字符是@
		}
	}

	return strFilePath;
}

int msgbox(lua_State *L) { 
	int nRet = IDOK;
	const char *szText = "nil";
	const char *szCaption = "提示";
	UINT uType = MB_OK;
	int i = 1;

	int n = lua_gettop(L);
	
	if ( n>=3 ) {
		if ( lua_isstring(L,1) ) {
			szText = lua_tostring(L,1);
		}
		if ( lua_isstring(L,2) ) {
			szCaption = lua_tostring(L,2);
		}
		if ( lua_isnumber(L,3) ){
			uType = (UINT)lua_tonumber(L,3);
		}
	}else if ( n>=2 ) {
		if ( lua_isstring(L,1) ) {
			szText = lua_tostring(L,1);
		}
		if ( lua_isnumber(L,2) ){
			uType = (UINT)lua_tonumber(L,2);
		}else if ( lua_isstring(L,2) ) {
			szCaption = lua_tostring(L,2);
		}
	}else if ( n>=1 ) {
		if ( lua_isstring(L,1) ) {
			szText = lua_tostring(L,1);
		}
	}else{
	}

	nRet = ::MessageBoxA(NULL, szText, szCaption, uType);
	lua_pushnumber(L,nRet);
	return 1;
}

//参数：s,[file],[isappend]
int log(lua_State *L)
{
	CString strLogFile;
	CString strFilePath;
	CString strText;
	BOOL bAppend = FALSE;
	DWORD dwWrite = 0;
	BOOL bOK = FALSE;

	int n =  lua_gettop(L);
	if ( n > 0 ){
		if ( lua_isnil(L, 1) ) {
			strText = "nil";
		}else{
			strText = lua_tostring(L, 1);
		}
	}
	if ( n > 1 ){
		if ( lua_isstring(L, 2) ) {
			strLogFile = lua_tostring(L, 2);
			strLogFile.Trim();
		}
	}
	if ( n > 2 ){
		bAppend = lua_toboolean(L, 3);
	}

	if ( strLogFile.IsEmpty() ) {
		strLogFile = Star::Common::GetParentPath(GetLuaFilePath(L)) + "log.txt";
	}else if ( strLogFile.Find(':')==-1 ) {
		strLogFile = Star::Common::GetParentPath(GetLuaFilePath(L)) + strLogFile;
	}

	HANDLE hFile = CreateFile(strLogFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, NULL, NULL);
	if ( hFile!=INVALID_HANDLE_VALUE ) {
		if ( bAppend ) {
			SetFilePointer(hFile, 0, 0, FILE_END);
		}else{
			SetFilePointer(hFile, 0, 0, FILE_BEGIN);
			SetEndOfFile(hFile);
		}

		bOK = WriteFile(hFile, (LPCTSTR)strText, strText.GetLength(), &dwWrite, NULL);
		CloseHandle(hFile);
	}

	lua_pushboolean(L, bOK);
	return 1;
}

//html gethtml(url) 失败返回''空串，非nil 
int gethtml(lua_State *L)
{
	CString strHtml;
	const char *lpszStr = NULL;
	int n =  lua_gettop(L);
	if ( n > 0 ){
		if ( lua_isstring(L, 1) ) {
			lpszStr = lua_tostring(L, 1);
		}
	}

	if ( lpszStr==NULL ){
		lua_pushnil(L);
		return 1;
	}

	GetHttpFileContent(lpszStr, strHtml);
	lua_pushlstring(L, (const char *)(LPCTSTR)strHtml, strHtml.GetLength());
	return 1;
}

//html gethtml(url) 失败返回''空串，非nil 
int gethtmlex(lua_State *L)
{
	CString strHtml;
	const char *lpszStr = NULL;
	int n =  lua_gettop(L);
	if ( n > 0 ){
		if ( lua_isstring(L, 1) ) {
			lpszStr = lua_tostring(L, 1);
		}
	}

	if ( lpszStr==NULL ){
		lua_pushnil(L);
		return 1;
	}

	GetHttpFileContentEx(lpszStr, strHtml);
	lua_pushlstring(L, (const char *)(LPCTSTR)strHtml, strHtml.GetLength());
	return 1;
}

int urldownload2str(lua_State *L)
{
	CString strHtml;
	const char *lpszStr = NULL;
	int n =  lua_gettop(L);
	if ( n > 0 ){
		if ( lua_isstring(L, 1) ) {
			lpszStr = lua_tostring(L, 1);
		}
	}
	
	if ( lpszStr==NULL ){
		lua_pushnil(L);
		return 1;
	}

	URLDownloadToString(lpszStr, strHtml);
	lua_pushlstring(L, (const char *)(LPCTSTR)strHtml, strHtml.GetLength());
	return 1;
}

//utf8串转换为多字节的串
int utf8s2ms(lua_State *L)
{
	const char *lpszStr = NULL;
	int n = lua_gettop(L);
	if ( n > 0 ){
		if ( lua_isstring(L, 1) ) {
			lpszStr = lua_tostring(L, 1);
		}
	}

	if ( lpszStr==NULL ){
		lua_pushnil(L);
		return 1;
	}

	CString strText = Star::Common::utf8s2ts(lpszStr);

	lua_pushlstring(L, (const char *)(LPCTSTR)strText, strText.GetLength());
	return 1;
}

int openurl(lua_State *L)
{
	string strUrl;
	int nshowcmd = SW_SHOWNORMAL;
	BOOL bRet = FALSE;

	int n = lua_gettop(L);
	if ( n>0 ){
		if ( lua_isstring(L,1) ){
			strUrl = lua_tostring(L,1);
		}
	}
	if ( n>1 ){
		if ( lua_isnumber(L,2) ){
			nshowcmd = (int)lua_tonumber(L,2);
		}
	}

	if ( strUrl.empty()==false ){
		bRet = (Star::Common::OpenUrl(strUrl.c_str(),nshowcmd) > (HINSTANCE)((PBYTE)32+NULL));
	}

	lua_pushboolean(L,bRet);
	return 1;
}

int md5(lua_State *L)
{
	CString s;
	CString strMd5;

	int n = lua_gettop(L);
	if ( n>0 ){
		if ( lua_isstring(L,1) ){
			s = lua_tostring(L,1);
			lua_pushstring(L, MD5Data((char *)(LPCTSTR)s,s.GetLength()));
			return 1;
		}
	}

	return 0;
}

int sleep(lua_State *L)
{
	int n = lua_gettop(L);
	if ( n>0 ){
		if ( lua_isnumber(L,1) ){
			Sleep((DWORD)lua_tonumber(L,1));
		}
	}

	return 0;
}

int unescapeunicode(lua_State *L)
{
	CString str;
	int n = lua_gettop(L);
	if ( n>0 ){
		if ( lua_isstring(L,1) ){
			str = Star::Common::unescapeunicode(lua_tostring(L,1));
		}
	}

	lua_pushstring(L,str);
	return 1;
}

int filterinvalidfilename(lua_State *L)
{
	CString str;
	int n = lua_gettop(L);
	if ( n>0 ){
		if ( lua_isstring(L,1) ){
			str = Star::Common::filterinvalidfilename(lua_tostring(L,1));
		}
	}

	lua_pushstring(L,str);
	return 1;
}

int unescapexml(lua_State *L)
{
	CString str;
	int n = lua_gettop(L);
	if ( n>0 ){
		if ( lua_isstring(L,1) ){
			str = lua_tostring(L,1);
			Star::Common::unescapexml(str);
		}
	}

	lua_pushstring(L,str);
	return 1;
}

int encodeurlutf8(lua_State *L)
{
	const char *lpszStr = NULL;

	int n = lua_gettop(L);
	if ( n>0 ){
		if ( lua_isstring(L,1) ){
			lpszStr = lua_tostring(L, 1);
		}
	}
	if ( lpszStr==NULL ) {
		lua_pushnil(L);
		return 1;
	}

	CString strResult = Star::Common::UTF8EncodeURI(lpszStr);
	lua_pushlstring(L, (const char *)(LPCTSTR)strResult, strResult.GetLength());
	return 1;
}

int encodeurlgbk(lua_State *L)
{
	const char *lpszStr = NULL;

	int n = lua_gettop(L);
	if ( n>0 ){
		if ( lua_isstring(L,1) ){
			lpszStr = lua_tostring(L, 1);
		}
	}
	if ( lpszStr==NULL ) {
		lua_pushnil(L);
		return 1;
	}

	CString strResult = Star::Common::GBKEncodeURI(lpszStr);
	lua_pushlstring(L, (const char *)(LPCTSTR)strResult, strResult.GetLength());
	return 1;
}

int sendhttpdata(lua_State *L)
{
	CString strRet;

	CString strHost;
	CString strPath;
	CString strHeaders;
	vector<CString>vtHeaders;
	CString strSendData;
	CString strNewCookie;
	int nMethod = 0;
	BOOL bNeedDocode = TRUE;

	int n = lua_gettop(L);
	if ( n>0 ){
		if ( lua_isstring(L,1) ){
			strHost = lua_tostring(L,1);
		}
	}
	if ( n>1 ){
		if ( lua_isstring(L,2) ){
			strPath = lua_tostring(L,2);
		}
	}
	if ( n>2 ){
		if ( lua_isstring(L,3) ){
			strHeaders = lua_tostring(L,3);
		}
	}
	if ( n>3 ){
		if ( lua_isstring(L,4) ){
			strSendData = lua_tostring(L,4);
		}
	}
	if ( n>4 ){
		if ( lua_isnumber(L,5) ){
			nMethod = (int)lua_tonumber(L,5);
		}
	}
	if ( n>5 ){
		if ( lua_isboolean(L,6) ){
			bNeedDocode = (int)lua_toboolean(L,6);
		}
	}

	Star::Common::SplitString(strHeaders, "\n", vtHeaders);
	strRet = SendHttpData(strHost, strPath, vtHeaders, strSendData, strNewCookie, bNeedDocode, nMethod);
	lua_pushlstring(L, (const char *)(LPCTSTR)strRet, strRet.GetLength());
	lua_pushlstring(L, (const char *)(LPCTSTR)strNewCookie, strNewCookie.GetLength());
	return 2;
}

//0,1,2
int getcurrenttime(lua_State *L)
{
	CString strRet;
	int nMethod = 0;

	int n = lua_gettop(L);
	if ( n>0 ){
		if ( lua_isnumber(L,1) ){
			nMethod = (int)lua_tonumber(L,1);
		}
	}

	switch ( nMethod )
	{
	case 0:
		{
			CTime time = CTime::GetCurrentTime();
			lua_pushnumber(L, time.GetYear());
			lua_pushnumber(L, time.GetMonth());
			lua_pushnumber(L, time.GetDay());
			lua_pushnumber(L, time.GetHour());
			lua_pushnumber(L, time.GetMinute());
			lua_pushnumber(L, time.GetSecond());
			lua_pushnumber(L, time.GetDayOfWeek());
			return 7;
		}
		break;
	case 1:
		lua_pushnumber(L, (lua_Number)Star::Common::GetCurrentTimeTotalSeconds());
		return 1;
		break;
	case 2:
		lua_pushnumber(L, (lua_Number)Star::Common::GetCurrentTimeTotalMiliSeconds());
		return 1;
		break;
	}

	return 0;
}


int trim(lua_State* L)
{
	CStringA str;

	int n = lua_gettop(L);
	if ( n>0 ){
		if ( lua_isstring(L,1) ){
			str = lua_tostring(L,1);
			str.Trim();
		}
	}

	lua_pushstring(L,str);
	return 1;
}

int	run(lua_State* L)
{
	BOOL bRet = FALSE;
	CStringA strCmdLine;
	int n = lua_gettop(L);
	if ( n>0 ){
		if ( lua_isstring(L,1) ){
			strCmdLine = lua_tostring(L,1);
		}
		STARTUPINFO si = { sizeof(si) };
		PROCESS_INFORMATION pi;
		bRet = ::CreateProcess(NULL,(LPSTR)(LPCSTR)strCmdLine,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}

	lua_pushboolean(L,bRet);
	lua_pushstring(L,Star::Common::FormatLastError());
	return 2;
}

int runbat(lua_State *L)
{
	BOOL bRet = FALSE;
	CString strBatContent;
	CString strWorkPath;
	LPCTSTR lpszWorkPath = NULL;
	DWORD dwMilliseconds = INFINITE;

	int n = lua_gettop(L);
	if ( n < 1 ) {
		lua_pushboolean(L, FALSE);
		return 1;
	}

	if ( n >= 1 ) {
		if ( lua_isstring(L, 1) ) {
			strBatContent = lua_tostring(L, 1);
		}
	}
	if ( n >= 2 ) {
		if ( lua_isstring(L, 2) ) {
			strWorkPath = lua_tostring(L, 2);
		}
	}
	if ( n >= 3 ) {
		if ( lua_isnumber(L, 3) ) {
			dwMilliseconds = (DWORD)lua_tonumber(L, 3);
		}
	}

	if ( strWorkPath.IsEmpty()==FALSE ) {
		lpszWorkPath = strWorkPath;
	}
	bRet = Star::Common::RunBat(strBatContent, lpszWorkPath, dwMilliseconds);
	lua_pushboolean(L, bRet);
	return 1;
}

int reversefind(lua_State* L)
{
	int n = lua_gettop(L);
	if ( n<2 ) {
		return 0;
	}

	CString str;
	CString strSub;

	if ( lua_isstring(L,1) ) {
		str = lua_tostring(L,1);
	}
	if ( lua_isstring(L,2) ) {
		strSub = lua_tostring(L,2);
	}

	if ( strSub.GetLength()==0 ) {
		return 0;
	}

	int nPos = str.ReverseFind(strSub.GetAt(0));
	if ( nPos==-1 ) {
		return 0;
	}

	lua_pushnumber(L,nPos+1);
	return 1;
}

int getimagesize(lua_State* L)
{
	CString strFileName;
	int w = -1;
	int h = -1;

	int n = lua_gettop(L);
	if ( n>0 ) {
		if ( lua_isstring(L,1) ) {
			strFileName = lua_tostring(L,1);
		}

		CImage img;
		if ( img.Load(strFileName)==S_OK ) {
			w = img.GetWidth();
			h = img.GetHeight();
		}
	}

	lua_pushnumber(L,w);
	lua_pushnumber(L,h);
	return 2;
}

int getfilesize(lua_State* L)
{
	CString strFileName;
	LARGE_INTEGER llSize;
	llSize.LowPart = llSize.HighPart = 0;

	int n = lua_gettop(L);
	if ( n>0 ) {
		if ( lua_isstring(L,1) ) {
			strFileName = lua_tostring(L,1);
		}

		HANDLE hFile = CreateFile(strFileName,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,NULL,NULL);
		if ( hFile!=INVALID_HANDLE_VALUE ){
			GetFileSizeEx(hFile,&llSize);
			CloseHandle(hFile);
		}
	}

	lua_pushnumber(L,llSize.LowPart);
	lua_pushnumber(L,llSize.HighPart);
	return 2;
}

//////////////////////////////////////////////////////////////////////////
#ifdef _USRDLL
//////////////////////////////////////////////////////////////////////////

int help(lua_State *L) {
	string strUsage = "star functions:\n\n";
	int nCount = _countof(starlib);

	for ( int i = 0; i < nCount; ++i ) {
		if ( starlib[i].name!=NULL ) {
			strUsage.append("star.");
			strUsage.append(starlib[i].name);
			strUsage.append("\n");
		}
	}//endfor

	lua_pushstring(L, strUsage.c_str());
	return 1;
}

int test(lua_State *L) {
	//CZipHelperEx zip;
	//zip.Add("C:\\Users\\hzzhuxingxing\\Desktop\\zip_utils_src.zip", "C:\\Users\\hzzhuxingxing\\Desktop\\ximalaya.lua");
	return 0;
}

//参考:luaB_print
int dbgprint(lua_State *L) { 
	std::string str;

	int n = lua_gettop(L);  /* number of arguments */
	int i = 1;
	lua_getglobal(L, "tostring");
	for (i=1; i<=n; ++i) {
		const char *s;
		lua_pushvalue(L, -1);  /* function to be called */
		lua_pushvalue(L, i);   /* value to print */
		lua_call(L, 1, 1);
		s = lua_tostring(L, -1);  /* get result */
		if (s == NULL)
			return luaL_error(L, LUA_QL("tostring") " must return a string to "
			LUA_QL("dbgprint"));
		if (i>1) str.append("\t");

		str.append(s);
		lua_pop(L, 1);  /* pop result */
	}

	::OutputDebugStringA(str.c_str());

	return 0;
}

//获取应用程序启动路径，带斜杠
int getstartpath(lua_State *L)
{
	TCHAR szTemp[MAX_PATH*4]={0};
	GetModuleFileName(NULL,szTemp,sizeof(szTemp)/sizeof(TCHAR));
	_tcsrchr(szTemp,'\\')[1]=0;

	lua_pushstring(L,szTemp);
	return 1;
}

//获取当前lua脚本的路径,返回3个结果:脚本路径,脚本全文件名,脚本文件名
//参考:http://cjbskysea.blogbus.com/logs/68708391.html
int getluapath(lua_State *L)
{
	CString strFilePath = GetLuaFilePath(L);

	int nPos=strFilePath.ReverseFind('\\');
	if ( nPos!=-1 ){
		CString strName=strFilePath.Mid(nPos+1);
		CString strDir=strFilePath.Left(nPos+1);
		lua_pushstring(L,(const char *)(LPCTSTR)strDir);
		lua_pushstring(L,(const char *)(LPCTSTR)strFilePath);
		lua_pushstring(L,(const char *)(LPCTSTR)strName);
		return 3;
	}
	return 0;
}

int gettemppath(lua_State *L)
{
	CString strRet;

	strRet = Star::Common::GetSysTempPath();
	lua_pushstring(L,strRet);
	return 1;
}

int gettempfilepath(lua_State *L)
{
	CString strRet;

	strRet = Star::Common::GetSysTempFileName();

	lua_pushstring(L,strRet);
	return 1;
}

int getdesktoppath(lua_State *L)
{
	CString strRet;

	strRet = Star::Common::GetDesktopPath();

	lua_pushstring(L,strRet);
	return 1;
}

int createdirectory(lua_State *L)
{
	int n = lua_gettop(L);
	if ( n < 2 ) {
		lua_pushboolean(L, false);
		return 1;
	}

	CString strRoot;
	CString strSubdir;

	strRoot = lua_tostring(L, 1);
	strSubdir = lua_tostring(L, 2);
	BOOL bOK = Star::Common::CreateDirRecursively(strRoot, strSubdir);

	lua_pushboolean(L, bOK);
	return 1;
}

int deletedirectory(lua_State *L)
{
	CString strDir;
	BOOL bDelSelf = TRUE;
	int n = lua_gettop(L);
	if ( n > 0 ) {
		strDir = lua_tostring(L, 1);
	}
	if ( n > 1 ) {
		bDelSelf = lua_toboolean(L, 2);
	}

	BOOL bOK = Star::Common::DeleteDirectory(strDir);
	lua_pushboolean(L, bOK);
	return 1;
}

//参数:url,本地文件路径
//返回:true/false,errmsg
int downloadfile(lua_State *L)
{
	int n = lua_gettop(L);
	if ( n<2 ){
		lua_pushboolean(L,0);
		lua_pushstring(L,"parameter not enough");
		return 2;
	}

	if ( lua_type(L, 2)!=LUA_TSTRING ){
		lua_pushboolean(L, 0);
		lua_pushstring(L, "invalid parameter 2");
		return 2;
	}

	if ( lua_type(L, 1)!=LUA_TSTRING ){
		lua_pushboolean(L, 0);
		lua_pushstring(L, "invalid parameter 1");
		return 2;
	}

	const char *lpszUrl = lua_tostring(L, 1);
	if ( lpszUrl==NULL || strlen(lpszUrl) < 5 ){
		lua_pushboolean(L,0);
		lua_pushstring(L,"invalid parameter 1");
		return 2;
	}

	const char *lpszFile = lua_tostring(L, 2);
	if ( lpszUrl==NULL || strlen(lpszFile) < 4 ){
		lua_pushboolean(L,0);
		lua_pushstring(L,"invalid parameter 2");
		return 2;
	}

	if ( DownloadFileCachely(lpszUrl, lpszFile)==0 ){
		lua_pushboolean(L, 1);
		lua_pushstring(L, "ok");
		return 2;
	}

	lua_pushboolean(L, 0);
	lua_pushstring(L, "failed");
	return 2;
}

int copytoclipboard(lua_State *L)
{
	CString s;
	BOOL bRet = FALSE;

	int n = lua_gettop(L);
	if ( n>0 ){
		if ( lua_isstring(L,1) ){
			s = lua_tostring(L,1);
			bRet = Star::Common::CopyToClipboard(s, s.GetLength());
		}
	}

	lua_pushboolean(L, bRet);
	return 1;
}

int getclipboardtext(lua_State *L)
{
	CString s;
	BOOL bRet = Star::Common::GetClipboardText(s);
	if ( bRet==FALSE ) {
		lua_pushnil(L);
	}else{
		lua_pushstring(L, s);
	}
	return 1;
}


int md5file(lua_State *L)
{
	CString s;
	CString strMd5;

	int n = lua_gettop(L);
	if ( n>0 ){
		if ( lua_isstring(L,1) ){
			s = lua_tostring(L,1);
			strMd5 = Md5File(s);
			if ( strMd5.IsEmpty()==FALSE ) {
				lua_pushstring(L, Md5File(s));
				return 1;
			}
		}
	}

	return 0;
}


int _crc32(lua_State *L)
{
	CString strResult;
	CString strText;
	unsigned long calculate_crc = 0;


	int n = lua_gettop(L);
	if ( n > 0 ){
		if ( lua_isstring(L, 1) ){
			CString strText = lua_tostring(L, 1);
			calculate_crc = crc32(calculate_crc, (const Bytef *)(LPCSTR)strText, strText.GetLength());
			strResult.Format("%08X", calculate_crc);
			lua_pushlstring(L, strResult, strResult.GetLength());
			return 1;
		}
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////
int sendorpostmessge(lua_State *L,int nMethod)
{
	BOOL bRet = FALSE;

	HWND hWnd = NULL;
	DWORD dwMsg = WM_NULL;
	WPARAM wParam = NULL;
	LPARAM lParam = NULL;

	int n = lua_gettop(L);
	if ( n>0 ){
		if ( lua_isnumber(L,1) ){
			hWnd = HWND( PBYTE(NULL) + (DWORD)lua_tonumber(L,1) );
		}
	}
	if ( n>1 ){
		if ( lua_isnumber(L,2) ){
			dwMsg = (DWORD)lua_tonumber(L,2);
		}
	}
	if ( n>2 ){
		switch (lua_type(L,3) ) {
		case LUA_TNIL:
			wParam = NULL;
			break;
		case LUA_TNUMBER:
			wParam = WPARAM( PBYTE(NULL) + (DWORD)lua_tonumber(L,3) );
			break;
		case LUA_TBOOLEAN:
			wParam = WPARAM( PBYTE(NULL) + (DWORD)lua_toboolean(L,3) );
			break;
		case LUA_TSTRING:
			wParam = WPARAM(lua_tostring(L,3));
			break;
		case LUA_TLIGHTUSERDATA:
			wParam = WPARAM( PBYTE(NULL) + (DWORD)lua_touserdata(L,3) );
			break;
		default:
			wParam = NULL;
			break;
		}
	}
	if ( n>3 ){
		switch (lua_type(L,3) ) {
		case LUA_TNIL:
			lParam = NULL;
			break;
		case LUA_TNUMBER:
			lParam = LPARAM( PBYTE(NULL) + (DWORD)lua_tonumber(L,3) );
			break;
		case LUA_TBOOLEAN:
			lParam = LPARAM( PBYTE(NULL) + (DWORD)lua_toboolean(L,3) );
			break;
		case LUA_TSTRING:
			lParam = LPARAM(lua_tostring(L,3));
			break;
		case LUA_TLIGHTUSERDATA:
			lParam = LPARAM( PBYTE(NULL) + (DWORD)lua_touserdata(L,3) );
			break;
		default:
			lParam = NULL;
			break;
		}
	}

	if ( hWnd!=NULL && dwMsg!=WM_NULL ){
		if ( nMethod==0 ){
			bRet = ::PostMessageA(hWnd,dwMsg,wParam,lParam);
		}else{
			bRet = (DWORD)::SendMessageA(hWnd,dwMsg,wParam,lParam);
		}
	}

	lua_pushboolean(L,bRet);
	return 1;
}

int postmessage(lua_State *L)
{
	return sendorpostmessge(L,0);
}

int sendmessage(lua_State *L)
{
	return sendorpostmessge(L,1);
}

int loadlibrary(lua_State *L)
{
	HMODULE hModule = NULL;
	CString strModuleName;
	int n = lua_gettop(L);
	if ( n>0 ){
		if ( lua_isstring(L,1) ){
			strModuleName = lua_tostring(L,1);
		}
	}

	if ( strModuleName.IsEmpty()==FALSE ){
		hModule = GetModuleHandleA(strModuleName);
		if ( hModule==NULL ){
			hModule = LoadLibraryA(strModuleName);
		}
	}

	lua_pushnumber(L,(DWORD)hModule);
	return 1;
}

static int CallApi(lua_State* L)
{
	int n = lua_gettop(L);
	FARPROC fc=(FARPROC)lua_touserdata(L,lua_upvalueindex(1));
	if ( fc==NULL ){
		lua_pushinteger(L,0);
		return 1;
	}

	DWORD dwRet = 0;
	DWORD dwParam = 0;

	for ( int i = n; i > 0; --i ) {
		switch (lua_type(L,i) ) {
		case LUA_TNIL:
			__asm{
				push 0;
			}
			break;
		case LUA_TNUMBER:
			dwParam=(DWORD)lua_tointeger(L,i);
			__asm{
				push dwParam;
			}
			break;
		case LUA_TBOOLEAN:
			dwParam=(DWORD)lua_toboolean(L,i);
			__asm{
				push dwParam;
			}
			break;
		case LUA_TSTRING:
			dwParam=(DWORD)lua_tostring(L,i);
			__asm{
				push dwParam;
			}
			break;
		case LUA_TLIGHTUSERDATA:
			dwParam=(DWORD)lua_touserdata(L,i);
			__asm{
				push dwParam;
			}
			break;
		default:
			lua_pushstring(L,"unknown argument type");
			lua_error(L);
			break;
		}
	}
	__asm{
		call fc;
		mov dwRet,eax;
	}

	lua_pushinteger(L,dwRet);
	return 1;
}

int getprocaddress(lua_State* L)
{
	FARPROC func = NULL;
	DWORD dwModule = 0;
	string strFuncName;

	int n = lua_gettop(L);
	if ( n>1 ){
		if ( lua_isnumber(L,-2) ){
			dwModule = (DWORD)lua_tonumber(L,-2);
		}
		if ( lua_isstring(L,-1) ){
			strFuncName = lua_tostring(L,-1);
		}

		func = GetProcAddress((HMODULE)(PBYTE(NULL)+dwModule),strFuncName.c_str());
	}

	lua_pushlightuserdata(L,func);
	lua_pushcclosure(L,CallApi,1);
	return 1;
}


int keypress(lua_State* L)
{
	int nKey = VK_END;
	int nRepeatCnt = 1;

	int n = lua_gettop(L);
	if ( n>=1 ){
		if ( lua_isnumber(L,1) ){
			nKey = (int)lua_tonumber(L,1);
		}
	}
	if ( n>=2 ){
		if ( lua_isnumber(L,2) ){
			nRepeatCnt = (int)lua_tonumber(L,2);
		}
	}

	for ( int i = 0; i < nRepeatCnt; ++i ) {
		keybd_event(nKey,0,0,0); 
		keybd_event(nKey,0,KEYEVENTF_KEYUP,0); 
	}//endfor

	return 0;
}


int writeunicodestring(lua_State* L)
{
	BOOL bRet = FALSE;
	CString strFileName;
	CStringW strText;

	int n = lua_gettop(L);
	if ( n>0 ) {
		if ( lua_isstring(L,1) ) {
			strFileName = lua_tostring(L,1);
		}
	}
	if ( n>1 ) {
		if ( lua_isstring(L,2) ) {
			strText = lua_tostring(L,2);
		}
	}

	CFile file;
	const unsigned char LeadBytes[] = {0xFF, 0xFE};
	UINT nOpenFlags = CFile::modeCreate|CFile::modeReadWrite;
	if ( GetFileAttributes(strFileName)!=-1 ) {
		nOpenFlags = CFile::modeReadWrite;
	}
	if ( file.Open(strFileName,nOpenFlags) ) {
		file.SetLength(0);
		file.Write(LeadBytes,sizeof(LeadBytes));
		file.Write(strText,strText.GetLength()*sizeof(WCHAR));
		file.Close();
		bRet = TRUE;
	}

	lua_pushboolean(L,bRet);
	return 1;
}

int renamefile(lua_State* L)
{
	BOOL bRet = FALSE;
	CString strErrorMsg;
	CString strExistingFileName;
	CString strNewFileName;

	int n = lua_gettop(L);
	if ( n>=2 ) {
		if ( lua_isstring(L,1) ) {
			strExistingFileName = lua_tostring(L,1);
		}
		if ( lua_isstring(L,2) ) {
			strNewFileName = lua_tostring(L,2);
		}
		bRet = ::MoveFileEx(strExistingFileName,strNewFileName,MOVEFILE_REPLACE_EXISTING);
		strErrorMsg = Star::Common::FormatLastError(GetLastError());
		lua_pushboolean(L, bRet);
		lua_pushlstring(L, (const char *)(LPCTSTR)strErrorMsg, strErrorMsg.GetLength());
		return 2;
	}

	lua_pushboolean(L, bRet);
	return 0;
}

//定位一个文件
int locatefile(lua_State *L)
{
	CString strFileName;
	int nshowcmd = SW_SHOWNORMAL;
	BOOL bRet = TRUE;

	int n = lua_gettop(L);
	if ( n>0 ){
		if ( lua_isstring(L,1) ){
			strFileName = lua_tostring(L,1);
		}
	}
	bRet = Star::Common::LocateFile(strFileName);

	lua_pushboolean(L,bRet);
	return 1;
}

int readtextfile(lua_State *L)
{
	CString strText;
	CString strFilePath;

	int n = lua_gettop(L);
	if ( n < 1 ) {
		return 0;
	}

	if ( n >= 1 ) {
		if ( lua_isstring(L, 1) ) {
			strFilePath = lua_tostring(L, 1);
		}
	}

	if ( Star::Common::ReadTextFile(strFilePath, strText)==TRUE ) {
		lua_pushstring(L, strText);
		return 1;
	}

	return 0;
}
//////////////////////////////////////////////////////////////////////////
#endif
//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Common.h"
#include <afxwin.h>
#include <ShlObj.h>
#include <atlimage.h>
#include <sys/timeb.h>
#include <IPHlpApi.h>

//#include <atltime.h>
//#include <ShellAPI.h>
//#include <shlwapi.h>
//#pragma comment(lib, "wsock32.lib")
//#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Iphlpapi.lib")

#pragma warning(disable:4996)


typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
typedef BOOL (WINAPI *LPFN_Wow64DisableWow64FsRedirection) (PVOID* OldValue);
typedef BOOL (WINAPI *LPFN_Wow64RevertWow64FsRedirection) (PVOID OldValue);


void _Trace(const CHAR* lpszFormat, ...)
{
	if( lpszFormat ){
		va_list vlArgs;
		CHAR tzText[1024];
		va_start(vlArgs, lpszFormat);
		if(_vscprintf(lpszFormat,vlArgs) < 1024){
			vsprintf(tzText, lpszFormat, vlArgs);
		}else{
			strcpy_s(tzText, 1024, "_Trace: too long...");
		}
		OutputDebugStringA(tzText);
		va_end(vlArgs);
	}
}

void _TraceW(const WCHAR* lpszFormat, ...)
{
	if( lpszFormat ){
		va_list vlArgs;
		WCHAR tzText[1024];
		va_start(vlArgs, lpszFormat);
		if(_vscwprintf(lpszFormat,vlArgs) < 1024){
			vswprintf_s(tzText, 1024, lpszFormat, vlArgs);
		}else{
			wcscpy_s(tzText, 1024, L"_Trace: too long...");
		}
		OutputDebugStringW(tzText);
		va_end(vlArgs);
	}
}

LONG Star::Common::GetRegKey(HKEY key, LPCTSTR subkey, LPTSTR retdata)
{
	HKEY hkey;
	LONG retval = RegOpenKeyEx(key, subkey, 0, KEY_QUERY_VALUE, &hkey);

	if (retval == ERROR_SUCCESS) {
		long datasize = MAX_PATH;
		TCHAR data[MAX_PATH];
		RegQueryValue(hkey, NULL, data, &datasize);
		lstrcpy(retdata,data);
		RegCloseKey(hkey);
	}

	return retval;
}

LONG Star::Common::GetRegValueDWord(HKEY key, LPCTSTR subkey, LPCTSTR szValue,DWORD*pData)
{
	HKEY hkey;
	LONG retval = RegOpenKeyEx(key, subkey, 0, KEY_QUERY_VALUE, &hkey);

	if (retval == ERROR_SUCCESS) {
		DWORD dwType=REG_DWORD;
		DWORD dwDataSize=sizeof(DWORD);
		retval=RegQueryValueEx(hkey, szValue,NULL,&dwType, (LPBYTE)pData, &dwDataSize);
		RegCloseKey(hkey);
	}

	return retval;
}

LONG Star::Common::SetRegValueDWord(HKEY key, LPCTSTR subkey, LPCTSTR szValue,DWORD dwData,BOOL bForce)
{
	HKEY hkey;
	LONG retval;

	if ( bForce==TRUE ){
		TCHAR szString[25];
		DWORD dwordbuffer;
		retval = RegCreateKeyEx(key,subkey,NULL,szString,REG_OPTION_NON_VOLATILE,
			KEY_ALL_ACCESS,NULL,&hkey,&dwordbuffer); 
	}else{
		retval = RegOpenKeyEx(key, subkey, 0, KEY_WRITE, &hkey);
	}

	if (retval == ERROR_SUCCESS) {
		DWORD dwTemp=dwData;
		retval=RegSetValueEx(hkey,szValue,0,REG_DWORD,(LPBYTE)&dwTemp,sizeof(DWORD));
		RegCloseKey(hkey);
	}

	return retval;
}

HINSTANCE Star::Common::OpenUrl(LPCTSTR url, int showcmd)
{
	TCHAR key[MAX_PATH + MAX_PATH];

	// First try ShellExecute()
	HINSTANCE result = ShellExecute(NULL, _T("open"), url, NULL,NULL, showcmd);

	// If it failed, get the .htm regkey and lookup the program
	if ( result<=(HINSTANCE)HINSTANCE_ERROR ) {
		if ( GetRegKey(HKEY_CLASSES_ROOT, _T(".htm"), key) == ERROR_SUCCESS ) {
			lstrcat(key, _T("\\shell\\open\\command"));
			if ( GetRegKey(HKEY_CLASSES_ROOT,key,key) == ERROR_SUCCESS ) {
				TCHAR *pos = _tcsstr(key, _T("\"%1\""));
				if (pos == NULL) {                     // No quotes found
					pos = _tcsstr(key, _T("%1"));       // Check for %1, without quotes 
					if (pos == NULL){                   // No parameter at all...
						pos = key+lstrlen(key)-1;
					}else{
						*pos = _T('\0');                   // Remove the parameter
					}
				}else{
					*pos = _T('\0');                       // Remove the parameter
				}

				lstrcat(pos, _T(" "));
				lstrcat(pos, url);

				result = (HINSTANCE)NULL + WinExec((LPCSTR)key,showcmd);
			}
		}
	}

	return result;
}

CString Star::Common::GetStartPath()
{
	TCHAR szTemp[MAX_PATH];
	GetModuleFileName(NULL, szTemp, sizeof(szTemp) / sizeof(TCHAR));
	_tcsrchr(szTemp, '\\')[1] = 0;
	return szTemp;
}

CString Star::Common::GetAppName()
{
	TCHAR szTemp[MAX_PATH];
	GetModuleFileName(NULL, szTemp, sizeof(szTemp) / sizeof(TCHAR));
	return (_tcsrchr(szTemp, '\\') + 1);
}

BOOL Star::Common::IsWow64()
{
	BOOL bIsWow64 = FALSE;
	TCHAR szkernel32[] = { 'k', 'e', 'r', 'n', 'e', 'l', '3', '2', '\0' };
	CHAR szIsWow64Process[] = { 'I', 's', 'W', 'o', 'w', '6', '4', 'P', 'r', 'o', 'c', 'e', 's', 's', '\0' };

	LPFN_ISWOW64PROCESS 
		fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
		GetModuleHandle(szkernel32),szIsWow64Process);

	if (NULL != fnIsWow64Process)
	{
		if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
		{
			// handle error
		}
	}
	return bIsWow64;
}

CString Star::Common::GetMyTempPath()
{
	TCHAR szTemp[MAX_PATH];
	GetModuleFileName(NULL,szTemp,sizeof(szTemp)/sizeof(TCHAR));
	lstrcpy( _tcsrchr(szTemp,'\\')+1, _T("tmp\\") );
	if ( GetFileAttributes(szTemp)==-1 ){
		CreateDirectory(szTemp,NULL);
	}
	return szTemp;
}

//获取系统临时目录
CString Star::Common::GetSysTempPath()
{
	TCHAR szPath[MAX_PATH]={0};

	::GetTempPath( _countof(szPath), szPath );
	//::GetTempFileName(szPath,_T("tmp"),0,szFileName);
	return szPath;
}

//获取系统临时文件名
CString Star::Common::GetSysTempFileName(LPCTSTR lpszExt)
{
	TCHAR szPath[MAX_PATH] = {0};
	TCHAR szFileName[MAX_PATH] = {0};

	::GetTempPath(_countof(szPath), szPath);
	::GetTempFileName(szPath, lpszExt, 0, szFileName);
	return szFileName;
}

CString Star::Common::GetModuleFilePath()
{
	TCHAR szTemp[MAX_PATH*2];
	GetModuleFileName(NULL,szTemp,sizeof(szTemp)/sizeof(TCHAR));
	return szTemp;
}

CString Star::Common::GetWindowsPath()
{
	TCHAR str[MAX_PATH] = {};
	int nLen = GetWindowsDirectory(str, MAX_PATH);
	if ( nLen ){
		str[nLen]='\\';
	}
	return str;
}

CString Star::Common::GetSystemPath()
{
	TCHAR str[MAX_PATH]={0};
	int nLen = GetSystemDirectory(str, MAX_PATH);
	if ( nLen ){
		str[nLen]='\\';
	}
	return str;
}

CString Star::Common::GetDesktopPath()
{
	TCHAR szPath[MAX_PATH] = {0};
	SHGetSpecialFolderPath(0, szPath, CSIDL_DESKTOPDIRECTORY, FALSE );
	return szPath;
}

CString Star::Common::GetMypicturesPath()
{
	TCHAR szPath[MAX_PATH] = {0};
	SHGetSpecialFolderPath(0, szPath, CSIDL_COMMON_PICTURES, FALSE );
	return szPath;
}

/************************************************************************/  
/* 函数说明：释放资源中某类型的文件                                      
/* 参    数：新文件名、资源ID、资源类型                                  
/* 返 回 值：成功返回TRUE，否则返回FALSE   
/* By:Koma   2009.07.24 23:30                               
/************************************************************************/  
BOOL Star::Common::ReleaseRes(LPCTSTR szFileName, DWORD dwResID, LPCTSTR lpType)  
{  
	// 资源大小  
	DWORD dwWritten = 0; 
	BOOL bResult = FALSE;

	// 创建文件  
	HANDLE hFile=CreateFile( szFileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );  
	if ( hFile==INVALID_HANDLE_VALUE ){  
		return (::GetFileAttributes(szFileName)!=-1); 
	}  

	// 查找资源文件中、加载资源到内存、得到资源大小  
	HRSRC   hrsc = FindResource(NULL, MAKEINTRESOURCE(dwResID), lpType);  
	HGLOBAL hG = LoadResource(NULL, hrsc);  
	DWORD   dwSize = SizeofResource( NULL,  hrsc);  

	// 写入文件
	if ( hG && dwSize>0 ){	  
		WriteFile(hFile,hG,dwSize,&dwWritten,NULL);
		bResult = TRUE;
	}
	CloseHandle( hFile );

	return bResult;  
}


#pragma comment(lib,"Rpcrt4.lib")
int Star::Common::GenerateGUID(CString& sGUID)
{
	int status = 1;
	sGUID.Empty();

	// Create GUID

	WCHAR *pszUuid = 0; 
	GUID *pguid = NULL;
	pguid = new GUID;
	if(pguid!=NULL){
		HRESULT hr = CoCreateGuid(pguid);
		if(SUCCEEDED(hr)){
			// Convert the GUID to a string
			hr = UuidToStringW(pguid, (RPC_WSTR *)&pszUuid);
			if(SUCCEEDED(hr) && pszUuid!=NULL){ 
				status = 0;
				sGUID = pszUuid;
				RpcStringFreeW((RPC_WSTR *)&pszUuid);
			}
		}
		delete pguid; 
	}

	return status;
}

CString Star::Common::utf8s2ts(const char*lpszText, int nSize/* = -1*/)
{
	CString strResult;
	CStringW strTextW;
	if ( lpszText==NULL ) {
		return strResult;
	}

	int len = MultiByteToWideChar(CP_UTF8, 0, lpszText, nSize, NULL,0);
	WCHAR * wszUnicode = new WCHAR[len+1];
	memset(wszUnicode, 0, len * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, lpszText, nSize, wszUnicode, len);
	strTextW = wszUnicode;
	delete[] wszUnicode;
#ifdef _UNICODE
	return strTextW;
#else
	strResult =strTextW;
	return strResult;
#endif
}

CStringA Star::Common::ts2utf8s(const CString&str)
{
	CStringW strW;
	LPCWSTR lpwstrSrc = NULL;
	int cchWideChar = 0;
	CStringA strResultA;

#ifdef _UNICODE
	lpwstrSrc = str;
	cchWideChar = str.GetLength();
#else
	strW = str;
	lpwstrSrc = strW;
	cchWideChar = strW.GetLength();
#endif

	char *pBuff = NULL;
	int nLen = WideCharToMultiByte(CP_UTF8, 0, lpwstrSrc, cchWideChar, NULL, 0, NULL, NULL);
	if ( nLen > 0 ){
		pBuff = new char[nLen + 1];
		if ( pBuff!=NULL ){
			WideCharToMultiByte(CP_UTF8, 0, lpwstrSrc, cchWideChar, pBuff, nLen, NULL, NULL);
			pBuff[nLen] = 0;
			strResultA = pBuff;
			delete[] pBuff;
		}
	}

	return strResultA;
}

//一个字符串将要作为文件名，去除这个字符串中非法的字符
//可能返回为空串，也可能这个字符串很合法但是太长了，这两条需要外部处理
CString Star::Common::filterinvalidfilename(CString strText)
{
	if ( strText.IsEmpty()==TRUE ){
		return strText;
	}

	CStringW strFileNameW;
	strFileNameW=strText;
	strFileNameW.Replace(L"\r",L"");
	strFileNameW.Replace(L"\n",L"");
	strFileNameW.Replace(L"\t",L"");
	strFileNameW.Replace(L"\\",L"");
	strFileNameW.Replace(L"/",L"");
	strFileNameW.Replace(L":",L"");
	strFileNameW.Replace(L"*",L"");
	strFileNameW.Replace(L"?",L"");
	strFileNameW.Replace(L"\"",L"");
	strFileNameW.Replace(L"<",L"");
	strFileNameW.Replace(L">",L"");
	strFileNameW.Replace(L"|",L"");
	while ( strFileNameW.IsEmpty()==FALSE && strFileNameW.Right(1)=="." ){
		strFileNameW=strFileNameW.Left(strFileNameW.GetLength()-1);
	}
	strFileNameW.Trim();

	return (CString)strFileNameW;
}

//转义网页中的一些字符UrlEscape和UrlUnescape只能转换%的转义字符，如&-%26 空格-%20
//参考：http://www.cnblogs.com/anjou/archive/2007/03/15/676476.html
void Star::Common::unescapexml(CString&strText)
{
	strText.Replace(_T("&amp;"),_T("&"));
	strText.Replace(_T("&lt;"),_T("<"));
	strText.Replace(_T("&gt;"),_T(">"));
	strText.Replace(_T("&quot;"),_T("\""));
	strText.Replace(_T("&apos;"),_T("\'"));
	strText.Replace(_T("&nbsp;"),_T(" "));
	strText.Replace(_T("&copy;"),_T(" "));
	strText.Replace(_T("&reg;"),_T(" "));
	strText.Replace(_T("&#47;"),_T("/"));
	strText.Replace(_T("&#60;"),_T("<"));
	strText.Replace(_T("&#62;"),_T(">"));
	strText.Replace(_T("&#38;"),_T("&"));
	strText.Replace(_T("&#039;"),_T("\'"));
}


//解析\u65F6\u5149\u5F71\u50CF\u9986
//参考jsoncpp中Reader::decodeUnicodeEscapeSequence
CString Star::Common::unescapeunicode(const char *lpszText)
{
	CString strResult;
	CStringW strTemp;
	if ( lpszText==NULL ){
		return strResult;
	}

	WCHAR unicode = 0;
	const char *pCurrent = lpszText;
	while ( *pCurrent ){
		if ( *pCurrent=='\\' ){
			switch ( pCurrent[1] )
			{
			case 'u':
				{
					pCurrent+=2;
					unicode = 0;
					for ( int index =0; index < 4; ++index ){
						char c = *pCurrent++;
						unicode *= 16;
						if ( c >= '0'  &&  c <= '9' )
							unicode += c - '0';
						else if ( c >= 'a'  &&  c <= 'f' )
							unicode += c - 'a' + 10;
						else if ( c >= 'A'  &&  c <= 'F' )
							unicode += c - 'A' + 10;
						else
							return strResult;
					}

					strTemp = unicode;
					strResult += strTemp;
				}
				break;
			case '"': 
			case '/': 
			case '\\':
			case 'b': 
			case 'f': 
			case 'n': 
			case 'r': 
			case 't':
				strResult.AppendChar(pCurrent[1]);
				pCurrent+=2;
				break;
			default:
				strResult.AppendChar(*pCurrent);
				++pCurrent;
				break;
			}

			
		}else{
			strResult.AppendChar(*pCurrent);
			++pCurrent;
		}
	}

	return strResult;
}

//转换百度账号中的中文字符，注意如果是中文和英文的混合则英文不变
CString Star::Common::GBKEncodeURI(const CString&strText)
{
	CString strResult;
	CString strTemp;
	TCHAR ch;
	for ( int i=0; i<strText.GetLength(); ++i ){
		ch = strText.GetAt(i);
		if ( ch<0 ){
			strTemp.Format(_T("%X"), (unsigned char)ch);
			strResult += _T("%") + strTemp;
		}else{
			strResult+=ch;
		}
	}

	return strResult;
}

//转换百度账号中的中文字符，一个中文字符转换为三个%。注意如果是中文和英文的混合则英文不变
CString Star::Common::UTF8EncodeURI(const CString&str)
{
	if ( str.IsEmpty()==TRUE ){
		return str;
	}

	CString strResult;
	CString strTemp;
	TCHAR ch;

	CStringA strText = Star::Common::ts2utf8s(str);
	for ( int i=0; i<strText.GetLength(); ++i ){
		ch = strText.GetAt(i);
		if ( ch<0 ){
			strTemp.Format(_T("%X"), (unsigned char)ch);
			strResult+=_T("%")+strTemp;
			++i;
			ch = strText.GetAt(i);
			strTemp.Format(_T("%X"), (unsigned char)ch);
			strResult+=_T("%")+strTemp;
			++i;
			ch = strText.GetAt(i);
			strTemp.Format(_T("%X"), (unsigned char)ch);
			strResult+=_T("%")+strTemp;

		}else{
			strResult+=ch;
		}
	}

	return strResult;
}

BOOL Star::Common::IsSignalToStop(HANDLE hEvent)
{
	return ( ::WaitForSingleObject(hEvent,0)!=WAIT_OBJECT_0 );
}

void Star::Common::InitializeSeed()  
{  
	srand((unsigned)time(0));  
} 

//产生一个随机数范围：[Low，High]
int Star::Common::RandIntNum(int Low,int High)    
{  
	if(Low > High){  
		int nTemp=Low;
		Low=High;
		High=nTemp;
	}  

	int IntervalSize=High-Low+1;  
	int RandomOffset=rand()%IntervalSize;  
	return Low+RandomOffset;   
}

//产生一个0-1之间的随机浮点数字符串，小数点后面的位数由nCount定,不得超过MAX_PATH位
CString Star::Common::RandFloatNum(int nCount)
{
	CString strFloat;
	CString strText;
	if ( nCount<=0 || nCount>MAX_PATH ){
		return strFloat;
	}

	strFloat=_T("0.");
	for ( int i=0; i<nCount; ++i ){
		strText.Format( _T("%d"), Star::Common::RandIntNum(0,9) );
		strFloat+=strText;
	}

	return strFloat;
}

CString Star::Common::RandNum(int nCount)
{
	CString strFloat;
	CString strText;
	if ( nCount<=0 || nCount>MAX_PATH ){
		return strFloat;
	}

	for ( int i=0; i<nCount; ++i ){
		strText.Format( _T("%d"), Star::Common::RandIntNum(0,9) );
		strFloat+=strText;
	}

	return strFloat;
}
//
//CString Star::Common::GetCpuId()
//{
//	unsigned long s0a,s0b,s0c,s0d;     
//	unsigned long s1a,s1b,s1c,s1d;     
//	unsigned long s2a,s2b,s2c,s2d;     
//	unsigned long s3a,s3b,s3c,s3d;     
//	unsigned long cpuid1,cpuid2;     
//	CString strCpuID;   
//	__asm{   
//		mov eax,00h   
//			cpuid   
//			mov s0a,eax   
//			mov s0b,ebx   
//			mov s0c,ecx   
//			mov s0d,edx   
//	}   
//	__asm{   
//		mov eax,01h   
//			cpuid   
//			mov s1a,eax   
//			mov s1b,ebx   
//			mov s1c,ecx   
//			mov s1d,edx   
//	}   
//	__asm{   
//		mov eax,02h   
//			cpuid   
//			mov s2a,eax   
//			mov s2b,ebx   
//			mov s2c,ecx   
//			mov s2d,edx   
//	}   
//	__asm{   
//		mov eax,03h   
//			cpuid   
//			mov s3a,eax   
//			mov s3b,ebx   
//			mov s3c,ecx   
//			mov s3d,edx   
//	}   
//
//	cpuid1=s0a+s0b+s1a+s1b+s2a+s2b+s3a+s3b;
//	cpuid2=s0c+s0d+s1c+s1d+s2c+s2d+s3c+s3d;
//	strCpuID.Format(_T("%08X%08X"),cpuid1,cpuid2);
//
//
//	//////////////////////////////////////////////////////////////////////////
//	DWORD veax;
//	DWORD deax;
//	DWORD debx;
//	DWORD decx;
//	DWORD dedx;
//
//	veax=0;
//	__asm
//	{
//		mov eax, veax
//			cpuid
//			mov deax, eax
//			mov debx, ebx
//			mov decx, ecx
//			mov dedx, edx
//	}
//
//	//获得CPU的制造商信息(Vender ID String)
//	char cVID[13]={0};			// 字符串，用来存储制造商信息
//	memcpy(cVID, &debx, 4); // 复制前四个字符到数组
//	memcpy(cVID+4, &dedx, 4); // 复制中间四个字符到数组
//	memcpy(cVID+8, &decx, 4); // 复制最后四个字符到数组
//
//	//获得CPU商标信息(Brand String)
//	veax=0x80000002;
//	__asm
//	{
//		mov eax, veax
//			cpuid
//			mov deax, eax
//			mov debx, ebx
//			mov decx, ecx
//			mov dedx, edx
//	}
//	veax=0x80000003;
//	__asm
//	{
//		mov eax, veax
//			cpuid
//			mov deax, eax
//			mov debx, ebx
//			mov decx, ecx
//			mov dedx, edx
//	}	
//	veax=0x80000004;
//	__asm
//	{
//		mov eax, veax
//			cpuid
//			mov deax, eax
//			mov debx, ebx
//			mov decx, ecx
//			mov dedx, edx
//	}
//
//	return strCpuID;
//}

void Star::Common::RestartApp()
{
	TCHAR szExePath[MAX_PATH] = {0};
	GetModuleFileName(NULL,szExePath,MAX_PATH);

	CString strCmdLine;
	strCmdLine.Format(_T("\"%s\""),szExePath);

	STARTUPINFO si={};
	PROCESS_INFORMATION pi={};
	si.cb=sizeof(si);
	CreateProcess(NULL,(LPTSTR)(LPCTSTR)strCmdLine,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
	::ExitProcess(0);
}

int Star::Common::SplitString(const CString&strSrc,const CString&strSep,vector<CString>&vtStr)
{	
	vtStr.clear();
	TCHAR *pContext = NULL;

	TCHAR *pToken=_tcstok_s( (LPTSTR)(LPCTSTR)strSrc, (LPCTSTR)strSep, &pContext ); 
	while( pToken!=NULL ){ 
		vtStr.push_back(pToken);
		pToken=_tcstok_s( NULL, (LPCTSTR)strSep, &pContext );
	}

	return (int)vtStr.size();
}

//http://zhidao.baidu.com/question/272885337.html
//设置代理       
//参数分别为：代理服务器地址，端口号，用户名，密码   
//用到的变量说明：   
//sess为CInternetSession变量   
//pConnect为CHttpConnection变量   
//pFile   为CHttpFile变量   
//int Star::Common::SetProxy(CInternetSession&sess,CHttpFile*pFile,char*szproxy, char*proxyUser, char* proxyPassword)   
//{   
//	INTERNET_PROXY_INFO   proxyinfo;   
//
//	try   
//	{   
//		proxyinfo.dwAccessType=INTERNET_OPEN_TYPE_PROXY;   
//		proxyinfo.lpszProxy=szproxy;   
//		proxyinfo.lpszProxyBypass=NULL;   
//
//		if   (!   sess.SetOption(INTERNET_OPTION_PROXY,   (LPVOID)&proxyinfo,   sizeof(INTERNET_PROXY_INFO)))   
//		{   
//			return   0;   
//		}   
//
//		if   (!pFile->SetOption(INTERNET_OPTION_PROXY_USERNAME,   proxyUser,   strlen(proxyUser)   +   1))   
//		{   
//			return   0;   
//		}   
//
//		if   (!pFile->SetOption(INTERNET_OPTION_PROXY_PASSWORD,   proxyPassword,   strlen(proxyPassword)   +   1))   
//		{   
//			return   0;   
//		}   
//
//		return   1;   
//	}   
//	catch(...)   
//	{   
//		return   0;   
//	}   
//
//}

//在字符串中查找字符，pSub含有多个字符
int Star::Common::StrFindOneOf(char*pMain,char*pSub)
{
	int nRet=-1;
	char ch=0;

	for ( int i=0; ; ++i ){
		ch=pMain[i];
		if ( ch==0 ){
			break;
		}
		for ( int j=0; ; ++j ){
			if ( pSub[j]==0 ){
				break;
			}else if ( pSub[j]==ch ){
				return i;
			}
		}
	}

	return nRet;
}

//查找直到不是数字的字符，实际效率太慢啊啊啊啊，尽量少用，不要在循环中使用
int Star::Common::StrFindCharNotDigit(char*pMain)
{
	int nRet=-1;
	char ch=0;

	for ( int i=0; ; ++i ){
		ch=pMain[i];
		if ( ch==0 ){
			break;
		}

		if ( ch<'0' || ch>'9' ){
			return i;
		}
	}

	return nRet;
}


//实现关机的函数
//摘自：http://www.google.com/codesearch/p?hl=zh-CN#X_1VXq0XKpU/trunk/shareaza/Scheduler.cpp&q=ExitWindowsEx%20lang:c%2B%2B&sa=N&cd=3&ct=rc
BOOL Star::Common::SetShutdownRights()
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;

	// Get a token for this process.
	if ( !OpenProcessToken( GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken ) )
		return( FALSE );

	// Get the LUID for the shutdown privilege.
	LookupPrivilegeValue( NULL, SE_SHUTDOWN_NAME,
		&tkp.Privileges[0].Luid );

	tkp.PrivilegeCount = 1;  // One privilege to set
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	// Get the shutdown privilege for this process.
	AdjustTokenPrivileges( hToken, FALSE, &tkp, 0,
		(PTOKEN_PRIVILEGES)NULL, 0 );

	if ( GetLastError() != ERROR_SUCCESS )
		return FALSE;

	return TRUE;
}

BOOL Star::Common::ShutDownComputer(UINT ShutdownFlags)
{
	int ShutdownSuccess = ExitWindowsEx( ShutdownFlags, 
		SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_MINOR_OTHER | SHTDN_REASON_FLAG_PLANNED );
	return (ShutdownSuccess != 0);

	/*
	int ShutdownSuccess = 0;

	// Try 2000/XP way first
	ShutdownSuccess = InitiateSystemShutdownEx( NULL,_T("Shareaza Scheduled Shutdown\n\nA system shutdown was scheduled using Shareaza. The system will now shut down."), 30, FALSE, FALSE, SHTDN_REASON_FLAG_USER_DEFINED );

	// Fall back to 9x way if this does not work
	if ( !ShutdownSuccess && GetLastError() != ERROR_SHUTDOWN_IN_PROGRESS )
	{
	UINT ShutdownFlags = EWX_POWEROFF;
	DWORD dReason;
	dReason = ( SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_MINOR_OTHER | SHTDN_REASON_FLAG_PLANNED );
	ShutdownSuccess = ExitWindowsEx( ShutdownFlags, dReason );
	}
	return (ShutdownSuccess != 0);
	*/
}

CString Star::Common::GetCurrentTimeStr(LPCTSTR lpszFormat/* = NULL*/)
{
	LPCTSTR lpszFormatInternel = lpszFormat;
	if ( lpszFormatInternel==NULL ) {
		lpszFormatInternel = _T("%Y%m%d_%H%M%S");
	}

	//COleDateTime::GetCurrentTime().Format(_T("%Y-%m-%d_%H-%M-%S"));
	CTime time = CTime::GetCurrentTime();
	CString str=time.Format(lpszFormatInternel);
	return str;
}

LONGLONG Star::Common::GetCurrentTimeTotalSeconds()
{
	char szBuff[MAX_PATH] = {0};
	CTime time = CTime::GetCurrentTime();
	return time.GetTime();
}

LONGLONG Star::Common::GetCurrentTimeTotalMiliSeconds()
{
	CString strText;
	CString strTemp;
	//char szBuff[MAX_PATH] = {0};

	struct timeb t1;
	ftime(&t1);

	//strText = _i64toa(t1.time,szBuff,10);
	//strTemp.Format("%d",t1.millitm);

	return t1.time * 1000 + t1.millitm;
}


BOOL Star::Common::MsgSleep(IN DWORD dwMillseconds)
{
	MSG msg;
	UINT_PTR nTimerID=SetTimer(NULL,NULL,dwMillseconds,NULL);
	while(GetMessage(&msg,NULL,NULL,NULL))
	{
		if(msg.message==WM_TIMER && msg.wParam==nTimerID)
		{
			KillTimer(NULL,nTimerID);
			return TRUE;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return FALSE;
}
//////////////////////////////////////////////////////////////////////////

#if 0


/*------------------------------------------------------------------------
[7/24/2009 ]
说明: 提高本进程权限,以取得系统进程的信息
------------------------------------------------------------------------*/
BOOL WINAPI Star::Common::EnableDebugPrivilege(BOOL bEnable) 
{
	// 附给本进程特权，用以取得系统进程的信息
	BOOL bOk = FALSE;    // Assume function fails
	HANDLE hToken;

	// 打开一个进程的访问令牌
	if (OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, 
		&hToken)) 
	{
		TOKEN_PRIVILEGES tp;
		tp.PrivilegeCount = 1;
		LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid);
		tp.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;
		AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
		bOk = (GetLastError() == ERROR_SUCCESS);
		::CloseHandle(hToken);
	}
	return(bOk);
}

/*------------------------------------------------------------------------
[7/24/2009 ]
说明: dll远程注入到进程
------------------------------------------------------------------------*/
BOOL WINAPI Star::Common::RemoteLoadLibrary(LPCTSTR pszDllName, DWORD dwProcessId)
{
	// 试图打开目标进程
	HANDLE hProcess = ::OpenProcess(
		PROCESS_VM_WRITE|PROCESS_CREATE_THREAD|PROCESS_VM_OPERATION, FALSE, dwProcessId);
	if(hProcess == NULL)
		return FALSE;


	// 在目标进程申请空间，存放字符串pszDllName，作为远程线程的参数
	int cbSize = (::lstrlen(pszDllName) + 1);
	LPVOID lpRemoteDllName = ::VirtualAllocEx(hProcess, NULL, cbSize, MEM_COMMIT, PAGE_READWRITE);
	::WriteProcessMemory(hProcess, lpRemoteDllName, pszDllName, cbSize, NULL);

	// 取得LoadLibraryA函数的地址，我们将以它作为远程线程函数启动
	HMODULE hModule=::GetModuleHandle (_T("kernel32.dll"));
	LPTHREAD_START_ROUTINE pfnStartRoutine = 
		(LPTHREAD_START_ROUTINE)::GetProcAddress(hModule, "LoadLibraryA");


	// 启动远程线程
	HANDLE hRemoteThread = ::CreateRemoteThread(hProcess, NULL, 0, pfnStartRoutine, lpRemoteDllName, 0, NULL);
	if(hRemoteThread == NULL){
		::CloseHandle(hProcess);
		return FALSE;
	}

	::CloseHandle(hRemoteThread);
	::CloseHandle(hProcess);

	return TRUE;
}

int Star::Common::WaitData(SOCKET hSocket,DWORD dwTime)
{
	fd_set stFdSet;
	timeval stTimeval;

	stFdSet.fd_count=1;
	stFdSet.fd_array[0]=hSocket;
	stTimeval.tv_usec=dwTime;
	stTimeval.tv_sec=0;
	return select(0,&stFdSet,NULL,NULL,&stTimeval);
}

//lpszHostAddress填:"time-a.timefreq.bldrdoc.gov"或者"132.163.4.101"都可以; 外部不需要调用htons(nHostPort)
//返回0表示失败,成功返回:从1900年1月1日起的秒数
ULONG Star::Common::GetInternetTime(char *lpszHostAddress, UINT nHostPort,int nTimeOut/*=10000*/)
{
	ULONG ulTime = 0;

	WSADATA WSAData;
	::WSAStartup (MAKEWORD(2,0), &WSAData);	


	//创建套接字

	SOCKET hSocket = socket(AF_INET, SOCK_STREAM, 0);   
	if(hSocket == INVALID_SOCKET){   
		//OutputDebugString(_T("创建套接字失败\n"));
		WSACleanup();
		return ulTime;   
	}  


	//连接服务器

	SOCKADDR_IN sockAddr;   
	memset(&sockAddr,0,sizeof(sockAddr));   

	LPSTR lpszAscii = lpszHostAddress;   
	sockAddr.sin_family = AF_INET;   
	sockAddr.sin_addr.s_addr = inet_addr(lpszAscii);   

	if (sockAddr.sin_addr.s_addr == INADDR_NONE){   
		LPHOSTENT lphost;   
		lphost = gethostbyname(lpszAscii);   
		if (lphost != NULL){  
			sockAddr.sin_addr.s_addr = ((LPIN_ADDR)lphost->h_addr)->s_addr;   
		}else{   
			WSACleanup();
			return ulTime;   
		}   
	}   
	sockAddr.sin_port = htons((u_short)nHostPort);   

	if( connect(hSocket,(SOCKADDR*)&sockAddr, sizeof(sockAddr)) == SOCKET_ERROR ){   
		if ( hSocket != INVALID_SOCKET ){   
			closesocket(hSocket);   
			hSocket = INVALID_SOCKET;   
		}   

		//OutputDebugString("连接服务器失败\n");
		WSACleanup();
		return ulTime;   
	}   


	//接受数据

	while ( TRUE ){
		int ret= WaitData(hSocket,nTimeOut);
		if ( ret==SOCKET_ERROR ){
			break;
		}

		if ( ret && !recv(hSocket, (LPSTR)&ulTime, sizeof(ulTime), 0) ){	//与服务器断开链接
			//OutputDebugString("服务器断开连接\n");
			break;
		}

		if (ret){	//没有退出且ret不为零，说明是正常的发送与接收
			//ProcessServerCmd
			ulTime = ::ntohl (ulTime);	//大小端转换	
		}
	}

	WSACleanup();
	return ulTime; 
}


ULONG Star::Common::GetInternetTime(int nTimeOut/*=10000*/)
{
	ULONG ulTime = 0;
	CHAR szTimeHosts[]={
		//_T("time-a.nist.gov\0")
		//_T("time-b.nist.gov\0")
		"time-a.timefreq.bldrdoc.gov\0"
		//_T("time-b.timefreq.bldrdoc\0")
		//_T("time-c.timefreq.bldrdoc.gov\0")
		"utcnist.colorado.edu\0"
		"time.nist.gov\0"
		//_T("time-nw.nist.gov\0")
		"nist1.datum.com\0"
		//_T("nist1-dc.glassey.com\0")
		//_T("nist1-ny.glassey.com\0")
		//_T("nist1-sj.glassey.com\0")
		//_T("nist1.aol-ca.truetime.com\0")
		//_T("nist1.aol-va.truetime.com\0")
		"\0"
	};

	CHAR *p=szTimeHosts;
	while ( *p ){
		ulTime = GetInternetTime(p,IPPORT_TIMESERVER,nTimeOut);
		if ( ulTime ){
			break;
		}

		p+=strlen(p)+1;
	}



	return ulTime; 
}
//////////////////////////////////////////////////////////////////////////



//截屏 如果hwnd为NULL则截取整个桌面，否则只截取该窗口,文件保存到szFilePath
typedef BOOL (__stdcall *TPrintWindow)(
									   HWND hwnd,               // Window to copy
									   HDC  hdcBlt,             // HDC to print into
									   UINT nFlags              // Optional flags
									   );

DWORD Star::Common::CaptureScreen(HWND hWnd, LPCTSTR sSavePath)
{
	if ( sSavePath==NULL ){
		return -1;
	}

	BOOL bSuccess=FALSE;
	DWORD dwError = 0;

	int nWidth=0;
	int nHeight=0;
	HDC hDC = NULL;
	HDC hCompatDC = NULL;
	HBITMAP hBitmap = NULL;
	RECT rcWnd = {0};
	TCHAR szUser32dll[] = {'u','s','e','r','3','2','.','d','l','l','\0'};
	CHAR szPrintWindow[] = {'P','r','i','n','t','W','i','n','d','o','w','\0'};

	HMODULE hModule = GetModuleHandle(szUser32dll);
	TPrintWindow pfnPrintWindow = (TPrintWindow)GetProcAddress(hModule,szPrintWindow);

	if ( hWnd==0 || IsIconic(hWnd) || IsWindowVisible(hWnd)==FALSE ){
		nWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
		nHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
		int nLeft = GetSystemMetrics(SM_XVIRTUALSCREEN);
		int nTop = GetSystemMetrics(SM_YVIRTUALSCREEN);

		if ( nWidth==0 || nHeight==0 ){
			dwError = -2;
		}

		hDC = ::GetDC(NULL);
		hCompatDC = CreateCompatibleDC(hDC);
		hBitmap = CreateCompatibleBitmap(hDC, nWidth, nHeight);
		SelectObject(hCompatDC, hBitmap);
		bSuccess = BitBlt(hCompatDC, 0, 0, nWidth, nHeight, hDC, nLeft, nTop, SRCCOPY|CAPTUREBLT);
		if ( bSuccess==FALSE ){
			dwError = -3;
		}
		ReleaseDC(NULL,hDC);
	}else{
		::GetWindowRect(hWnd,&rcWnd);
		nWidth=rcWnd.right - rcWnd.left;
		nHeight=rcWnd.bottom - rcWnd.top;
		if ( nWidth==0 || nHeight==0 ){
			dwError = -4;
		}

		hDC = ::GetWindowDC(hWnd);
		hCompatDC = CreateCompatibleDC(hDC);
		hBitmap = CreateCompatibleBitmap(hDC, nWidth, nHeight);
		SelectObject(hCompatDC, hBitmap);
		if ( pfnPrintWindow!=NULL ){
			bSuccess = pfnPrintWindow(hWnd, hCompatDC, 0);	//此处用PrintWindow不会被遮挡，用BitBlt截图会被遮挡的
		}

		if ( bSuccess==FALSE ){
			bSuccess = BitBlt(hCompatDC, 0, 0, nWidth, nHeight, hDC, 0, 0, SRCCOPY|CAPTUREBLT);
		}

		if ( bSuccess==FALSE ){
			dwError = -5;
		}
		ReleaseDC(hWnd,hDC);
	}

	if ( hBitmap!=NULL ){
		CImage image;
		image.Attach(hBitmap);
		image.Save(sSavePath,Gdiplus::ImageFormatJPEG);
		dwError = 0;
	}else{
		dwError = -6;
	}

	return dwError;
}

#endif


//转换路径,\Device\HarddiskVolume2\ -》 D
CString DosDevicePath2LogicalPathReal(LPCTSTR lpszDosPath)
{
	CString strResult;

	// Translate path with device name to drive letters.
	TCHAR szTemp[MAX_PATH];
	szTemp[0] = '\0';

	if ( lpszDosPath==NULL || !GetLogicalDriveStrings(_countof(szTemp)-1, szTemp) ){
		return lpszDosPath;
	}


	TCHAR szName[MAX_PATH];
	TCHAR szDrive[3] = TEXT(" :");
	BOOL bFound = FALSE;
	TCHAR* p = szTemp;

	do{
		// Copy the drive letter to the template string
		*szDrive = *p;

		// Look up each device name
		if ( QueryDosDevice(szDrive, szName, _countof(szName)) ){
			UINT uNameLen = (UINT)_tcslen(szName);

			if (uNameLen < MAX_PATH) 
			{
				bFound = _tcsnicmp(lpszDosPath, szName, uNameLen) == 0;

				if ( bFound ){
					// Reconstruct pszFilename using szTemp
					// Replace device path with DOS path
					strResult = szDrive;
					strResult += lpszDosPath+uNameLen;
					strResult.MakeLower();
				}
			}
		}

		// Go to the next NULL character.
		while (*p++);
	} while (!bFound && *p); // end of string

	//safe path
	if ( strResult.GetLength() > MAX_PATH ){
		strResult = strResult.Left(MAX_PATH);
	}

	return strResult;
}

void DosDevicePath2LogicalPathTemp(LPCTSTR lpszDosPath,CString *pOutPath)
{
	*pOutPath = DosDevicePath2LogicalPathReal(lpszDosPath);
}

void DosDevicePath2LogicalPathSafely(LPCTSTR lpszDosPath,CString *pOutPath)
{
	if ( pOutPath!=NULL ){
		__try{
			DosDevicePath2LogicalPathTemp(lpszDosPath,pOutPath);
		}__except(1){
			pOutPath->Empty();
		}
	}
}

CString Star::Common::DosDevicePath2LogicalPath(LPCTSTR lpszDosPath)
{
	CString strResult;
	DosDevicePath2LogicalPathSafely(lpszDosPath,&strResult);
	return strResult;
}


//////////////////////////////////////////////////////////////////////////

//换行要用\r\n
BOOL Star::Common::CopyToClipboard(LPCTSTR szText,int nCbSize/*=-1*/,HWND hWndNewOwner/*=NULL*/)
{
	if ( szText==NULL ){
		return FALSE;
	}

	if ( nCbSize==-1 ){
		nCbSize = lstrlen(szText)*sizeof(TCHAR);
	}

	HGLOBAL hClip = GlobalAlloc(GHND|GMEM_SHARE,nCbSize+1);
	if ( hClip==NULL ){
		return FALSE;
	}

	char *pBuff = (char*)GlobalLock(hClip);
	if( pBuff==NULL ){
		GlobalFree(hClip);
		return FALSE;
	}

	memcpy(pBuff,szText,nCbSize);
	pBuff[nCbSize] = 0;
	GlobalUnlock(hClip);

	if ( OpenClipboard(hWndNewOwner) ){
		EmptyClipboard();
		SetClipboardData( CF_TEXT,hClip);
		CloseClipboard();
	}
	
	//注意：这里不能释放，否则会每隔一次设置不成功的。
	//GlobalFree(hClip);
	return TRUE;
}

BOOL Star::Common::GetClipboardText(CString&strText)
{
	BOOL bSuccess = FALSE;
	strText.Empty();
	if ( OpenClipboard(NULL) ) {
		HANDLE hClipData = GetClipboardData(CF_TEXT);
		LPTSTR lpszText = (LPTSTR)GlobalLock(hClipData);
		if ( lpszText!=NULL ) {
			strText = lpszText;
			bSuccess = TRUE;
			GlobalUnlock(hClipData);
		}

		CloseClipboard();
	}

	return bSuccess;
}

#define OID_802_3_PERMANENT_ADDRESS        0x01010101
//#define OID_802_3_CURRENT_ADDRESS        0x01010102
#define IOCTL_NDIS_QUERY_GLOBAL_STATS    0x00170002


//Apple Mobile Device Ethernet
CString FilterInvalidMac(const CString&strMac)
{
	CString str = strMac;
	str.MakeLower();

	if ( str.Find(_T("FF:FF:FF:FF:FF:"))!=-1 || str.Find(_T("00:00:00:00:00:"))!=-1 ) {
		return _T("");
	}else{
		return strMac;
	}
}

//虚拟网卡,电脑插上手机了Apple Mobile Device Ethernet
CString FilterInvalidMacAdapter(const CString&strAdapterDescription)
{
	CString str = strAdapterDescription;
	str.MakeLower();

	if ( str.Find(_T("mobile"))!=-1 || str.Find(_T("virtual"))!=-1 || str.Find(_T("vmware"))!=-1 ) {
		return _T("");
	}else{
		return strAdapterDescription;
	}
}

CString GetMacByAdapterName(const CString&strAdapterName)
{
	int nError = 0;
	DWORD dwRet = 0;
	DWORD cbData = 0;
	CString strMac;
	CString strDriverName;
	TCHAR szFormat[] = { '%', '0', '2', 'X', '%', '0', '2', 'X', '%', '0', '2', 'X', '%', '0', '2', 'X', '%', '0', '2', 'X', '%', '0', '2', 'X', '\0' };

	strDriverName = _T("\\\\.\\") + strAdapterName;
	HANDLE hFile = CreateFile(strDriverName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
	if ( hFile!=INVALID_HANDLE_VALUE ){
		DWORD dwInBuff = OID_802_3_PERMANENT_ADDRESS;
		BYTE outBuff[MAX_PATH] = {0};
		dwRet = DeviceIoControl(hFile,IOCTL_NDIS_QUERY_GLOBAL_STATS,&dwInBuff,sizeof(dwInBuff),outBuff,sizeof(outBuff),&cbData,NULL);

		//无论成功失败关闭文件句柄
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;

		if ( dwRet ){
			strMac.Format(szFormat,outBuff[0],outBuff[1],outBuff[2],outBuff[3],outBuff[4],outBuff[5]);
		}else{
			nError = GetLastError();
		}
	}else{
		nError = GetLastError();
	}

	return strMac;
}


void GetMacByReg(vector<CString>&vtMacs)
{
    int nError = 0;
	CString strText;
	CString strMac;

    HKEY hKey = NULL;
    HKEY hKey2 = NULL;
    TCHAR szKey[MAX_PATH],szBuffer[MAX_PATH];
	TCHAR szServiceName[MAX_PATH];
    TCHAR szDescription[MAX_PATH];
    DWORD dwType = 0;
	DWORD cbData = 0;
    DWORD cName = _countof(szBuffer);
	TCHAR szRegPath1[] = { 'S', 'O', 'F', 'T', 'W', 'A', 'R', 'E', '\\', 'M', 'i', 'c', 'r', 'o', 's', 'o', 'f', 't', '\\', 'W', 'i', 'n', 'd', 'o', 'w', 's', ' ', 'N', 'T', '\\', 'C', 'u', 'r', 'r', 'e', 'n', 't', 'V', 'e', 'r', 's', 'i', 'o', 'n', '\\', 'N', 'e', 't', 'w', 'o', 'r', 'k', 'C', 'a', 'r', 'd', 's', '\\', '\0' };
	TCHAR szServiceNameKey[] = { 'S', 'e', 'r', 'v', 'i', 'c', 'e', 'N', 'a', 'm', 'e', '\0' };
	TCHAR szDescriptionKey[] = { 'D', 'e', 's', 'c', 'r', 'i', 'p', 't', 'i', 'o', 'n', '\0' };
	TCHAR szvirtual[] = { 'v', 'i', 'r', 't', 'u', 'a', 'l', '\0' };

	if ( RegOpenKey(HKEY_LOCAL_MACHINE,szRegPath1,&hKey)!=ERROR_SUCCESS ){
        return;
    }

    for ( int i=0; RegEnumKeyEx(hKey,i,szBuffer,&cName,NULL,NULL,NULL,NULL)==ERROR_SUCCESS; ++i, cName = _countof(szBuffer) ){
		_tcscpy_s(szKey,MAX_PATH,szRegPath1);
		_tcscat_s(szKey,MAX_PATH,szBuffer);
        if ( RegOpenKey(HKEY_LOCAL_MACHINE,szKey,&hKey2)!=ERROR_SUCCESS ){
            continue;
        }

        dwType = REG_SZ;
		cbData = sizeof(szDescription);
		memset(szDescription,0,sizeof(szDescription));
		RegQueryValueEx(hKey2,szDescriptionKey,NULL,&dwType,(LPBYTE)szDescription,&cbData);
		strText = FilterInvalidMacAdapter(szDescription);
		if ( strText.IsEmpty()==TRUE ){
			RegCloseKey(hKey2);
			continue;
		}

		dwType = REG_SZ;
		cbData = sizeof(szServiceName);
        if ( RegQueryValueEx(hKey2,szServiceNameKey,NULL,&dwType,(LPBYTE)szServiceName,&cbData)==ERROR_SUCCESS ){

            //读取成功后关闭句柄
            RegCloseKey(hKey2);
			strMac = GetMacByAdapterName(szServiceName);
			strMac = FilterInvalidMac(strMac);
			if ( strMac.IsEmpty()==FALSE ){
				vtMacs.push_back(strMac);
			}
        }else{
			nError = GetLastError();
			//读取失败关闭句柄
			RegCloseKey(hKey2);
		}        

    }//end for

	if ( hKey!=NULL ){
		RegCloseKey(hKey);
	}
}

/*
根据IP地址获取MAC，返回当前ip对应的MAC，一般为真实的
*/
CString GetMacByIP(vector<CString>&vtMacs)
{
	CString strText;
	CString strRealMac;
	CString strMac;
	WSADATA wsaData;
	if ( WSAStartup(MAKEWORD(2,0),&wsaData)!=0 ){
		return strMac;
	}

	char szName[MAX_PATH] = {0};
	int err = gethostname(szName, MAX_PATH);

	if( err==SOCKET_ERROR ) {
		WSACleanup();
		return strMac;
	}

	hostent *pHostent = gethostbyname(szName);
	ULONG IP = *(ULONG*)pHostent->h_addr_list[0];

	ULONG ulAdapterInfoSize = sizeof(IP_ADAPTER_INFO);
	IP_ADAPTER_INFO *pAdapterInfoEnum = NULL;
	IP_ADAPTER_INFO *pAdapterInfo = (IP_ADAPTER_INFO*)new char[ulAdapterInfoSize];
	if ( pAdapterInfo==NULL ){
		WSACleanup();
		return strMac;
	}

	if( GetAdaptersInfo(pAdapterInfo, &ulAdapterInfoSize) == ERROR_BUFFER_OVERFLOW ){
		// 缓冲区不够大
		delete[] pAdapterInfo;
		pAdapterInfo = (IP_ADAPTER_INFO*)new char[ulAdapterInfoSize];
		if ( pAdapterInfo==NULL ){
			WSACleanup();
			return strMac;
		}
	}

	pAdapterInfoEnum = pAdapterInfo;
	if( GetAdaptersInfo(pAdapterInfoEnum, &ulAdapterInfoSize) == ERROR_SUCCESS ){
		do{     //遍历所有适配器
			if( pAdapterInfoEnum->Type == MIB_IF_TYPE_ETHERNET ){
				// 判断是否为以太网接口
				//pAdapterInfoEnum->Description 是适配器描述
				//pAdapterInfoEnum->AdapterName 是适配器名称
				//pAdapterInfoEnum->Address      是Mac地址
				CString strDescription;
				strDescription = pAdapterInfoEnum->Description;
				strText = FilterInvalidMacAdapter(strDescription);
				if ( strText.IsEmpty()==TRUE ) {
					pAdapterInfoEnum = pAdapterInfoEnum->Next;
					continue;
				}

				CString strAdapterName;
				strAdapterName = pAdapterInfoEnum->AdapterName;
				strMac = GetMacByAdapterName(strAdapterName);
				strMac = FilterInvalidMac(strMac);
				if ( strMac.IsEmpty()==FALSE ) {
					vtMacs.push_back(strMac);
				}
				if ( inet_addr(pAdapterInfoEnum->IpAddressList.IpAddress.String) == IP ){
					strRealMac = strMac;
					break;
				}
			}
			pAdapterInfoEnum = pAdapterInfoEnum->Next;
		}while(pAdapterInfoEnum);
	}

	delete []pAdapterInfo;

	WSACleanup();
	return strRealMac;
}

//获取MAC列表
int Star::Common::GetMacs(vector<CString>&vtMacs)
{
	vtMacs.clear();
	GetMacByReg(vtMacs);
	GetMacByIP(vtMacs);

	return (int)vtMacs.size();
}

CString Star::Common::GetMac()
{
	CString strMac;
	vector<CString> vtMacs;

	GetMacByReg(vtMacs);
	if ( vtMacs.empty()==false ) {
		strMac = vtMacs[0];
	}else{
		strMac = GetMacByIP(vtMacs);
	}

	return strMac;
}

//HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Run
//在win7 64系统下会自动映射到：HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Run
//举例：CreateAutoRun(_T("myautoruntest"),_T("/auto"));
int Star::Common::CreateAutoRun( LPCTSTR szDispInfo, LPCTSTR szAutoRunCmd, BOOL bAdmin, BOOL bAdd )
{
	int nError = 0;
	HKEY hRootKey = HKEY_LOCAL_MACHINE;
	HKEY hKey = NULL;   
	CString strFilePath;   
	GetModuleFileName(NULL,strFilePath.GetBufferSetLength(MAX_PATH+1),MAX_PATH);   
	strFilePath.ReleaseBuffer(); 

	strFilePath = _T("\"") + strFilePath + _T("\" ") + szAutoRunCmd;

	if ( bAdmin==FALSE ) {
		hRootKey = HKEY_CURRENT_USER;
	}
	RegOpenKey(hRootKey, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), &hKey);
	if ( hKey!=NULL ){
		if ( bAdd ){
			nError = RegSetValueEx(hKey,szDispInfo,0,REG_SZ,(const BYTE *)(LPCTSTR)strFilePath,strFilePath.GetLength());
		}else{
			nError = RegDeleteValue(hKey,szDispInfo);
		}
		RegCloseKey(hKey);
	}

	return nError;
} 


//////////////////////////////////////////////////////////////////////////
//功能:弹出文件的系统菜单
//strFilePath文件完整路径
//pt 弹出菜单的坐标
//////////////////////////////////////////////////////////////////////////
void Star::Common::PopExplorerMenu(HWND hWnd, const CString&strFilePath, const CPoint&pt)
{
	IShellFolder *pIShellFolder = NULL;
	CStringW strFilePathW;
	strFilePathW = strFilePath;

	HRESULT hr = SHGetDesktopFolder(&pIShellFolder);
	if(NOERROR != hr)
	{
		return;
	}
	LPITEMIDLIST pidl = NULL;
	ULONG cbEaten = 0;
	DWORD dwAttribs = SFGAO_COMPRESSED;

	hr = pIShellFolder->ParseDisplayName(NULL, NULL, (LPWSTR)(LPCWSTR)strFilePathW, &cbEaten, &pidl, &dwAttribs);
	if(S_OK  != hr)
	{
		return;
	}
	IShellFolder *psfParent;
	LPCITEMIDLIST pidlRelative = NULL;
	hr = SHBindToParent(pidl, IID_IShellFolder, (void**)&psfParent, &pidlRelative); 
	if(S_OK != hr)
	{
		return;
	}

	CMenu mu;
	mu.CreatePopupMenu();
	LPCONTEXTMENU ppv = NULL;
	pIShellFolder->GetUIObjectOf(NULL, 1, &pidlRelative, IID_IContextMenu, NULL, (void**)&ppv);
	ppv->QueryContextMenu(mu.m_hMenu, 0, 0, 0x7FFF, CMF_NORMAL | CMF_EXPLORE);
	int nCmd = mu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, CWnd::FromHandle(hWnd));
	if( nCmd ){
		CMINVOKECOMMANDINFO ici;
		ici.cbSize          = sizeof (CMINVOKECOMMANDINFO);
		ici.fMask           = 0;
		ici.hwnd            = hWnd;
		ici.lpVerb          = MAKEINTRESOURCEA(nCmd - 1);
		ici.lpParameters    = NULL;
		ici.lpDirectory     = NULL;
		ici.nShow           = SW_SHOWNORMAL;
		ici.dwHotKey        = 0;
		ici.hIcon           = NULL;
		ppv->InvokeCommand(&ici);
	}
}

CString Star::Common::FormatLastError(DWORD dwError/* = -1*/)
{
	CString strError;
	if ( dwError==-1 ) {
		dwError = ::GetLastError();
	}

	HLOCAL hlocal = NULL;   // Buffer that gets the error message string

	// Use the default system locale since we look for Windows messages.
	// Note: this MAKELANGID combination has 0 as value
	DWORD systemLocale = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);

	// Get the error code's textual description
	BOOL fOk = FormatMessage(
							 FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS |
							 FORMAT_MESSAGE_ALLOCATE_BUFFER, 
							 NULL, dwError, systemLocale, 
							 (PTSTR) &hlocal, 0, NULL);

	if (!fOk) {
		// Is it a network-related error?
		HMODULE hDll = LoadLibraryEx(TEXT("netmsg.dll"), NULL, 
			DONT_RESOLVE_DLL_REFERENCES);

		if (hDll != NULL) {
			fOk = FormatMessage(
				FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS |
				FORMAT_MESSAGE_ALLOCATE_BUFFER,
				hDll, dwError, systemLocale,
				(PTSTR) &hlocal, 0, NULL);
			FreeLibrary(hDll);
		}
	}

	if (fOk && (hlocal != NULL)) {
		strError = (PCTSTR) LocalLock(hlocal);
		LocalFree(hlocal);
	} else {
		strError = TEXT("No text found for this error number.");
	}

	return strError;
}

void Star::Common::PathWithSlash(CString&strPath)
{
	if ( strPath.GetLength()>1 ) {
		strPath.Replace('/', '\\');
		if ( strPath.Right(1)!="\\" ) {
			strPath += "\\";	
		}
	}
}

CString Star::Common::GetFileExt(const CString&strFilePath)
{
	CString strExt;
	int nPos = strFilePath.ReverseFind('.');
	if ( nPos!=-1 ) {
		strExt = strFilePath.Mid(nPos);
	}

	return strExt;
}

BOOL Star::Common::CreateDirRecursively(const CString&strRoot,const CString&strSubDir)
{
	BOOL bOK = FALSE;
	CString strTemp;
	CString strParent = strRoot;
	CString strPath = strSubDir;
	Star::Common::PathWithSlash(strParent);
	Star::Common::PathWithSlash(strPath);

	if ( strParent.GetLength() < 3 || strPath.GetLength() < 1 ) {
		return FALSE;
	}

	//判断strPath是否是全路经
	if ( strPath.Find(':')!=-1 ) {
		if ( strPath.GetLength() > strParent.GetLength() ) {
			strPath = strPath.Mid(strParent.GetLength());
		}
	}
	if ( strPath.GetAt(0)=='\\' ) {
		strPath = strPath.Mid(1);
	}
	strPath = strParent + strPath;

	int nPos = 0;
	int nStart = strParent.GetLength();
	while ( nStart<strPath.GetLength() ){
		nPos = strPath.Find('\\',nStart);
		if ( nPos==-1 ) {
			break;
		}

		strTemp = strPath.Left(nPos);
		if ( GetFileAttributes(strTemp)==-1 ) {
			CreateDirectory(strTemp, NULL);
		}
		nStart = nPos + 1;
	}

	DWORD dwAttr =  GetFileAttributes(strPath);
	if ( dwAttr!=-1 && (dwAttr & FILE_ATTRIBUTE_DIRECTORY) ) {
		bOK = TRUE;
	}

	return bOK;
}

BOOL Star::Common::DeleteDirectory(LPCTSTR lpszDirPath)
{
	BOOL bOK = TRUE;
	if ( lpszDirPath==NULL || GetFileAttributes(lpszDirPath)==-1 ) {
		return FALSE;
	}

	CString strDirPath = lpszDirPath;
	Star::Common::PathWithSlash(strDirPath);

	WIN32_FIND_DATA stFindData = {0};
	HANDLE hFind = ::FindFirstFile(strDirPath + _T("*.*"), &stFindData);
	if ( hFind!=INVALID_HANDLE_VALUE ) {

		do 
		{
			if ( stFindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY ) {
				if ( stFindData.cFileName[0]!='.' ) {
					bOK = bOK & DeleteDirectory(strDirPath + stFindData.cFileName);
				}
			}else{
				bOK = bOK & ::DeleteFile(strDirPath + stFindData.cFileName);
			}
		} while ( ::FindNextFile(hFind, &stFindData) );

		::FindClose(hFind);
	}

	bOK = bOK & ::RemoveDirectory(lpszDirPath);

	return bOK;
}

//SHFileOperation中的pFrom可以表示多个路径，各个路径之间以'\0'分隔，最后以'\0\0'(即连续的两个'\0')结束，
BOOL Star::Common::DeleteFileToRecycle(const CString&strFilePath)
{
	BOOL bSuccess = FALSE;
	TCHAR szFilePath[MAX_PATH] = {0};
	SHFILEOPSTRUCT stFileOp = {0};

	_tcscpy_s(szFilePath, strFilePath);
	szFilePath[strFilePath.GetLength()+1] = 0;

	stFileOp.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION;
	stFileOp.pFrom = szFilePath;
	stFileOp.pTo = NULL;
	stFileOp.wFunc = FO_DELETE;
	bSuccess = (SHFileOperation(&stFileOp)==0);
	return bSuccess;
}

BOOL Star::Common::Run(LPCTSTR lpszCmdLine, DWORD dwMilliseconds/* = INFINITE*/)
{
	BOOL bSuccess = FALSE;
	CString strCmdLine = lpszCmdLine;
	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi;

	bSuccess = ::CreateProcess(NULL,(LPTSTR)(LPCTSTR)strCmdLine,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	return bSuccess;
}

BOOL Star::Common::RunWaitFinish(LPCTSTR lpszCmdLine, WORD wShowWindow, DWORD dwMilliseconds/* = INFINITE*/ , BOOL bForceKillTimeOut)
{
	BOOL bSuccess = FALSE;
	CString strCmdLine = lpszCmdLine;
	PROCESS_INFORMATION pi;
	STARTUPINFO si = { sizeof(si) };
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = wShowWindow;

	bSuccess = ::CreateProcess(NULL,(LPTSTR)(LPCTSTR)strCmdLine,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
	DWORD dwStatus = WaitForSingleObject(pi.hProcess, dwMilliseconds);
	if ( dwStatus == WAIT_TIMEOUT ) {
		if ( bForceKillTimeOut==TRUE ) {
			TerminateProcess(pi.hProcess, -1);
		}
	}
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	return bSuccess;
}

BOOL Star::Common::RunBat(const CString&strBatContent, LPCTSTR lpszWorkPath/* = NULL*/, DWORD dwMilliseconds)
{
	BOOL bSuccess = FALSE;
	DWORD dwReadWrite = 0;
	CString strWorkPath;

	if ( lpszWorkPath==NULL ) {
		//临时目录选择在当前目录下
		strWorkPath = Star::Common::GetStartPath();
	}else if ( GetFileAttributes(lpszWorkPath)==-1 ) {
		//目录不存在则创建之
		if ( CreateDirectory(lpszWorkPath, NULL)==TRUE ) {
			strWorkPath = lpszWorkPath;
		}else{
			strWorkPath = Star::Common::GetStartPath();
		}
	}

	Star::Common::PathWithSlash(strWorkPath);
	CString strBatFile = strWorkPath + _T("tempbat.bat");
	HANDLE hFile = ::CreateFile(strBatFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( hFile!=INVALID_HANDLE_VALUE ) {
		WriteFile(hFile, strBatContent, strBatContent.GetLength(), &dwReadWrite, NULL);
		CloseHandle(hFile);

		bSuccess = Star::Common::RunWaitFinish(strBatFile, SW_SHOWNORMAL, dwMilliseconds);
		::DeleteFile(strBatFile);
	}else{
		_tprintf_s(_T("create tempbat.bat file failed, error code: %d\r\n"), GetLastError());
	}

	return bSuccess;
}

void Star::Common::FindFileRecursively(OUT vector<CString>&vtFiles, IN LPCTSTR lpszDirPath, IN LPCTSTR lpszFilter/* = NULL*/)
{
	CString strDirPath = lpszDirPath;
	CString strFindFilter;

	Star::Common::PathWithSlash(strDirPath);
	if ( lpszFilter==NULL ) {
		strFindFilter = "*.*";
	}else{
		strFindFilter = lpszFilter;
	}

	WIN32_FIND_DATA stFindData = {0};
	HANDLE hFind = ::FindFirstFile(strDirPath + strFindFilter, &stFindData);
	if ( hFind!=INVALID_HANDLE_VALUE ) {

		do 
		{
			if ( stFindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY ) {
				if ( stFindData.cFileName[0]!='.' ) {
					FindFileRecursively(vtFiles, strDirPath + stFindData.cFileName, lpszFilter);
				}
			}else{
				vtFiles.push_back(strDirPath + stFindData.cFileName);
			}
		} while ( ::FindNextFile(hFind, &stFindData) );

		::FindClose(hFind);
	}

}

LONGLONG Star::Common::GetFileSize(LPCTSTR lpszFileName)
{
	LONGLONG llSize = 0;

	if ( lpszFileName==NULL ){
		return llSize;
	}

	HANDLE hFile = CreateFile(lpszFileName,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,NULL,NULL);
	if ( hFile!=INVALID_HANDLE_VALUE ){
		GetFileSizeEx(hFile,(PLARGE_INTEGER)&llSize);
		CloseHandle(hFile);
	}

	return llSize;
}

DWORD Star::Common::GetFileSize(HANDLE hFile,LPDWORD lpFileSizeHigh)
{
	DWORD dwSize = 0;

	if ( hFile!=NULL && hFile!=INVALID_HANDLE_VALUE ){
		dwSize = ::GetFileSize(hFile,lpFileSizeHigh);
		if ( dwSize==INVALID_FILE_SIZE ){
			dwSize = 0;
		}
	}

	return dwSize;
}

CString Star::Common::FormatSize(LONGLONG llSize)
{
	CString strText;
	if ( llSize >= 1024 * 1024 * 1024 ) {
		strText.Format(_T("%.2f GB"), llSize / (float)(1024 * 1024 * 1024));
	}else if ( llSize >= 1024 * 1024 ) {
		strText.Format(_T("%.2f MB"), llSize / (float)(1024 * 1024));
	}else if ( llSize >= 1024 ) {
		strText.Format(_T("%.2f KB"), llSize / (float)1024);
	}else{
		strText.Format(_T("%d byte"), (int)llSize);
	}

	return strText;
}

BOOL Star::Common::ReadTextFile(const CString&strFile, OUT CStringA&strText)
{
	BOOL bSuccess = FALSE;
	DWORD dwReadWrite = 0;
	DWORD dwSize = 0;
	strText.Empty();
	HANDLE hFile = ::CreateFile(strFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( hFile!=INVALID_HANDLE_VALUE ) {
		dwSize = ::GetFileSize(hFile, NULL);
		ReadFile(hFile, strText.GetBuffer(dwSize), dwSize, &dwReadWrite, NULL);
		CloseHandle(hFile);
		strText.ReleaseBuffer(dwReadWrite);
		bSuccess = TRUE;
	}

	return bSuccess;
}

//获取文件路径的父目录，返回的路径最后带斜杠
CString Star::Common::GetParentPath(const CString&strFilePath)
{
	CString strPath;
	strPath = strFilePath;
	strPath.Replace('/','\\');

	if ( strPath.Right(1)=="\\" ) {
		strPath = strPath.Left(strPath.GetLength()-1);
	}

	int nPos = strPath.ReverseFind('\\');
	if ( nPos!=-1 ) {
		strPath = strPath.Left(nPos+1);
	}

	return strPath;
}

//获取文件名，斜杠反斜杠后面的（包含扩展名）
CString Star::Common::GetFileName(const CString&strFilePath)
{
	CString strFileName = strFilePath;
	strFileName.Replace('/','\\');
	int nPos = strFileName.ReverseFind('\\');
	if ( nPos!=-1 ) {
		strFileName = strFileName.Mid(nPos + 1);
	}

	return strFileName;
}

CString Star::Common::GetFileNameWithoutExt(const CString&strFilePath)
{
	CString strFileName = strFilePath;
	strFileName.Replace('/','\\');

	int nPos = strFileName.ReverseFind('\\');
	int nPos2 = 0;

	if ( nPos!=-1 ) {
		if ( GetFileAttributes(strFileName) & FILE_ATTRIBUTE_DIRECTORY ) {
			nPos2=strFileName.GetLength();
		}else{
			nPos2 = strFileName.ReverseFind('.');
			if ( nPos2==-1 ) {
				nPos2=strFileName.GetLength();
			}
		}
		strFileName = strFileName.Mid(nPos + 1, nPos2 - nPos - 1);
	}

	return strFileName;
}

//自动定位一个文件并选中
BOOL Star::Common::LocateFile(const CString&strFileName)
{
	BOOL bSuccess = TRUE;

	if ( GetFileAttributes(strFileName)==-1 ){	//文件不存在打开它所在的目录
		CString strFolder;
		int nPos=strFileName.ReverseFind('\\');
		if ( nPos!=-1 ){
			strFolder=strFileName.Left(nPos);
		}

		if ( GetFileAttributes(strFolder)==-1 ){
			//AfxMessageBox("以下文件不存在，可能已经被删除了：\n"+strFileName);
			bSuccess = FALSE;
		}else{
			ShellExecute(NULL,_T("open"),_T("explorer.exe"),strFolder,NULL,SW_NORMAL);
		}
	}else{
		CString strCmdLine;
		strCmdLine.Format( _T("/select, \"%s\""), strFileName );
		ShellExecute(NULL,_T("open"),_T("explorer.exe"),strCmdLine,NULL,SW_NORMAL);
	}

	return bSuccess;
}

//打开文件属性对话框
void Star::Common::OpenFilePropertyDialog(const CString&strFileName)
{
	SHELLEXECUTEINFO info = { 0 };
	info.cbSize = sizeof(info);
	info.lpVerb = _T("properties");
	info.lpFile = strFileName;
	info.nShow = SW_SHOW;
	info.fMask = SEE_MASK_INVOKEIDLIST;
	ShellExecuteEx(&info);        
}

BOOL Star::Common::CreateShortcut(const CString&strShotcutFile, const CString&strSourcePath, LPCTSTR lpszArguments/* = NULL*/)
{
	BOOL bSuccess = FALSE;
	DWORD FileAttributes = -1;
	IShellLink* psl = NULL;
	IPersistFile* ppf = NULL;

	if( SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL,CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl)) ){
		psl->SetPath(strSourcePath);
		FileAttributes = GetFileAttributes(strSourcePath);
		if ( FileAttributes!=-1 && (FileAttributes&FILE_ATTRIBUTE_DIRECTORY) ) {
			psl->SetWorkingDirectory(strSourcePath);
		}else{
			psl->SetWorkingDirectory(GetParentPath(strSourcePath));
		}
		if ( lpszArguments!=NULL ) {
			psl->SetArguments(lpszArguments);
		}
		if( SUCCEEDED(psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf)) ) {
			BSTR pPath = strShotcutFile.AllocSysString(); 
			if(SUCCEEDED(ppf->Save(pPath,TRUE))) {
				bSuccess = TRUE;
			}
			if ( pPath!=NULL ) {
				::SysFreeString(pPath);
				pPath = NULL;
			}

			ppf->Release();
		}

		psl->Release();
	}

	return bSuccess;
}

BOOL Star::Common::GetShortcutInfo(const CString&strFileName, CString&strTarget, CString&strParams, CString&strStartPath)
{
	BOOL bSuccess = FALSE;
	TCHAR szTarget[MAX_PATH] = {0};
	TCHAR szArguments[MAX_PATH] = {0};
	TCHAR szWorkingDirectory[MAX_PATH] = {0};
	CStringW strFilePathW;
	strFilePathW = strFileName;

	IShellLink *pShellLink = NULL;
	HRESULT hRes = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&pShellLink);
	if(SUCCEEDED(hRes))
	{
		IPersistFile *ppf = NULL;
		hRes = pShellLink->QueryInterface(IID_IPersistFile, (void**)&ppf);
		if(SUCCEEDED(hRes))
		{
			hRes = ppf->Load(strFilePathW, TRUE);
			if(SUCCEEDED(hRes)) {
				pShellLink->GetPath(szTarget, MAX_PATH, NULL, 0);
				pShellLink->GetArguments(szArguments, MAX_PATH);
				pShellLink->GetWorkingDirectory(szWorkingDirectory, MAX_PATH);
			}
			ppf->Release();
		}
		pShellLink->Release();
	}

	strTarget = szTarget;
	strParams = szArguments;
	strStartPath = szWorkingDirectory;
	return bSuccess;
}

HANDLE Star::Common::CreateFile(
						  LPCTSTR lpFileName, 
						  DWORD dwDesiredAccess, 
						  DWORD dwShareMode, 
						  LPSECURITY_ATTRIBUTES lpSecurityAttributes, 
						  DWORD dwCreationDisposition, 
						  DWORD dwFlagsAndAttributes, 
						  HANDLE hTemplateFile
						  )
{
	HANDLE hFile = INVALID_HANDLE_VALUE;

	CStringW strFileNameW;
	strFileNameW = lpFileName;
	strFileNameW = L"\\\\?\\" + strFileNameW;

	hFile = ::CreateFileW(strFileNameW, dwDesiredAccess , dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

	return hFile;
}

//将字符串转换为十六进制字符串：支持unicode和多字节。参数1：字符串指针，参数2：字符数
CString Star::Common::Str2HexStr(LPCTSTR lpszText, int nChCount)
{
	CString strResult;
	CString strTemp;
	if ( lpszText==NULL || nChCount<=0 ) {
		return strResult;
	}

	int nCbSize = nChCount * sizeof(TCHAR);
	byte *pByte = (byte *)lpszText;
	for ( int i = 0; i < nCbSize; ++i ) {
		strTemp.Format(_T("%02X"), pByte[i]);
		strResult += strTemp;
	}//end for

	return strResult;
}

int Star::Common::GetOsMajorVersion()
{
	OSVERSIONINFO svex;
	svex.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&svex);
	return svex.dwMajorVersion;
}
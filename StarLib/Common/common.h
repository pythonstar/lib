 
#pragma once
#include <afx.h>
#include <atlstr.h>
#include <vector>
using namespace std;



// _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
// _CrtSetBreakAlloc(118);	

/*------------------------------------------------------------------------
˵��: ȡ���Ա�����ĵ�ַ.vc8�汾.����ȡ˽�г�Ա������ַ.
ʹ��ʾ��:

DWORD dwAddr;
#if _MSC_VER >1200
GetMemberFuncAddr_VC8(dwAddr,obj::FuncMember);
#else
Star::Common::GetMemberFuncAddr_VC6(dwAddr,&obj::FuncMember);
#endif
------------------------------------------------------------------------*/
// 
// #define GetMemberFuncAddr_VC8(FuncAddr,FuncType)\
// {	\
// 	__asm{mov eax,offset FuncType};\
// 	__asm{mov FuncAddr, eax};\
// }

void _Trace(const CHAR* lpszFormat, ...);
void _TraceW(const CHAR* lpszFormat, ...);
#define Trace _Trace
#define TraceW _TraceW

namespace Star
{
	namespace Common
	{
		HINSTANCE OpenUrl(LPCTSTR url, int showcmd=SW_SHOWNORMAL);
		CString GetStartPath();
		CString GetAppName();
		BOOL IsWow64();
		CString GetMyTempPath();
		CString GetSysTempPath();
		CString GetSysTempFileName(LPCTSTR lpszExt = NULL);
		CString GetModuleFilePath();
		CString GetWindowsPath();
		CString GetSystemPath();
		CString GetDesktopPath();
		CString GetMypicturesPath();

		LONG GetRegKey(HKEY key, LPCTSTR subkey, LPTSTR retdata);
		LONG GetRegValueDWord(HKEY key, LPCTSTR subkey, LPCTSTR szValue,DWORD*pData);
		LONG SetRegValueDWord(HKEY key, LPCTSTR subkey, LPCTSTR szValue,DWORD dwData,BOOL bForce=TRUE);

		//�ͷ���Դ��ĳ���͵��ļ� 
		BOOL ReleaseRes(LPCTSTR szFileName, DWORD dwResID, LPCTSTR lpType);  

		int GenerateGUID(CString& sGUID);
		CStringA ts2utf8s(const CString&str);
		CString utf8s2ts(const char*lpszText, int nSize = -1);

		//һ���ַ�����Ҫ��Ϊ�ļ�����ȥ������ַ����зǷ����ַ�
		//���ܷ���Ϊ�մ���Ҳ��������ַ����ܺϷ�����̫���ˣ���������Ҫ�ⲿ����
		CString filterinvalidfilename(CString strText);

		//ת����ҳ�е�һЩ�ַ�
		void unescapexml(CString&strText);

		//����\u65F6\u5149\u5F71\u50CF\u9986
		CString unescapeunicode(const char *lpszText);

		//ת�������ַ���һ�������ַ�ת��Ϊ����%��ע����������ĺ�Ӣ�ĵĻ����Ӣ�Ĳ���
		CString GBKEncodeURI(const CString&strText);
		//ת�������ַ���һ�������ַ�ת��Ϊ����%��ע����������ĺ�Ӣ�ĵĻ����Ӣ�Ĳ���
		CString UTF8EncodeURI(const CString&str);

		BOOL IsSignalToStop(HANDLE hEvent);

		void InitializeSeed();
		//����һ���������Χ��[Low��High]
		int RandIntNum(int Low,int High);    

		//����һ��0-1֮�������������ַ�����С��������λ����nCount��,���ó���MAX_PATHλ
		CString RandFloatNum(int nCount);
		CString RandNum(int nCount);

		void RestartApp();

		int SplitString(const CString&strSrc,const CString&strSep,vector<CString>&vtStr);

		//int SetProxy(CInternetSession&sess,CHttpFile*pFile,char*szproxy, char*proxyUser, char* proxyPassword);   
		
		//���ַ����в����ַ���pSub���ж���ַ�
		int StrFindOneOf(char*pMain,char*pSub);
		//����ֱ���������ֵ��ַ�
		int StrFindCharNotDigit(char*pMain);

		//ʵ�ֹػ��ĺ���,�����http://shareaza.svn.sourceforge.net/svnroot/shareaza
		BOOL SetShutdownRights();
		//Ĭ�Ϲػ�
		BOOL ShutDownComputer(UINT ShutdownFlags=EWX_POWEROFF);
		
		CString GetCurrentTimeStr(LPCTSTR lpszFormat = NULL);
		LONGLONG GetCurrentTimeTotalSeconds();
		LONGLONG GetCurrentTimeTotalMiliSeconds();

		BOOL MsgSleep(IN DWORD dwMillseconds);
		//////////////////////////////////////////////////////////////////////////
#if 0
		BOOL WINAPI EnableDebugPrivilege(BOOL bEnable);
		BOOL WINAPI RemoteLoadLibrary(LPCTSTR pszDllName, DWORD dwProcessId);
		int WaitData(SOCKET hSocket,DWORD dwTime);
		ULONG GetInternetTime(char *lpszHostAddress, UINT nHostPort,int nTimeOut=10000);
		ULONG GetInternetTime(int nTimeOut=10000);

		//���� ���hwndΪNULL���ȡ�������棬����ֻ��ȡ�ô���,�ļ����浽szFilePath
		DWORD CaptureScreen(HWND hWnd, LPCTSTR sSavePath);
#endif

		//ת��·��,\Device\HarddiskVolume2\ -�� D
		CString DosDevicePath2LogicalPath(LPCTSTR lpszDosPath);
		//////////////////////////////////////////////////////////////////////////
		BOOL CopyToClipboard(LPCTSTR szText,int nCbSize=-1,HWND hWndNewOwner=NULL);
		BOOL GetClipboardText(CString&strText);

		CString GetMac();

		//��ȡMAC�б�
		int GetMacs(vector<CString>&vtMacs);


		int CreateAutoRun( LPCTSTR szDispInfo, LPCTSTR szAutoRunCmd, BOOL bAdmin, BOOL bAdd=TRUE );

		//�����ļ���ϵͳ�˵�
		void PopExplorerMenu(HWND hWnd, const CString&strFilePath, const CPoint&pt);

		CString FormatLastError(DWORD dwError = -1);

		void PathWithSlash(CString&strPath);
		BOOL CreateDirRecursively(const CString&strRoot,const CString&strSubDir);
		BOOL DeleteDirectory(LPCTSTR lpszDirPath);
		BOOL DeleteFileToRecycle(const CString&strFilePath);
		BOOL Run(LPCTSTR lpszCmdLine, DWORD dwMilliseconds = INFINITE);
		BOOL RunWaitFinish(LPCTSTR lpszCmdLine, WORD wShowWindow = SW_SHOWNORMAL, DWORD dwMilliseconds = INFINITE, BOOL bForceKillTimeOut = FALSE);
		BOOL RunBat(const CString&strBatContent, LPCTSTR lpszWorkPath = NULL, DWORD dwMilliseconds = INFINITE);

		//��ָ��Ŀ¼lpszDirPath�в��Һ�׺ΪlpszFilter���ļ������lpszFilterָ������չ������Ҳ�֧�ֵݹ���Ŀ¼�����û��ָ����չ�������*.*
		void FindFileRecursively(OUT vector<CString>&vtFiles, IN LPCTSTR lpszDirPath, IN LPCTSTR lpszFilter = NULL);
		
		LONGLONG GetFileSize(LPCTSTR lpszFileName);
		DWORD GetFileSize(HANDLE hFile,LPDWORD lpFileSizeHigh);
		CString FormatSize(LONGLONG llSize);
		BOOL ReadTextFile(const CString&strFile, OUT CStringA&strText);


		//��ȡ�ļ�·���ĸ�Ŀ¼�����ص�·������б��
		CString GetParentPath(const CString&strFilePath);
		//��ȡ�ļ�����б�ܷ�б�ܺ���ģ�������չ����
		CString GetFileName(const CString&strFilePath);
		CString GetFileNameWithoutExt(const CString&strFilePath);
		CString GetFileExt(const CString&strFilePath);
		//�Զ���λһ���ļ���ѡ��
		BOOL LocateFile(const CString&strFileName);
		//���ļ����ԶԻ���
		void OpenFilePropertyDialog(const CString&strFileName);

		BOOL CreateShortcut(const CString&strShotcutFile, const CString&strSourcePath, LPCTSTR lpszArguments = NULL);
		BOOL GetShortcutInfo(const CString&strFileName, CString&strTarget, CString&strParams, CString&strStartPath);

		//֧�ֳ���·���Ĵ��ļ�
		HANDLE CreateFile(
			LPCTSTR lpFileName, 
			DWORD dwDesiredAccess, 
			DWORD dwShareMode, 
			LPSECURITY_ATTRIBUTES lpSecurityAttributes, 
			DWORD dwCreationDisposition, 
			DWORD dwFlagsAndAttributes, 
			HANDLE hTemplateFile
			); 

		//���ַ���ת��Ϊʮ�������ַ�����֧��unicode�Ͷ��ֽڡ�����1���ַ���ָ�룬����2���ַ���
		CString Str2HexStr(LPCTSTR lpszText, int nChCount);
		int GetOsMajorVersion();
	}
}
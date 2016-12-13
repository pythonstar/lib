
#include "stdafx.h"
#include "Sys.h"

typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
BOOL Star::Sys::IsWow64()
{
	BOOL bIsWow64 = FALSE;

	LPFN_ISWOW64PROCESS  fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(_T("kernel32")),"IsWow64Process");
	if ( NULL!=fnIsWow64Process ){
		if ( !fnIsWow64Process(GetCurrentProcess(),&bIsWow64) ){
			// handle error
		}
	}

	return bIsWow64;
}

int Star::Sys::GetTrayInfo(HWND hWnd,vector<TRAYINFO>&vtTrays)
{
	if ( hWnd==NULL ){
		return 0;
	}

	DWORD dwProcessId = 0;
	GetWindowThreadProcessId(hWnd,&dwProcessId);

	HANDLE hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, dwProcessId);
	if ( hProcess==NULL ){
		return 0;
	}
	LPVOID lAddress = VirtualAllocEx(hProcess, 0, 4096, MEM_COMMIT, PAGE_READWRITE);
	if ( lAddress==NULL ){
		CloseHandle(hProcess);
		return 0;
	}

	DWORD lTextAdr = 0;
	BYTE buff[1024] = {0};
	TRAYINFO stTrayInfo;
	CString strFilePath;
	CString strTile;
	HWND hMainWnd = NULL;
	int nDataOffset = sizeof(TBBUTTON) - sizeof(INT_PTR) - sizeof(DWORD_PTR);
	int nStrOffset = 18; 
	if ( IsWow64() ){
		nDataOffset+=4;
		nStrOffset+=6;
	}

	//�õ��D�˂���
	int lButton = SendMessage(hWnd, TB_BUTTONCOUNT, 0, 0);
	for (int i = 0; i < lButton; i++) {
		SendMessage(hWnd, TB_GETBUTTON, i, (LPARAM)lAddress);
		//�x�ı���ַ
		ReadProcessMemory(hProcess, (LPVOID)((DWORD)lAddress + nDataOffset), &lTextAdr, 4, 0);
		if ( lTextAdr!=-1 ) {
			//�x�ı�
			ReadProcessMemory(hProcess, (LPCVOID)lTextAdr, buff, 1024, 0);
			hMainWnd = (HWND)(*((DWORD*)buff));
			strFilePath = (WCHAR *)buff + nStrOffset;
			strTile = (WCHAR *)buff + nStrOffset + MAX_PATH;
			stTrayInfo.strFilePath = strFilePath;
			stTrayInfo.strDiscription = strTile;
			vtTrays.push_back(stTrayInfo);
		}
	}
	VirtualFreeEx(hProcess, lAddress, 4096, MEM_RELEASE);
	CloseHandle(hProcess);

	return (int)vtTrays.size();
}

//win7 vista����
/************************************************************************/
/************************************************************************/
/************************************************************************/
/************************************************************************/
#if (WINVER >= 0x0600)

//�жϲ���ϵͳ�ǲ���win7����vista
BOOL Star::Sys::IsVista()
{
	BOOL bVista=FALSE;

	OSVERSIONINFOEX osver={0};
	osver.dwOSVersionInfoSize=sizeof(osver);
	osver.dwMajorVersion=6;
	osver.dwMinorVersion=0;
	osver.dwPlatformId=VER_PLATFORM_WIN32_NT;

	DWORDLONG dwlConditionMask=0;
	VER_SET_CONDITION(dwlConditionMask,VER_MAJORVERSION,VER_EQUAL);
	VER_SET_CONDITION(dwlConditionMask,VER_MINORVERSION,VER_EQUAL);
	VER_SET_CONDITION(dwlConditionMask,VER_PLATFORMID,VER_EQUAL);

	if ( VerifyVersionInfo(&osver,VER_MAJORVERSION|VER_MINORVERSION|VER_PLATFORMID,dwlConditionMask) ){
		bVista=TRUE;
	}

	return bVista;
}


/*
TOKEN_ELEVATION_TYPE s_elevationType = TokenElevationTypeDefault;
BOOL                 s_bIsAdmin = FALSE;
*/
BOOL GetProcessElevation(TOKEN_ELEVATION_TYPE* pElevationType, BOOL* pIsAdmin) 
{
	HANDLE hToken = NULL;
	DWORD dwSize; 

	// Get current process token
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
		return(FALSE);

	BOOL bResult = FALSE;

	// Retrieve elevation type information 
	if (GetTokenInformation(hToken, TokenElevationType, 
		pElevationType, sizeof(TOKEN_ELEVATION_TYPE), &dwSize)) {
			// Create the SID corresponding to the Administrators group
			byte adminSID[SECURITY_MAX_SID_SIZE];
			dwSize = sizeof(adminSID);
			CreateWellKnownSid(WinBuiltinAdministratorsSid, NULL, &adminSID, 
				&dwSize);

			if (*pElevationType == TokenElevationTypeLimited) {
				// Get handle to linked token (will have one if we are lua)
				HANDLE hUnfilteredToken = NULL;
				GetTokenInformation(hToken, TokenLinkedToken, (VOID*) 
					&hUnfilteredToken, sizeof(HANDLE), &dwSize);

				// Check if this original token contains admin SID
				if (CheckTokenMembership(hUnfilteredToken, &adminSID, pIsAdmin)) {
					bResult = TRUE;
				}

				// Don't forget to close the unfiltered token
				CloseHandle(hUnfilteredToken);
			} else {
				*pIsAdmin = IsUserAnAdmin();
				bResult = TRUE;
			}
	}

	// Don't forget to close the process token
	CloseHandle(hToken);

	return(bResult);
}

//��vista���жϵ�ǰ�û��Ƿ��ǹ���Ա
BOOL Star::Sys::IsAdmin()
{
	TOKEN_ELEVATION_TYPE elevationType = TokenElevationTypeDefault;
	BOOL                 bIsAdmin = FALSE;
	if ( GetProcessElevation(&elevationType, &bIsAdmin) ) {
		switch(elevationType) {
			// Default user or UAC is disabled
		 case TokenElevationTypeDefault:  
			 if ( IsUserAnAdmin() ) {
				 //�ǹ���Ա bCanReadSystemProcesses = true;
			 }else {
				 //���ǹ���Ա
			 }
			 break;

			 // Process has been successfully elevated
		 case TokenElevationTypeFull:
			 if ( IsUserAnAdmin() ) {
				 //�ǹ���Ա bCanReadSystemProcesses = true;
			 } else {
				 //���ǹ���Ա
			 }
			 break;

			 // Process is running with limited privileges
		 case TokenElevationTypeLimited:
			 if ( bIsAdmin ) {
				 //�ǹ���Ա�������ܶ�ȡϵͳ����
			 } else {
				 //���ǹ���Ա
			 }
			 break;
		}
	}

	return bIsAdmin;
}


#endif

//////////////////////////////////////////////////////////////////////////

HWND Star::Sys::FindTrayWnd()
{
	HWND hWnd = NULL;

	hWnd = FindWindow(_T("Shell_TrayWnd"), NULL);
	hWnd = FindWindowEx(hWnd, NULL, _T("TrayNotifyWnd"), NULL);
	hWnd = FindWindowEx(hWnd, NULL, _T("SysPager"), NULL);
	hWnd = FindWindowEx(hWnd, NULL, _T("ToolbarWindow32"), NULL);

	return hWnd;
}

HWND Star::Sys::FindNotifyIconOverflowWindow()
{
	HWND hWnd = NULL;

	hWnd = FindWindow(_T("NotifyIconOverflowWindow"), NULL);
	hWnd = FindWindowEx(hWnd, NULL, _T("ToolbarWindow32"), NULL);

	return hWnd;
}

//////////////////////////////////////////////////////////////////////////

//�����Ƿ�����
BOOL Star::Sys::IsScreensaversOn(void)
{
	BOOL isActive = FALSE;   //һ����BOOL������bool��������ʱ����
	SystemParametersInfo(SPI_GETSCREENSAVERRUNNING,0,&isActive,0);
	return isActive;
}

// This function is used for monitor enumeration
BOOL CALLBACK Star::Sys::EnumMonitorsProc(HMONITOR hMonitor, HDC /*hdcMonitor*/, LPRECT lprcMonitor, LPARAM dwData)
{	
	int *pMonitorCnt = (int *)dwData;
	if ( pMonitorCnt==NULL ){
		return FALSE;
	}

	++(*pMonitorCnt);

	return TRUE;
}

//��ȡ��ʾ�����������Ϊ�������
int  Star::Sys::GetMonitorCount(void)
{
	int nMonitorCnt = 0;
	::EnumDisplayMonitors(NULL, NULL, EnumMonitorsProc, (LPARAM)&nMonitorCnt);
	return nMonitorCnt;
}


//��ʾ���Ƿ�ر�
BOOL Star::Sys::IsMonitorOn(void)
{
	BOOL isActive = TRUE;
	HANDLE hMonitor=CreateFile(_T("\\\\.\\LCD\\"),GENERIC_WRITE|GENERIC_READ,0,NULL,OPEN_EXISTING,0,NULL);
	if( hMonitor!=INVALID_HANDLE_VALUE ){
		::GetDevicePowerState(hMonitor,&isActive);
		::CloseHandle(hMonitor);
	}
	return isActive;
}
//////////////////////////////////////////////////////////////////////////

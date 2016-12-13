
#include "stdafx.h"
#include "Thread.h"


_pfnZwQueryInformationThread	Thread::ZwQueryInformationThread=NULL;
_pfnRtlNtStatusToDosError		Thread::RtlNtStatusToDosError=NULL;

/*------------------------------------------------------------------------
[8/4/2009 zhuxingxing]
说明:   
------------------------------------------------------------------------*/
void Thread::EnumAllThreads(pfnEnumThreadCallBack EnumThreadCallBack,DWORD th32ProcessID)
{
	THREADENTRY32   te; 
	HINSTANCE		hNTDLL;
	HANDLE			hSnapShot;

	//提升权限
	Thread::GetDebugPrivilege();

	hNTDLL=GetModuleHandle("ntdll");  
	Thread::ZwQueryInformationThread=(_pfnZwQueryInformationThread) 
		GetProcAddress(hNTDLL,"ZwQueryInformationThread");  
	Thread::RtlNtStatusToDosError=(_pfnRtlNtStatusToDosError)  
		GetProcAddress(hNTDLL,"RtlNtStatusToDosError");

	hSnapShot=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,th32ProcessID);  
	te.dwSize=sizeof(te);  
	if (Thread32First(hSnapShot,&te))  
	{  
		do  
		{  
			if (te.th32OwnerProcessID==th32ProcessID)
			{
				EnumThreadCallBack(&te);
			} 
		}while( Thread32Next(hSnapShot,&te) );  
	}
	CloseHandle(hSnapShot); 
}

/*------------------------------------------------------------------------
[8/6/2009 zhuxingxing]
说明:   提高当前进程的权限
------------------------------------------------------------------------*/
BOOL Thread::GetDebugPrivilege()
{
	HANDLE hToken;
	LUID sedebugnameValue;
	TOKEN_PRIVILEGES tkp;

	HANDLE hGetHandle=GetCurrentProcess();

	if ( ! OpenProcessToken(hGetHandle,
		TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken ) )
	{
		return FALSE;
	}


	if ( ! LookupPrivilegeValue( NULL, SE_DEBUG_NAME, &sedebugnameValue ) )
	{
		CloseHandle( hToken );
		return FALSE;
	}

	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Luid = sedebugnameValue;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges( hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL ) )
	{
		CloseHandle( hToken );
		return FALSE;
	}

	CloseHandle(hToken);
	return TRUE;
}
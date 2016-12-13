
#pragma once
#include <afx.h>
#include <Tlhelp32.h>

namespace Star
{
	namespace Process
	{
		ULONG GetProcessID(char *szProcessName);

		//通过进程ID获取对应文件名
		CString GetModuleFileNameEx(DWORD dwProcessId);
		CString GetProcessImageFileName(DWORD dwProcessId);

		//获取进程对应的文件全路径名
		CString GetProcessFileName(DWORD dwProcessId);

		ULONG GetProcessThreads(ULONG PID,PULONG lpBuffer,ULONG dwBufferSize);
		BOOL WINAPI SuspendProcess(DWORD dwProcessID, BOOL bSuspend);
		HMODULE WINAPI ModuleFromAddress(PVOID pv);
		DWORD WINAPI GetThreadIdFromPID(DWORD dwProcessId);
		BOOL WINAPI GetModuleEntry(DWORD dwProcessID,LPCTSTR lpszModuleName,MODULEENTRY32*pEntry);
		DWORD GetParentProcessID(DWORD dwId);
		DWORD GetParentProcessID2(DWORD dwId);

		BOOL GetModuleName(LPVOID lpImageBase,CString&strModuleName);
		BOOL GetModuleName(HANDLE hProcess,LPVOID lpImageBase,DWORD dwImgSize,CString&strModuleName);


		BOOL GetProcessCurDir(HANDLE hProcess,CString&strCurDir);
		BOOL GetProcessCurDir64(HANDLE hProcess,CString&strCurDir);
		BOOL GetProcessCmdLine(HANDLE hProcess,CString&strCmdLine);
		BOOL GetProcessCmdLine64(HANDLE hProcess,CString&strCmdLine);
	}
}
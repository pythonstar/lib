#include "stdafx.h"
#include "CrashDumper.h"
#include <atltrace.h>
#include <atlstr.h>
#include "common.h"


#pragma comment(lib, "Dbghelp")


//CCrashDumper *g_pCrashDumper = NULL;
DWORD CCrashDumper::m_dwMiniDumpType = MiniDumpNormal;

CCrashDumper::CCrashDumper()
{
	//g_pCrashDumper = this;
	//m_dwMiniDumpType = MiniDumpNormal;
}

CCrashDumper::~CCrashDumper()
{
}

void set_unexpected_function() 
{
	throw "unexpected_function";
}

void myInvalidParameterHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved)
{
	//ATLTRACE(_T("invalid parameter in function: %s file: %s line: %d\ndesc: %s\n"), function, file, line, expression);
	//抛出异常触发捕获生成DUMP文件
	throw "invalid_parameter";
}

BOOL Dump(IN struct _EXCEPTION_POINTERS *pException)
{
	BOOL bRet = FALSE;
	CString strDumpFile = Star::Common::GetStartPath() + Star::Common::GetCurrentTimeStr() + _T(".dmp");
	HANDLE hFile = CreateFile(strDumpFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if( hFile == INVALID_HANDLE_VALUE ) {
		//ATLTRACE(_T("[%s] CreateFile failed: %d"), __FUNCTION__, GetLastError());
		return HRESULT_FROM_WIN32(GetLastError());
	}

	MINIDUMP_EXCEPTION_INFORMATION mei;
	mei.ThreadId = GetCurrentThreadId();
	mei.ExceptionPointers = pException;
	mei.ClientPointers = TRUE;
	bRet = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, (MINIDUMP_TYPE)CCrashDumper::m_dwMiniDumpType , &mei, NULL, NULL);
	CloseHandle(hFile);
	//ATLTRACE(_T("[%s] DumpFile ret: %d, error: %08X\n"), __FUNCTION__, strDumpFile, bRet, GetLastError());
	//ATLTRACE(_T("ExceptionCode : %08X, ExceptionFlags : %d , ExceptionAddress : %08X, \n"), 
	//	pException->ExceptionRecord->ExceptionCode, pException->ExceptionRecord->ExceptionFlags, pException->ExceptionRecord->ExceptionAddress);

	return bRet;
}

LONG WINAPI myUnhandledExceptionFilter(__in struct _EXCEPTION_POINTERS *pException)
{
	Dump(pException);
	return EXCEPTION_EXECUTE_HANDLER;
}

void CCrashDumper::SetUnHandleException(DWORD dwMiniDumpType/* = MiniDumpNormal*/)
{
	m_dwMiniDumpType = dwMiniDumpType;
	_set_invalid_parameter_handler(myInvalidParameterHandler);
	_CrtSetReportMode(_CRT_ASSERT, 0);

	set_unexpected(set_unexpected_function);

	LPTOP_LEVEL_EXCEPTION_FILTER pLastUnhandledExceptionFilter = SetUnhandledExceptionFilter(myUnhandledExceptionFilter);
}
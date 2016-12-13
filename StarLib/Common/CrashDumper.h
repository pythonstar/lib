#pragma once
#include <afx.h>
#include <Dbghelp.h>


class CCrashDumper
{
public:
	CCrashDumper();
	~CCrashDumper();

public:
	static void SetUnHandleException(DWORD dwMiniDumpType = MiniDumpNormal);
	static DWORD m_dwMiniDumpType;
};

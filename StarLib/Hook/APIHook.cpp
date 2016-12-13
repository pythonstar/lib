//

#include "stdafx.h"
#include "APIHook.h"


Star::CAPIHook::CAPIHook(LPSTR pszModName, LPSTR pszFuncName, PROC pfnHook)
{
	// 生成新的执行代码
	BYTE btNewBytes[8] = { 0x0B8, 0x0, 0x0, 0x40, 0x0, 0x0FF, 0x0E0, 0 }; 
	memcpy(m_btNewBytes, btNewBytes, 8);
	*(DWORD *)(m_btNewBytes + 1) = (DWORD)pfnHook; 

	// 加载指定模块
	m_hModule = ::LoadLibrary(pszModName);
	if(m_hModule == NULL)
	{
		m_pfnOrig = NULL;
		return;
	}
	m_pfnOrig = ::GetProcAddress(m_hModule, pszFuncName);


	// 修改原API函数执行代码的前8个字节，使它跳向我们的函数
	if(m_pfnOrig != NULL)
	{
		DWORD dwOldProtect;
		MEMORY_BASIC_INFORMATION    mbi;
		VirtualQuery( m_pfnOrig, &mbi, sizeof(mbi) );
		VirtualProtect(m_pfnOrig, 8, PAGE_READWRITE, &dwOldProtect);

		// 保存原来的执行代码
		memcpy(m_btOldBytes, m_pfnOrig, 8);
		// 写入新的执行代码
		::WriteProcessMemory(::GetCurrentProcess(), (void *)m_pfnOrig, 
						m_btNewBytes, sizeof(DWORD)*2, NULL); 
	
		VirtualProtect(m_pfnOrig, 8, mbi.Protect, 0);
	}
}

Star::CAPIHook::~CAPIHook()
{
	Unhook();
	if(m_hModule != NULL)
		::FreeLibrary(m_hModule);
}


void Star::CAPIHook::Rehook()
{
	// 修改原API函数执行代码的前8个字节，使它跳向我们的函数
	if(m_pfnOrig != NULL)
	{
		DWORD dwOldProtect;
		MEMORY_BASIC_INFORMATION    mbi;
		VirtualQuery( m_pfnOrig, &mbi, sizeof(mbi) );
		VirtualProtect(m_pfnOrig, 8, PAGE_READWRITE, &dwOldProtect);

		// 写入新的执行代码
		::WriteProcessMemory(::GetCurrentProcess(), (void *)m_pfnOrig, 
						m_btNewBytes, sizeof(DWORD)*2, NULL); 
	
		VirtualProtect(m_pfnOrig, 8, mbi.Protect, 0);
	}
}

void Star::CAPIHook::Unhook()
{
	if(m_pfnOrig != NULL)
	{
		DWORD dwOldProtect;
		MEMORY_BASIC_INFORMATION    mbi;
		VirtualQuery(m_pfnOrig, &mbi, sizeof(mbi));
		VirtualProtect(m_pfnOrig, 8, PAGE_READWRITE, &dwOldProtect);

		// 写入原来的执行代码
		::WriteProcessMemory(::GetCurrentProcess(), (void *)m_pfnOrig, 
						m_btOldBytes, sizeof(DWORD)*2, NULL); 
	
		VirtualProtect(m_pfnOrig, 8, mbi.Protect, 0);
	}
}
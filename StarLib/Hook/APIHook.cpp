//

#include "stdafx.h"
#include "APIHook.h"


Star::CAPIHook::CAPIHook(LPSTR pszModName, LPSTR pszFuncName, PROC pfnHook)
{
	// �����µ�ִ�д���
	BYTE btNewBytes[8] = { 0x0B8, 0x0, 0x0, 0x40, 0x0, 0x0FF, 0x0E0, 0 }; 
	memcpy(m_btNewBytes, btNewBytes, 8);
	*(DWORD *)(m_btNewBytes + 1) = (DWORD)pfnHook; 

	// ����ָ��ģ��
	m_hModule = ::LoadLibrary(pszModName);
	if(m_hModule == NULL)
	{
		m_pfnOrig = NULL;
		return;
	}
	m_pfnOrig = ::GetProcAddress(m_hModule, pszFuncName);


	// �޸�ԭAPI����ִ�д����ǰ8���ֽڣ�ʹ���������ǵĺ���
	if(m_pfnOrig != NULL)
	{
		DWORD dwOldProtect;
		MEMORY_BASIC_INFORMATION    mbi;
		VirtualQuery( m_pfnOrig, &mbi, sizeof(mbi) );
		VirtualProtect(m_pfnOrig, 8, PAGE_READWRITE, &dwOldProtect);

		// ����ԭ����ִ�д���
		memcpy(m_btOldBytes, m_pfnOrig, 8);
		// д���µ�ִ�д���
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
	// �޸�ԭAPI����ִ�д����ǰ8���ֽڣ�ʹ���������ǵĺ���
	if(m_pfnOrig != NULL)
	{
		DWORD dwOldProtect;
		MEMORY_BASIC_INFORMATION    mbi;
		VirtualQuery( m_pfnOrig, &mbi, sizeof(mbi) );
		VirtualProtect(m_pfnOrig, 8, PAGE_READWRITE, &dwOldProtect);

		// д���µ�ִ�д���
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

		// д��ԭ����ִ�д���
		::WriteProcessMemory(::GetCurrentProcess(), (void *)m_pfnOrig, 
						m_btOldBytes, sizeof(DWORD)*2, NULL); 
	
		VirtualProtect(m_pfnOrig, 8, mbi.Protect, 0);
	}
}

#include "stdafx.h"
#include "APIHook_new.h"
#include <Tlhelp32.h>
#include <ImageHlp.h>
#pragma comment(lib, "ImageHlp")

using namespace Star;
/////////////////////////////////////////////////////////////

// CAPIHook_New���������ͷָ��
CAPIHook_New* CAPIHook_New::sm_pHeader=NULL;

Star::CAPIHook_New::CAPIHook_New(LPSTR pszModName, LPSTR pszFuncName, PROC pfnHook, BOOL bExcludeAPIHookMod)
{
	// �������Hook��������Ϣ
	m_bExcludeAPIHookMod = bExcludeAPIHookMod;
	m_pszModName = pszModName;
	m_pszFuncName = pszFuncName;
	m_pfnHook = pfnHook;
	m_pfnOrig = ::GetProcAddress(::GetModuleHandle(pszModName), pszFuncName);

	// ���˶�����ӵ�������
	m_pNext = sm_pHeader;
	sm_pHeader = this;
	
	// �����е�ǰ�Ѽ��ص�ģ����HOOK�������
	ReplaceIATEntryInAllMods(m_pszModName, m_pfnOrig, m_pfnHook, bExcludeAPIHookMod); 
}

Star::CAPIHook_New::~CAPIHook_New()
{
	// ȡ��������ģ���к�����HOOK
	ReplaceIATEntryInAllMods(m_pszModName, m_pfnHook, m_pfnOrig, m_bExcludeAPIHookMod);

	CAPIHook_New *p = sm_pHeader;

	// ���������Ƴ��˶���
	if(p == this)
	{
		sm_pHeader = p->m_pNext;
	}
	else
	{
		while(p != NULL)
		{
			if(p->m_pNext == this)
			{
				p->m_pNext = this->m_pNext;
				break;
			}
			p = p->m_pNext;
		}
	}

}

void Star::CAPIHook_New::ReplaceIATEntryInAllMods(LPSTR pszExportMod, PROC pfnCurrent, PROC pfnNew, BOOL bExcludeAPIHookMod)
{
	// ȡ�õ�ǰģ��ľ��
	HMODULE hModThis = NULL;
	if(bExcludeAPIHookMod)
	{
		MEMORY_BASIC_INFORMATION mbi;
		if(::VirtualQuery(ReplaceIATEntryInAllMods, &mbi, sizeof(mbi)) != 0)
			hModThis = (HMODULE)mbi.AllocationBase;
	}

	// ȡ�ñ����̵�ģ���б�
	HANDLE hSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, ::GetCurrentProcessId());

	// ��ͼ�޸�ÿ��ģ���е�IAT
	MODULEENTRY32 me = { sizeof(MODULEENTRY32) };
	BOOL bOK = ::Module32First(hSnap, &me);
	while(bOK)
	{
		// ע�⣺���ǲ�HOOK��ǰģ��ĺ���
		if(me.hModule != hModThis)
			ReplaceIATEntryInOneMod(pszExportMod, pfnCurrent, pfnNew, me.hModule);

		bOK = ::Module32Next(hSnap, &me);
	}

}

void Star::CAPIHook_New::ReplaceIATEntryInOneMod(LPSTR pszExportMod, 
				   PROC pfnCurrent, PROC pfnNew, HMODULE hModCaller)
{
	// ȡ��ģ��ĵ�����ַ
	ULONG ulSize;
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)
		ImageDirectoryEntryToData(hModCaller, TRUE, 
		IMAGE_DIRECTORY_ENTRY_IMPORT, &ulSize);

	if(pImportDesc == NULL) // ���ģ��û�е���ڱ�
	{
		return;
	}

	// ���Ұ���pszExportModģ���к���������Ϣ�ĵ���������import descriptor��
	while(pImportDesc->Name != 0)
	{
		LPSTR pszMod = (LPSTR)((DWORD)hModCaller + pImportDesc->Name);
		if(lstrcmpiA(pszMod, pszExportMod) == 0) // �ҵ�
			break;

		pImportDesc++;
	}

	if(pImportDesc->Name == 0) // ���ģ��û�д�pszExportModģ�鵼���κκ���
	{
		return;
	}

	// ȡ�õ����ߵĵ����ַ��import address table, IAT��
	PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)(pImportDesc->FirstThunk + (DWORD)hModCaller);

	// ��������ҪHOOK�ĺ����������ĵ�ַ���º����ĵ�ַ�滻��
	while(pThunk->u1.Function != 0)
	{
		// lpAddrָ����ڴ汣���˺����ĵ�ַ
		PDWORD lpAddr = (PDWORD)&(pThunk->u1.Function);
		if(*lpAddr == (DWORD)pfnCurrent)
		{
			// �޸�ҳ�ı�������
			DWORD dwOldProtect;
			MEMORY_BASIC_INFORMATION mbi;
			VirtualQuery(lpAddr, &mbi, sizeof(mbi));
			VirtualProtect(lpAddr, sizeof(DWORD), PAGE_READWRITE, &dwOldProtect);

			// �޸��ڴ��ַ  �൱�ڡ�*lpAddr = (DWORD)pfnNew;��
			::WriteProcessMemory(::GetCurrentProcess(), 
						lpAddr, &pfnNew, sizeof(DWORD), NULL);

			VirtualProtect(lpAddr, sizeof(DWORD), dwOldProtect, 0);
			break;
		}
		pThunk++;
	}
}



// �ҹ�LoadLibrary��GetProcAddress�������Ա�����Щ�����������Ժ󣬹ҹ��ĺ���Ҳ�ܹ�����ȷ�Ĵ���

CAPIHook_New CAPIHook_New::sm_LoadLibraryA("Kernel32.dll", "LoadLibraryA",   
					(PROC)CAPIHook_New::LoadLibraryA, TRUE);

CAPIHook_New CAPIHook_New::sm_LoadLibraryW("Kernel32.dll", "LoadLibraryW",   
					(PROC)CAPIHook_New::LoadLibraryW, TRUE);

CAPIHook_New CAPIHook_New::sm_LoadLibraryExA("Kernel32.dll", "LoadLibraryExA", 
					(PROC)CAPIHook_New::LoadLibraryExA, TRUE);

CAPIHook_New CAPIHook_New::sm_LoadLibraryExW("Kernel32.dll", "LoadLibraryExW", 
					(PROC)CAPIHook_New::LoadLibraryExW, TRUE);

CAPIHook_New CAPIHook_New::sm_GetProcAddress("Kernel32.dll", "GetProcAddress", 
					(PROC)CAPIHook_New::GetProcAddress, TRUE);



void WINAPI Star::CAPIHook_New::HookNewlyLoadedModule(HMODULE hModule, DWORD dwFlags)
{
	// ���һ���µ�ģ�鱻���أ��ҹ���CAPIHook_New����Ҫ���API����
	if((hModule != NULL) && ((dwFlags&LOAD_LIBRARY_AS_DATAFILE) == 0))
	{
		CAPIHook_New *p = sm_pHeader;
		while(p != NULL)
		{
			ReplaceIATEntryInOneMod(p->m_pszModName, p->m_pfnOrig, p->m_pfnHook, hModule);
			p = p->m_pNext;
		}
	}
}


HMODULE WINAPI Star::CAPIHook_New::LoadLibraryA(PCSTR pszModulePath) 
{
	HMODULE hModule = ::LoadLibraryA(pszModulePath);
	HookNewlyLoadedModule(hModule, 0);
	return(hModule);
}

HMODULE WINAPI Star::CAPIHook_New::LoadLibraryW(PCWSTR pszModulePath) 
{
	HMODULE hModule = ::LoadLibraryW(pszModulePath);
	HookNewlyLoadedModule(hModule, 0);
	return(hModule);
}

HMODULE WINAPI Star::CAPIHook_New::LoadLibraryExA(PCSTR pszModulePath, HANDLE hFile, DWORD dwFlags) 
{
	HMODULE hModule = ::LoadLibraryExA(pszModulePath, hFile, dwFlags);
	HookNewlyLoadedModule(hModule, dwFlags);
	return(hModule);
}

HMODULE WINAPI Star::CAPIHook_New::LoadLibraryExW(PCWSTR pszModulePath, HANDLE hFile, DWORD dwFlags) 
{
	HMODULE hModule = ::LoadLibraryExW(pszModulePath, hFile, dwFlags);
	HookNewlyLoadedModule(hModule, dwFlags);
	return(hModule);
}

FARPROC WINAPI Star::CAPIHook_New::GetProcAddress(HMODULE hModule, PCSTR pszProcName)
{
	// �õ������������ʵ��ַ
	FARPROC pfn = ::GetProcAddress(hModule, pszProcName);

	// �����ǲ�������Ҫhook�ĺ���
	CAPIHook_New *p = sm_pHeader;
	while(p != NULL)
	{
		if(p->m_pfnOrig == pfn)
		{
			pfn = p->m_pfnHook;
			break;
		}

		p = p->m_pNext;
	}

	return pfn;
}

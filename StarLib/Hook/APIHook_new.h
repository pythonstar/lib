// APIHook.h�ļ�
// this file is coded by ����ƽ��
// If you found anything incorrect, please feel free to contact me.

#pragma once
#include <afx.h>

namespace Star
{
	class CAPIHook_New;

	class CAPIHook_New  
	{
	public:
		CAPIHook_New(LPSTR pszModName, LPSTR pszFuncName, PROC pfnHook, BOOL bExcludeAPIHookMod = TRUE);
		virtual ~CAPIHook_New();
		operator PROC() { return m_pfnOrig; }

	// Implementations
	private:
		LPSTR m_pszModName;		// ����ҪHOOK������ģ�������
		LPSTR m_pszFuncName;		// ҪHOOK�ĺ���������
		PROC m_pfnOrig;			// ԭAPI��ˮ����ַ
		PROC m_pfnHook;			// HOOK�����ĵ�ַ
		BOOL m_bExcludeAPIHookMod;	// �Ƿ�HOOK API��ģ���ų�����

	private:
		static void ReplaceIATEntryInAllMods(LPSTR pszExportMod, PROC pfnCurrent, 
					PROC pfnNew, BOOL bExcludeAPIHookMod);
		static void ReplaceIATEntryInOneMod(LPSTR pszExportMod, 
					PROC pfnCurrent, PROC pfnNew, HMODULE hModCaller);


	// ����Ĵ��������������ģ�鶯̬����DLL������
	private:
		// ������ָ����������ǰģ�������е�CAPIHook_New��������һ��
		static CAPIHook_New *sm_pHeader;
		CAPIHook_New *m_pNext;

	private:
		// ��һ���µ�DLL������ʱ�����ô˺���
		static void WINAPI HookNewlyLoadedModule(HMODULE hModule, DWORD dwFlags);

		// �������ٵ�ǰ���̼����µ�DLL
		static HMODULE WINAPI LoadLibraryA(PCSTR  pszModulePath);
		static HMODULE WINAPI LoadLibraryW(PCWSTR pszModulePath);
		static HMODULE WINAPI LoadLibraryExA(PCSTR  pszModulePath, 
			HANDLE hFile, DWORD dwFlags);
		static HMODULE WINAPI LoadLibraryExW(PCWSTR pszModulePath, 
			HANDLE hFile, DWORD dwFlags);
		
		// ������ҹ��ĺ��������󣬷����޸ĺ����ĵ�ַ
		static FARPROC WINAPI GetProcAddress(HMODULE hModule, PCSTR pszProcName);
	private:
		// �Զ�����Щ�������йҹ�
		static CAPIHook_New sm_LoadLibraryA;
		static CAPIHook_New sm_LoadLibraryW;
		static CAPIHook_New sm_LoadLibraryExA;
		static CAPIHook_New sm_LoadLibraryExW;
		static CAPIHook_New sm_GetProcAddress;
	};

}
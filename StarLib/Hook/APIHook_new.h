// APIHook.h文件
// this file is coded by 王艳平，
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
		LPSTR m_pszModName;		// 导出要HOOK函数的模块的名字
		LPSTR m_pszFuncName;		// 要HOOK的函数的名字
		PROC m_pfnOrig;			// 原API汗水大额地址
		PROC m_pfnHook;			// HOOK后函数的地址
		BOOL m_bExcludeAPIHookMod;	// 是否将HOOK API的模块排除在外

	private:
		static void ReplaceIATEntryInAllMods(LPSTR pszExportMod, PROC pfnCurrent, 
					PROC pfnNew, BOOL bExcludeAPIHookMod);
		static void ReplaceIATEntryInOneMod(LPSTR pszExportMod, 
					PROC pfnCurrent, PROC pfnNew, HMODULE hModCaller);


	// 下面的代码用来解决其它模块动态加载DLL的问题
	private:
		// 这两个指针用来将当前模块中所有的CAPIHook_New对象连在一起
		static CAPIHook_New *sm_pHeader;
		CAPIHook_New *m_pNext;

	private:
		// 当一个新的DLL被加载时，调用此函数
		static void WINAPI HookNewlyLoadedModule(HMODULE hModule, DWORD dwFlags);

		// 用来跟踪当前进程加载新的DLL
		static HMODULE WINAPI LoadLibraryA(PCSTR  pszModulePath);
		static HMODULE WINAPI LoadLibraryW(PCWSTR pszModulePath);
		static HMODULE WINAPI LoadLibraryExA(PCSTR  pszModulePath, 
			HANDLE hFile, DWORD dwFlags);
		static HMODULE WINAPI LoadLibraryExW(PCWSTR pszModulePath, 
			HANDLE hFile, DWORD dwFlags);
		
		// 如果被挂钩的函数被请求，返回修改后函数的地址
		static FARPROC WINAPI GetProcAddress(HMODULE hModule, PCSTR pszProcName);
	private:
		// 自动对这些函数进行挂钩
		static CAPIHook_New sm_LoadLibraryA;
		static CAPIHook_New sm_LoadLibraryW;
		static CAPIHook_New sm_LoadLibraryExA;
		static CAPIHook_New sm_LoadLibraryExW;
		static CAPIHook_New sm_GetProcAddress;
	};

}
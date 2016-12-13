//////////////////////////////////////////////////////////
// ApiHook.h

#pragma once
#include <afx.h>

namespace Star
{

	class CAPIHook
	{
	public:
		CAPIHook(LPSTR pszModName, LPSTR pszFuncName, PROC pfnHook);
		~CAPIHook();

		void Unhook();
		void Rehook();
	protected:
		PROC m_pfnOrig;                 // �Ǹ�������������ַ
		BYTE m_btNewBytes[8]; 
		BYTE m_btOldBytes[8];
		HMODULE m_hModule;
	};

}

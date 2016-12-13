#pragma once
#include <atlstr.h>
#ifdef _USRDLL

namespace Star
{
	namespace File
	{
		class CIni
		{
		public:
			CIni(LPCTSTR lpszFileName);
			CIni();

			void SetIniFile(LPCTSTR lpszFileName);
			CString GetIniString(LPCTSTR appName, LPCTSTR keyName,LPCTSTR lpszDefault=NULL);
			BOOL SetIniString(LPCTSTR appName, LPCTSTR keyName, LPCTSTR value);
			UINT GetIniInt(LPCTSTR appName, LPCTSTR keyName,int nDefault=0);
			BOOL SetIniInt(LPCTSTR appName, LPCTSTR keyName, UINT value);
		private:
			CString m_strFileName;
		};
	}
}
#endif

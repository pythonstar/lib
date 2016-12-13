// GetWinVer.cpp  Version 1.1
//
// Copyright (C) 2001-2003 Hans Dietrich
//
// This software is released into the public domain.  
// You are free to use it in any way you like, except
// that you may not sell this source code.
//
// This software is provided "as is" with no expressed 
// or implied warranty.  I accept no liability for any 
// damage or loss of business that this software may cause. 
//
///////////////////////////////////////////////////////////////////////////////

#include "common/header.h"
#include "common/version.h"
#pragma comment(lib, "version.lib")

// from winbase.h
#ifndef VER_PLATFORM_WIN32s
#define VER_PLATFORM_WIN32s             0
#endif
#ifndef VER_PLATFORM_WIN32_WINDOWS
#define VER_PLATFORM_WIN32_WINDOWS      1
#endif
#ifndef VER_PLATFORM_WIN32_NT
#define VER_PLATFORM_WIN32_NT           2
#endif
#ifndef VER_PLATFORM_WIN32_CE
#define VER_PLATFORM_WIN32_CE           3
#endif


/*
    This table has been assembled from Usenet postings, personal
    observations, and reading other people's code.  Please feel
    free to add to it or correct it.


         dwPlatFormID  dwMajorVersion  dwMinorVersion  dwBuildNumber
95             1              4               0             950
95 SP1         1              4               0        >950 && <=1080
95 OSR2        1              4             <10           >1080
98             1              4              10            1998
98 SP1         1              4              10       >1998 && <2183
98 SE          1              4              10          >=2183
ME             1              4              90            3000

NT 3.51        2              3              51
NT 4           2              4               0            1381
2000           2              5               0            2195
XP             2              5               1            2600
2003 Server    2              5               2            3790

CE             3

*/
namespace XGC
{
	namespace common
	{
		///////////////////////////////////////////////////////////////////////////////
		// GetWinVer
		BOOL GetWinVer(LPTSTR pszVersion, int *nVersion, LPTSTR pszMajorMinorBuild)
		{
			if (!pszVersion || !nVersion || !pszMajorMinorBuild)
				return FALSE;
			lstrcpy(pszVersion, WUNKNOWNSTR);
			*nVersion = WUNKNOWN;

			OSVERSIONINFO osinfo;
			osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

			if (!GetVersionEx(&osinfo))
				return FALSE;

			DWORD dwPlatformId   = osinfo.dwPlatformId;
			DWORD dwMinorVersion = osinfo.dwMinorVersion;
			DWORD dwMajorVersion = osinfo.dwMajorVersion;
			DWORD dwBuildNumber  = osinfo.dwBuildNumber & 0xFFFF;	// Win 95 needs this

			wsprintf(pszMajorMinorBuild, _T("%u.%u.%u"), dwMajorVersion, dwMinorVersion, dwBuildNumber);

			if ((dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) && (dwMajorVersion == 4))
			{
				if ((dwMinorVersion < 10) && (dwBuildNumber == 950))
				{
					lstrcpy(pszVersion, W95STR);
					*nVersion = W95;
				}
				else if ((dwMinorVersion < 10) && 
						((dwBuildNumber > 950) && (dwBuildNumber <= 1080)))
				{
					lstrcpy(pszVersion, W95SP1STR);
					*nVersion = W95SP1;
				}
				else if ((dwMinorVersion < 10) && (dwBuildNumber > 1080))
				{
					lstrcpy(pszVersion, W95OSR2STR);
					*nVersion = W95OSR2;
				}
				else if ((dwMinorVersion == 10) && (dwBuildNumber == 1998))
				{
					lstrcpy(pszVersion, W98STR);
					*nVersion = W98;
				}
				else if ((dwMinorVersion == 10) && 
						((dwBuildNumber > 1998) && (dwBuildNumber < 2183)))
				{
					lstrcpy(pszVersion, W98SP1STR);
					*nVersion = W98SP1;
				}
				else if ((dwMinorVersion == 10) && (dwBuildNumber >= 2183))
				{
					lstrcpy(pszVersion, W98SESTR);
					*nVersion = W98SE;
				}
				else if (dwMinorVersion == 90)
				{
					lstrcpy(pszVersion, WMESTR);
					*nVersion = WME;
				}
			}
			else if (dwPlatformId == VER_PLATFORM_WIN32_NT)
			{
				if ((dwMajorVersion == 3) && (dwMinorVersion == 51))
				{
					lstrcpy(pszVersion, WNT351STR);
					*nVersion = WNT351;
				}
				else if ((dwMajorVersion == 4) && (dwMinorVersion == 0))
				{
					lstrcpy(pszVersion, WNT4STR);
					*nVersion = WNT4;
				}
				else if ((dwMajorVersion == 5) && (dwMinorVersion == 0))
				{
					lstrcpy(pszVersion, W2KSTR);
					*nVersion = W2K;
				}
				else if ((dwMajorVersion == 5) && (dwMinorVersion == 1))
				{
					lstrcpy(pszVersion, WXPSTR);
					*nVersion = WXP;
				}
				else if ((dwMajorVersion == 5) && (dwMinorVersion == 2))
				{
					lstrcpy(pszVersion, W2003SERVERSTR);
					*nVersion = W2003SERVER;
				}
			}
			else if (dwPlatformId == VER_PLATFORM_WIN32_CE)
			{
				lstrcpy(pszVersion, WCESTR);
				*nVersion = WCE;
			}
			return TRUE;
		}

		///////////////////////////////////////////////////////////////////////////////
		// ctor
		CMiniVersion::CMiniVersion(xgc_lpctstr lpszPath)
		{
			ZeroMemory(m_szPath, sizeof(m_szPath));

			if (lpszPath && lpszPath[0] != 0)
			{
				lstrcpyn(m_szPath, lpszPath, sizeof(m_szPath)-1);
			}
			else
			{
			}

			m_pData = NULL;
			m_dwHandle = 0;

			for (int i = 0; i < 4; i++)
			{
				m_wFileVersion[i] = 0;
				m_wProductVersion[i] = 0;
			}

			m_dwFileFlags = 0;
			m_dwFileOS = 0;
			m_dwFileType = 0;
			m_dwFileSubtype = 0;

			ZeroMemory(m_szCompanyName, sizeof(m_szCompanyName));
			ZeroMemory(m_szProductName, sizeof(m_szProductName));
			ZeroMemory(m_szFileDescription, sizeof(m_szFileDescription));

			Init();
		}

		///////////////////////////////////////////////////////////////////////////////
		// Init
		BOOL CMiniVersion::Init()
		{
			DWORD dwHandle;
			DWORD dwSize;
			BOOL rc;

			dwSize = ::GetFileVersionInfoSize( m_szPath, &dwHandle );
			if (dwSize == 0)
				return FALSE;

			m_pData = new BYTE [dwSize + 1];	
			ZeroMemory(m_pData, dwSize+1);

			rc = ::GetFileVersionInfo( m_szPath, dwHandle, dwSize, m_pData);
			if (!rc)
				return FALSE;

			// get fixed info

			VS_FIXEDFILEINFO FixedInfo;

			if (GetFixedInfo(FixedInfo))
			{
				m_wFileVersion[0] = HIWORD(FixedInfo.dwFileVersionMS);
				m_wFileVersion[1] = LOWORD(FixedInfo.dwFileVersionMS);
				m_wFileVersion[2] = HIWORD(FixedInfo.dwFileVersionLS);
				m_wFileVersion[3] = LOWORD(FixedInfo.dwFileVersionLS);

				m_wProductVersion[0] = HIWORD(FixedInfo.dwProductVersionMS);
				m_wProductVersion[1] = LOWORD(FixedInfo.dwProductVersionMS);
				m_wProductVersion[2] = HIWORD(FixedInfo.dwProductVersionLS);
				m_wProductVersion[3] = LOWORD(FixedInfo.dwProductVersionLS);

				m_dwFileFlags   = FixedInfo.dwFileFlags;
				m_dwFileOS      = FixedInfo.dwFileOS;
				m_dwFileType    = FixedInfo.dwFileType;
				m_dwFileSubtype = FixedInfo.dwFileSubtype;
			}
			else
				return FALSE;

			// get string info

			GetStringInfo(_T("CompanyName"),     m_szCompanyName);
			GetStringInfo(_T("FileDescription"), m_szFileDescription);
			GetStringInfo(_T("ProductName"),     m_szProductName);

			return TRUE;		
		}

		///////////////////////////////////////////////////////////////////////////////
		// Release
		void CMiniVersion::Release()
		{
			// do this manually, because we can't use objects requiring
			// a dtor within an exception handler
			if (m_pData)
				delete [] m_pData;
			m_pData = NULL;
		}


		///////////////////////////////////////////////////////////////////////////////
		// GetFileVersion
		BOOL CMiniVersion::GetFileVersion(WORD * pwVersion)
		{
			for (int i = 0; i < 4; i++)
				*pwVersion++ = m_wFileVersion[i];
			return TRUE;
		}					 	 

		///////////////////////////////////////////////////////////////////////////////
		// GetProductVersion
		BOOL CMiniVersion::GetProductVersion(WORD * pwVersion)
		{
			for (int i = 0; i < 4; i++)
				*pwVersion++ = m_wProductVersion[i];
			return TRUE;
		}

		///////////////////////////////////////////////////////////////////////////////
		// GetFileFlags
		BOOL CMiniVersion::GetFileFlags(DWORD& rdwFlags)
		{
			rdwFlags = m_dwFileFlags;
			return TRUE;
		}

		///////////////////////////////////////////////////////////////////////////////
		// GetFileOS
		BOOL CMiniVersion::GetFileOS(DWORD& rdwOS)
		{
			rdwOS = m_dwFileOS;
			return TRUE;
		}

		///////////////////////////////////////////////////////////////////////////////
		// GetFileType
		BOOL CMiniVersion::GetFileType(DWORD& rdwType)
		{
			rdwType = m_dwFileType;
			return TRUE;
		}

		///////////////////////////////////////////////////////////////////////////////
		// GetFileSubtype
		BOOL CMiniVersion::GetFileSubtype(DWORD& rdwType)
		{
			rdwType = m_dwFileSubtype;
			return TRUE;
		}

		///////////////////////////////////////////////////////////////////////////////
		// GetCompanyName
		BOOL CMiniVersion::GetCompanyName(LPTSTR lpszCompanyName, int nSize)
		{
			if (!lpszCompanyName)
				return FALSE;
			ZeroMemory(lpszCompanyName, nSize);
			lstrcpyn(lpszCompanyName, m_szCompanyName, nSize-1);
			return TRUE;
		}

		///////////////////////////////////////////////////////////////////////////////
		// GetFileDescription
		BOOL CMiniVersion::GetFileDescription(LPTSTR lpszFileDescription, int nSize)
		{
			if (!lpszFileDescription)
				return FALSE;
			ZeroMemory(lpszFileDescription, nSize);
			lstrcpyn(lpszFileDescription, m_szFileDescription, nSize-1);
			return TRUE;
		}

		///////////////////////////////////////////////////////////////////////////////
		// GetProductName
		BOOL CMiniVersion::GetProductName(LPTSTR lpszProductName, int nSize)
		{
			if (!lpszProductName)
				return FALSE;
			ZeroMemory(lpszProductName, nSize);
			lstrcpyn(lpszProductName, m_szProductName, nSize-1);
			return TRUE;
		}



		///////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////
		//
		// protected methods
		//
		///////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////


		///////////////////////////////////////////////////////////////////////////////
		// GetFixedInfo
		BOOL CMiniVersion::GetFixedInfo(VS_FIXEDFILEINFO& rFixedInfo)
		{
			BOOL rc;
			UINT nLength;
			VS_FIXEDFILEINFO *pFixedInfo = NULL;

			if (!m_pData)
				return FALSE;

			if (m_pData)
				rc = ::VerQueryValue(m_pData, _T("\\"), (void **) &pFixedInfo, &nLength);
			else
				rc = FALSE;

			if (rc)
				memcpy (&rFixedInfo, pFixedInfo, sizeof (VS_FIXEDFILEINFO));	

			return rc;
		}

		///////////////////////////////////////////////////////////////////////////////
		// GetStringInfo
		BOOL CMiniVersion::GetStringInfo(xgc_lpctstr lpszKey, LPTSTR lpszReturnValue)
		{
			BOOL rc;
			DWORD *pdwTranslation;
			UINT nLength;
			LPTSTR lpszValue;

			if (m_pData == NULL)
				return FALSE;

			if (!lpszReturnValue)
				return FALSE;

			if (!lpszKey)
				return FALSE;

			*lpszReturnValue = 0;

			rc = ::VerQueryValue(m_pData, _T("\\VarFileInfo\\Translation"), 
				(void**) &pdwTranslation, &nLength);
			if (!rc)
				return FALSE;

			TCHAR szKey[2000];
			_sntprintf(szKey, _countof(szKey), _T("\\StringFileInfo\\%04x%04x\\%s"),
				LOWORD (*pdwTranslation), HIWORD (*pdwTranslation),
				lpszKey);

			rc = ::VerQueryValue(m_pData, szKey, (void**) &lpszValue, &nLength);

			if (!rc)
				return FALSE;

			lstrcpy(lpszReturnValue, lpszValue);

			return TRUE;
		}
	}
}
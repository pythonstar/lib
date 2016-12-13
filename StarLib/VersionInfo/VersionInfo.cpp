// VersionInfo.cpp : implementation file
//

#include "stdafx.h"
#include "VersionInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

struct STranslation
{
	WORD wLangID;
	WORD wCharSet;
};

//_strDLLFilename should be just the filename, with no path. e.g. "blah.dll"
//returns the file-version with product-version appended
//if the file-version is in the "1, 1, 0, 2" format, returns that part in A.B.CC.DDD format
//(however skips part DDD if it's zero, and also skips part CC if both CC and DDD are zero)
//e,g if file-version is "1, 0, 3, 0" and product-version is "beta 2", returns "1.0.03 beta 2"
CString CVersionInfo::GetVersion(const CString& _strDLLFilename)
{
	CString strVersion;

	TCHAR szDLLFilename[MAX_PATH];
	lstrcpy(szDLLFilename, _strDLLFilename);

	DWORD dwUnused = 0;
	DWORD dwSize = GetFileVersionInfoSize(szDLLFilename, &dwUnused);

	if (dwSize > 0)
	{
		LPBYTE lpBuffer = new BYTE[dwSize];
		GetFileVersionInfo(szDLLFilename, 0, dwSize, lpBuffer);

		LPVOID lpvi;
		UINT nSize = 0;
		if (VerQueryValue(lpBuffer, _T("\\VarFileInfo\\Translation"), &lpvi, &nSize) && (nSize >= 4))
		{
			struct STranslation sTrans = *(STranslation*)lpvi;

			CString strSubBlockFileVersion;
			CString strSubBlockProductVersion;
			strSubBlockFileVersion.Format(_T("\\StringFileInfo\\%04x%04x\\FileVersion"), sTrans.wLangID, sTrans.wCharSet);
			strSubBlockProductVersion.Format(_T("\\StringFileInfo\\%04x%04x\\ProductVersion"), sTrans.wLangID, sTrans.wCharSet);
			LPTSTR pBufFileVersion;
			LPTSTR pBufProductVersion;
			if ( (::VerQueryValue(lpBuffer, (LPTSTR) ((LPCTSTR)strSubBlockFileVersion), (LPVOID*)(&pBufFileVersion), &nSize)) &&
				 (::VerQueryValue(lpBuffer, (LPTSTR) ((LPCTSTR)strSubBlockProductVersion), (LPVOID*)(&pBufProductVersion), &nSize)) )
			{
				CString strFileVersion = pBufFileVersion;
				CString strProductVersion = pBufProductVersion;
				strVersion = ReformatVersionString(strFileVersion, strProductVersion);
			}
		}
		delete[] lpBuffer;
	}
	return strVersion;
}

CString CVersionInfo::ReformatVersionString(LPCTSTR _pszFileVersion,
											LPCTSTR _pszProductVersion)
{
	////////////////////
	// File Version
	//
	CString strFileVersion = _pszFileVersion;
	strFileVersion.Replace(_T(" "), _T(""));	//remove spaces

	TCHAR* pszFileVersionLocal = new TCHAR[strFileVersion.GetLength()+1];
	lstrcpy(pszFileVersionLocal, strFileVersion);

	TCHAR seps[] = _T(",");
	TCHAR* token = _tcstok(pszFileVersionLocal, seps);

	CStringArray arrParts;
	while (token != NULL)
	{
		arrParts.Add(token);
		token = _tcstok(NULL, seps);
	}

	if (arrParts.GetSize() == 4)
	{
		//if all parts are meaningful, return A.B.CC.DDD
		//if part 4 is 0, return A.B.CC
		//if part 3 is also 0, return A.B
		CString strA;
		CString strB;
		CString strC;
		CString strD;

		strA.Format(_T("%s"), arrParts[0]);
		strB.Format(_T(".%s"), arrParts[1]);

		if ((arrParts[2] != _T("0")) || (arrParts[3] != _T("0")))
		{
			strC.Format(_T(".%02s"), arrParts[2]);

			if (arrParts[3] != _T("0"))
			{
				strD.Format(_T(".%03s"), arrParts[3]);
			}
		}

		strFileVersion = strA + strB + strC + strD;
	}
	else
	{
		//not in the expected "A, B, C, D" format - return the entire string
		strFileVersion = _pszFileVersion;
	}

	delete[] pszFileVersionLocal;


	////////////////////
	// Product Version
	//
	//don't try to be clever with this, just return the string
	CString strProductVersion = _pszProductVersion;


	CString strReturn;
	if (strProductVersion.IsEmpty())
	{
		strReturn.Format(_T("%s"), strFileVersion);
	}
	else
	{
		strReturn.Format(_T("%s %s"), strFileVersion, strProductVersion);
	}
	return strReturn;
}

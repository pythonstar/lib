#include "stdafx.h"
#include <ShlObj.h>
#include "File.h"
#include <afxdlgs.h>

/*------------------------------------------------------------------------
[7/24/2009 ]
说明:  让用户选择一个文件夹或目录,返回选择的路径.
------------------------------------------------------------------------*/
CString	Star::File::BrowseFolder(HWND hWnd, LPCTSTR lpszDefaultPath/* = NULL*/)
{
	BOOL bRet = FALSE;
	TCHAR szBuffer[MAX_PATH * 2] = { 0 };
	BROWSEINFO bi;

	ZeroMemory(&bi, sizeof(bi));
	bi.hwndOwner = hWnd;
	bi.ulFlags   = BIF_RETURNONLYFSDIRS;
	bi.lpszTitle = _T("请选择文件夹:");
	bi.lParam = (LPARAM)lpszDefaultPath;
	bi.pszDisplayName = szBuffer;

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	if (pidl) {
		if (SHGetPathFromIDList(pidl, szBuffer)) {
			bRet = TRUE;
		}
		IMalloc *pMalloc = NULL;
		if (SUCCEEDED(SHGetMalloc(&pMalloc)) && pMalloc) { 
			pMalloc->Free(pidl); 
			pMalloc->Release();
		}
	}

	return szBuffer;
}

int Star::File::BrowseFiles(vector<CString>&vtFiles, LPCTSTR lpszFilter)
{
	const int MAXFILENUM = 1000;
	LPCTSTR lpszFilterInternel = lpszFilter;
	if ( lpszFilterInternel==NULL ) {
		lpszFilterInternel = _T("All Files(*.*)|*.*||");
	}
	CFileDialog filedlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT |OFN_ALLOWMULTISELECT, lpszFilterInternel);
	vtFiles.clear();

	TCHAR *pBuffer = new TCHAR[MAX_PATH * MAXFILENUM];
	filedlg.m_ofn.lpstrFile = pBuffer;
	filedlg.m_ofn.nMaxFile = MAX_PATH * MAXFILENUM;
	filedlg.m_ofn.lpstrFile[0] = '\0';

	if (filedlg.DoModal() == IDOK) {
		CString cstrfilepath = _T("");
		POSITION pos = filedlg.GetStartPosition();
		while (pos != NULL) {
			vtFiles.push_back(filedlg.GetNextPathName(pos));
		}
	}

	if ( pBuffer!=NULL ) {
		delete[] pBuffer;
	}

	return (int)vtFiles.size();
}

CString Star::File::GetIniString(LPCTSTR fileName,LPCTSTR appName, LPCTSTR keyName,LPCTSTR lpszDefault)
{
	TCHAR szTemp[MAX_PATH*2] = {0};
	DWORD size = GetPrivateProfileString(appName, keyName, lpszDefault, szTemp, sizeof(szTemp), fileName);
	return szTemp;
}

BOOL Star::File::SetIniString(LPCTSTR fileName,LPCTSTR appName, LPCTSTR keyName, LPCTSTR value)
{
	return WritePrivateProfileString(appName, keyName, value, fileName);
}

UINT Star::File::GetIniInt(LPCTSTR fileName,LPCTSTR appName, LPCTSTR keyName,int nDefault)
{
	return GetPrivateProfileInt(appName, keyName, nDefault, fileName);
}

BOOL Star::File::SetIniInt(LPCTSTR fileName,LPCTSTR appName, LPCTSTR keyName, UINT value)
{
	CString strValue;
	strValue.Format(_T("%d"), value);
	return WritePrivateProfileString(appName, keyName, strValue, fileName);
}

#pragma once
#include <list>
#include <vector>
using namespace std;

namespace Star
{
	namespace File
	{
		CString	BrowseFolder(HWND hWnd = NULL, LPCTSTR lpszDefaultPath = NULL);
		int BrowseFiles(vector<CString>&vtFiles, LPCTSTR lpszFilter = NULL);
		CString GetIniString(LPCTSTR fileName,LPCTSTR appName, LPCTSTR keyName,LPCTSTR lpszDefault=NULL);
		BOOL SetIniString(LPCTSTR fileName,LPCTSTR appName, LPCTSTR keyName, LPCTSTR value);
		UINT GetIniInt(LPCTSTR fileName,LPCTSTR appName, LPCTSTR keyName,int nDefault=0);
		BOOL SetIniInt(LPCTSTR fileName,LPCTSTR appName, LPCTSTR keyName, UINT value);
	}
}
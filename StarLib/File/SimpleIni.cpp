
#include "stdafx.h"
#include "SimpleIni.h"
#ifdef _USRDLL

Star::File::CIni::CIni(LPCTSTR lpszFileName)
{
	m_strFileName = lpszFileName;
}

Star::File::CIni::CIni()
{
	m_strFileName.Empty();
}

void Star::File::CIni::SetIniFile(LPCTSTR lpszFileName)
{
	m_strFileName = lpszFileName;
}

CString Star::File::CIni::GetIniString(LPCTSTR appName, LPCTSTR keyName,LPCTSTR lpszDefault/*=NULL*/)
{
	TCHAR szTemp[MAX_PATH*2] = {0};
	DWORD size = GetPrivateProfileString(appName, keyName, lpszDefault, szTemp, sizeof(szTemp), m_strFileName);
	return szTemp;
}

BOOL Star::File::CIni::SetIniString(LPCTSTR appName, LPCTSTR keyName, LPCTSTR value)
{
	return WritePrivateProfileString(appName, keyName, value, m_strFileName);
}

UINT Star::File::CIni::GetIniInt(LPCTSTR appName, LPCTSTR keyName,int nDefault/*=0*/)
{
	return GetPrivateProfileInt(appName, keyName, nDefault, m_strFileName);
}

BOOL Star::File::CIni::SetIniInt(LPCTSTR appName, LPCTSTR keyName, UINT value)
{
	CString strValue;
	strValue.Format(_T("%d"), value);
	return WritePrivateProfileString(appName, keyName, strValue, m_strFileName);
}
#endif


#ifndef _VERSIONINFO_H_
#define _VERSIONINFO_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma comment (lib,"version.lib")

class CVersionInfo
{
public:
	CVersionInfo()	{ /*don't instantiate me*/ }
	~CVersionInfo()	{ /*don't instantiate me*/ }

	static CString GetVersion(const CString& _strDLLFilename);

private:
	static CString ReformatVersionString(LPCTSTR _pszFileVersion, LPCTSTR _pszProductVersion);
};


#endif//_VERSIONINFO_H_

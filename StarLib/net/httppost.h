
#pragma once
#include <atlstr.h>
#include <WinInet.h>
#include <vector>
using namespace std;

CString SendHttpData(const CString&strHost, const CString&strPath, const vector<CString>&vtHeaders, const CString&strSendData, CString&strNewCookie,
					 BOOL bNeedDocode = TRUE, int nMethod = 0);
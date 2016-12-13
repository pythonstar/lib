#include "stdafx.h"
#include <afxinet.h>
#include <afxwin.h>
#include "htmldown.h"
#include "../Common/common.h"

#pragma comment(lib,"Urlmon.lib")


HRESULT DownloadFileCachely(const CString&strUrl,LPCTSTR lpszSaveAs,int nTimeOutSeconds)
{
	HRESULT hRes = URLDownloadToFile(NULL, strUrl, lpszSaveAs, 0, NULL);
	//if ( FAILED(hRes) ){
	//	//INET_E_DOWNLOAD_FAILURE 0x800C0008L
	//}
	return hRes;  
}

#pragma warning(disable:4102)

HRESULT DownloadFileNoCache(const CString&strUrl,LPCTSTR lpszSaveAs,int nTimeOutSeconds)
{
	HRESULT dwHttpStatus=0x80000000;

	CString       strServer; 
	CString       strObject; 
	INTERNET_PORT nPort = INTERNET_DEFAULT_HTTP_PORT;
	DWORD         dwServiceType;
	HINTERNET	  hInternetSession=NULL;
	HINTERNET     hHttpConnection=NULL;
	HINTERNET     hHttpFile=NULL;
	HANDLE		  hFile = INVALID_HANDLE_VALUE;

	if (!AfxParseURL(strUrl, dwServiceType, strServer, strObject, nPort))
	{
		//Try sticking "http://" before it
		if (!AfxParseURL(_T("http://") + strUrl, dwServiceType, strServer, strObject, nPort))
		{
			TRACE(_T("Failed to parse the URL: %s\n"), strUrl);
			return dwHttpStatus;
		}
	}
	if ( nPort==INTERNET_DEFAULT_HTTPS_PORT/* || dwServiceType!=INTERNET_SERVICE_HTTP*/ ) {
		return E_INVALIDARG;	//https不支持，返回错误的参数
	}

	//Create the Internet session handle
	hInternetSession = ::InternetOpen(AfxGetAppName(), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (hInternetSession == NULL)
	{
		TRACE(_T("Failed in call to InternetOpen, Error:%d\n"), ::GetLastError());
		return dwHttpStatus;
	}

	//Make the connection to the HTTP server
	hHttpConnection = ::InternetConnect(hInternetSession, strServer, nPort, NULL, 
		NULL, dwServiceType, 0, NULL);
	if (hHttpConnection == NULL)
	{
		TRACE(_T("Failed in call to InternetConnect, Error:%d\n"), ::GetLastError());
		return dwHttpStatus;
	}

	//设置下载超时时间，默认10秒
	nTimeOutSeconds*=1000;
	//nTimeOutSeconds=1;
	InternetSetOption(hHttpConnection,INTERNET_OPTION_RECEIVE_TIMEOUT,&nTimeOutSeconds,sizeof(nTimeOutSeconds));


	//Issue the request to read the file
	LPCTSTR ppszAcceptTypes[2];
	ppszAcceptTypes[0] = _T("*/*");  //We support accepting any mime file type since this is a simple download of a file
	ppszAcceptTypes[1] = NULL;
	ASSERT(hHttpFile == NULL);
	hHttpFile = HttpOpenRequest(hHttpConnection, NULL, strObject, NULL, NULL, ppszAcceptTypes, INTERNET_FLAG_RELOAD | 
		INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_KEEP_CONNECTION, NULL);
	if (hHttpFile == NULL)
	{
		TRACE(_T("Failed in call to HttpOpenRequest, Error:%d\n"), ::GetLastError());
		return dwHttpStatus;
	}

	//label used to jump to if we need to resend the request
resend:

	//////////////////////////////////////////////////////////////////////////
	//Issue the request
	BOOL bSend = ::HttpSendRequestEx(hHttpFile, NULL, NULL, 0, 0);
	if (!bSend){
		TRACE(_T("Failed in call to HttpSendRequestEx, Error:%d\n"), ::GetLastError());
		return dwHttpStatus;
	}
	bSend = HttpEndRequest(hHttpFile,NULL,0,0);
	if (!bSend){
		TRACE(_T("Failed in call to HttpEndRequest, Error:%d\n"), ::GetLastError());
		return dwHttpStatus;
	}
	//////////////////////////////////////////////////////////////////////////

	//Check the HTTP status code
	TCHAR szStatusCode[32];
	DWORD dwInfoSize = 32;
	if (!HttpQueryInfo(hHttpFile, HTTP_QUERY_STATUS_CODE, szStatusCode, &dwInfoSize, NULL))
	{
		TRACE(_T("Failed in call to HttpQueryInfo for HTTP query status code, Error:%d\n"), ::GetLastError());
		return dwHttpStatus;
	}
	else
	{
		long nStatusCode = _ttol(szStatusCode);

		//Handle any authentication errors
		if (nStatusCode == HTTP_STATUS_PROXY_AUTH_REQ || nStatusCode == HTTP_STATUS_DENIED)
		{
			// We have to read all outstanding data on the Internet handle
			// before we can resubmit request. Just discard the data.
			char szData[51];
			DWORD dwSize;
			do
			{
				::InternetReadFile(hHttpFile, (LPVOID)szData, 50, &dwSize);
			}
			while (dwSize != 0);
		}
		else if (nStatusCode != HTTP_STATUS_OK)
		{
//			TRACE(_T("Failed to retrieve a HTTP 200 status, Status Code:%d\n"), nStatusCode);
			return dwHttpStatus;
		}
	}

	// Get the length of the file.            
	TCHAR szContentLength[32];
	dwInfoSize = 32;
	DWORD dwFileSize = 0;
	BOOL bGotFileSize = FALSE;
	if (::HttpQueryInfo(hHttpFile, HTTP_QUERY_CONTENT_LENGTH, szContentLength, &dwInfoSize, NULL))
	{
		//Set the progress control range
		bGotFileSize = TRUE;
		dwFileSize = (DWORD) _ttol(szContentLength);
	}

	//Now do the actual read of the file
	hFile = CreateFile(lpszSaveAs,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if ( hFile==INVALID_HANDLE_VALUE ){
		//TRACE(_T("Failed to create file :%s\n"), lpszSaveAs);
		return dwHttpStatus;
	}
	DWORD dwStartTicks = ::GetTickCount();
	DWORD dwCurrentTicks = dwStartTicks;
	DWORD dwBytesRead = 0;
	DWORD dwWritten = 0;
	char szReadBuf[1024*10];
	DWORD dwBytesToRead = 1024*10;
	DWORD dwTotalBytesRead = 0;
	DWORD dwLastTotalBytes = 0;
	DWORD dwLastPercentage = 0;
	do
	{
		if (!::InternetReadFile(hHttpFile, szReadBuf, dwBytesToRead, &dwBytesRead)){
			//TRACE(_T("Failed in call to InternetReadFile, Error:%d\n"), ::GetLastError());
			return dwHttpStatus;
		}else if ( dwBytesRead ){
			//Write the data to file
			WriteFile(hFile,szReadBuf,dwBytesRead,&dwWritten,NULL);

			//Increment the total number of bytes read
			dwTotalBytesRead += dwBytesRead;  
		}
	}while (dwBytesRead);

	//Delete the file being downloaded to if it is present and the download was aborted
	CloseHandle(hFile);
	dwHttpStatus=0;

	//We're finished
	return dwHttpStatus;
}

HRESULT GetHttpFileContentNoCache(const CString&strUrl, CString&strHtml,int nTimeOutSeconds)
{
	HRESULT dwHttpStatus=0x80000000;
	strHtml.Empty();

	CString       strServer; 
	CString       strObject; 
	INTERNET_PORT nPort = INTERNET_DEFAULT_HTTP_PORT;
	DWORD         dwServiceType = 0;
	HINTERNET	  hInternetSession=NULL;
	HINTERNET     hHttpConnection=NULL;
	HINTERNET     hHttpFile=NULL;
	HANDLE		  hFile = INVALID_HANDLE_VALUE;

	if (!AfxParseURL(strUrl, dwServiceType, strServer, strObject, nPort))
	{
		//Try sticking "http://" before it
		if (!AfxParseURL(_T("http://") + strUrl, dwServiceType, strServer, strObject, nPort))
		{
			TRACE(_T("Failed to parse the URL: %s\n"), strUrl);
			return dwHttpStatus;
		}
	}
	if ( nPort==INTERNET_DEFAULT_HTTPS_PORT/* || dwServiceType!=INTERNET_SERVICE_HTTP*/ ) {
		return E_INVALIDARG;	//https不支持，返回错误的参数
	}

	//Create the Internet session handle
	hInternetSession = ::InternetOpen(NULL, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (hInternetSession == NULL)
	{
		TRACE(_T("Failed in call to InternetOpen, Error:%d\n"), ::GetLastError());
		return dwHttpStatus;
	}

	//Make the connection to the HTTP server
	hHttpConnection = ::InternetConnect(hInternetSession, strServer, nPort, NULL, NULL, dwServiceType, 0, NULL);
	if (hHttpConnection == NULL)
	{
		TRACE(_T("Failed in call to InternetConnect, Error:%d\n"), ::GetLastError());
		return dwHttpStatus;
	}

	//设置下载超时时间，默认10秒
	nTimeOutSeconds*=1000;
	//nTimeOutSeconds=1;
	InternetSetOption(hHttpConnection,INTERNET_OPTION_RECEIVE_TIMEOUT,&nTimeOutSeconds,sizeof(nTimeOutSeconds));


	//Issue the request to read the file
	LPCTSTR ppszAcceptTypes[2];
	ppszAcceptTypes[0] = _T("*/*");  //We support accepting any mime file type since this is a simple download of a file
	ppszAcceptTypes[1] = NULL;
	ASSERT(hHttpFile == NULL);
	hHttpFile = HttpOpenRequest(hHttpConnection, NULL, strObject, NULL, NULL, ppszAcceptTypes, INTERNET_FLAG_RELOAD | 
		INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_KEEP_CONNECTION, NULL);
	if (hHttpFile == NULL)
	{
		TRACE(_T("Failed in call to HttpOpenRequest, Error:%d\n"), ::GetLastError());
		return dwHttpStatus;
	}

	//label used to jump to if we need to resend the request
resend:

	//////////////////////////////////////////////////////////////////////////
	//Issue the request
	BOOL bSend = ::HttpSendRequestEx(hHttpFile, NULL, NULL, 0, 0);
	if (!bSend){
		TRACE(_T("Failed in call to HttpSendRequestEx, Error:%d\n"), ::GetLastError());
		return dwHttpStatus;
	}
	bSend = HttpEndRequest(hHttpFile,NULL,0,0);
	if (!bSend){
		TRACE(_T("Failed in call to HttpEndRequest, Error:%d\n"), ::GetLastError());
		return dwHttpStatus;
	}
	//////////////////////////////////////////////////////////////////////////

	//Check the HTTP status code
	TCHAR szStatusCode[32];
	DWORD dwInfoSize = 32;
	if (!HttpQueryInfo(hHttpFile, HTTP_QUERY_STATUS_CODE, szStatusCode, &dwInfoSize, NULL))
	{
		TRACE(_T("Failed in call to HttpQueryInfo for HTTP query status code, Error:%d\n"), ::GetLastError());
		return dwHttpStatus;
	}
	else
	{
		long nStatusCode = _ttol(szStatusCode);

		//Handle any authentication errors
		if (nStatusCode == HTTP_STATUS_PROXY_AUTH_REQ || nStatusCode == HTTP_STATUS_DENIED)
		{
			// We have to read all outstanding data on the Internet handle
			// before we can resubmit request. Just discard the data.
			char szData[51];
			DWORD dwSize;
			do
			{
				::InternetReadFile(hHttpFile, (LPVOID)szData, 50, &dwSize);
			}
			while (dwSize != 0);
		}
		else if (nStatusCode != HTTP_STATUS_OK)
		{
//			TRACE(_T("Failed to retrieve a HTTP 200 status, Status Code:%d\n"), nStatusCode);
			return dwHttpStatus;
		}
	}

	// Get the length of the file.            
	TCHAR szContentLength[32];
	dwInfoSize = 32;
	DWORD dwFileSize = 0;
	BOOL bGotFileSize = FALSE;
	if (::HttpQueryInfo(hHttpFile, HTTP_QUERY_CONTENT_LENGTH, szContentLength, &dwInfoSize, NULL))
	{
		//Set the progress control range
		bGotFileSize = TRUE;
		dwFileSize = (DWORD) _ttol(szContentLength);
	}
	if ( (int)dwFileSize > 0 ) {
		DWORD dwBytesRead = 0;
		char*pBuf = strHtml.GetBuffer(dwFileSize);
		InternetReadFile(hHttpFile, pBuf, dwFileSize, &dwBytesRead);
		strHtml.ReleaseBuffer();
	}

	dwHttpStatus=0;

	//We're finished
	return dwHttpStatus;
}

HRESULT GetHttpFileContentEx(const CString&strUrl,CString&strHtml,int nTimeOutSeconds)
{
	HRESULT dwHttpStatus=0x80000000;
	strHtml.Empty();

	CString       strServer; 
	CString       strObject; 
	INTERNET_PORT nPort = INTERNET_DEFAULT_HTTP_PORT;
	DWORD         dwServiceType;
	HINTERNET hInternetSession=NULL;
	HINTERNET hHttpConnection=NULL;
	HINTERNET     hHttpFile=NULL;
	CString		strContentType;

	if (!AfxParseURL(strUrl, dwServiceType, strServer, strObject, nPort))
	{
		//Try sticking "http://" before it
		if (!AfxParseURL(_T("http://") + strUrl, dwServiceType, strServer, strObject, nPort))
		{
			TRACE(_T("Failed to parse the URL: %s\n"), strUrl);
			return dwHttpStatus;
		}
	}
	if ( nPort==INTERNET_DEFAULT_HTTPS_PORT/* || dwServiceType!=INTERNET_SERVICE_HTTP*/ ) {
		return E_INVALIDARG;	//https不支持，返回错误的参数
	}

	//Create the Internet session handle
	hInternetSession = ::InternetOpen(/*AfxGetAppName()+*/Star::Common::RandFloatNum(10), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (hInternetSession == NULL)
	{
//		TRACE(_T("Failed in call to InternetOpen, Error:%d\n"), ::GetLastError());
		return dwHttpStatus;
	}

	//Make the connection to the HTTP server
	hHttpConnection = ::InternetConnect(hInternetSession, strServer, nPort, NULL, 
		NULL, dwServiceType, 0, NULL);
	if (hHttpConnection == NULL)
	{
//		TRACE(_T("Failed in call to InternetConnect, Error:%d\n"), ::GetLastError());
		return dwHttpStatus;
	}

	nTimeOutSeconds*=1000;
	InternetSetOption(hHttpConnection,INTERNET_OPTION_RECEIVE_TIMEOUT,&nTimeOutSeconds,sizeof(nTimeOutSeconds));

	//Issue the request to read the file
	LPCTSTR ppszAcceptTypes[2];
	ppszAcceptTypes[0] = _T("*/*");  //We support accepting any mime file type since this is a simple download of a file
	ppszAcceptTypes[1] = NULL;
	hHttpFile = HttpOpenRequest(hHttpConnection, NULL, strObject, NULL, NULL, ppszAcceptTypes, INTERNET_FLAG_RELOAD | 
		INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_KEEP_CONNECTION, NULL);
	if (hHttpFile == NULL)
	{
//		TRACE(_T("Failed in call to HttpOpenRequest, Error:%d\n"), ::GetLastError());
		return dwHttpStatus;
	}

	//label used to jump to if we need to resend the request
resend:

	//////////////////////////////////////////////////////////////////////////
	//Issue the request
	BOOL bSend = ::HttpSendRequestEx(hHttpFile, NULL, NULL, 0, 0);
	if (!bSend){
//		TRACE(_T("Failed in call to HttpSendRequestEx, Error:%d\n"), ::GetLastError());
		return dwHttpStatus;
	}
	bSend = HttpEndRequest(hHttpFile,NULL,0,0);
	if (!bSend){
//		TRACE(_T("Failed in call to HttpEndRequest, Error:%d\n"), ::GetLastError());
		return dwHttpStatus;
	}
	//////////////////////////////////////////////////////////////////////////

	//Check the HTTP status code
	TCHAR szStatusCode[32];
	DWORD dwInfoSize = 32;
	if (!HttpQueryInfo(hHttpFile, HTTP_QUERY_STATUS_CODE, szStatusCode, &dwInfoSize, NULL))
	{
//		TRACE(_T("Failed in call to HttpQueryInfo for HTTP query status code, Error:%d\n"), ::GetLastError());
		return dwHttpStatus;
	}
	else
	{
		long nStatusCode = _ttol(szStatusCode);
		//toto...如果HTTP_QUERY_CONTENT_TYPE为utf-8进行转换，参考GetHttpFileContent
		//HttpQueryInfo(hHttpFile, HTTP_QUERY_CONTENT_TYPE, str, &dwInfoSize, NULL);

		//Handle any authentication errors
		if (nStatusCode == HTTP_STATUS_PROXY_AUTH_REQ || nStatusCode == HTTP_STATUS_DENIED)
		{
			// We have to read all outstanding data on the Internet handle
			// before we can resubmit request. Just discard the data.
			char szData[51]={0};
			DWORD dwSize;
			do
			{
				::InternetReadFile(hHttpFile, (LPVOID)szData, 50, &dwSize);
			}
			while (dwSize != 0);
		}
		else if (nStatusCode != HTTP_STATUS_OK)
		{
			//TRACE(_T("Failed to retrieve a HTTP 200 status, Status Code:%d\n"), nStatusCode);
			return dwHttpStatus;
		}
	}

	// Get the length of the file.            
	TCHAR szContentLength[32];
	dwInfoSize = 32;
	DWORD dwFileSize = 0;
	BOOL bGotFileSize = FALSE;
	if (::HttpQueryInfo(hHttpFile, HTTP_QUERY_CONTENT_LENGTH, szContentLength, &dwInfoSize, NULL))
	{
		//Set the progress control range
		bGotFileSize = TRUE;
		dwFileSize = (DWORD) _ttol(szContentLength);
	}

	DWORD dwStartTicks = ::GetTickCount();
	DWORD dwCurrentTicks = dwStartTicks;
	DWORD dwBytesRead = 0;
	DWORD dwBytesToRead = 1024;
	DWORD dwTotalLen=0;
	BOOL bRet = TRUE;
	DWORD dwMaxDataLength =1024*1024;
	char*pBuf=strHtml.GetBuffer(dwMaxDataLength);
	do
	{
		bRet = InternetReadFile(hHttpFile,pBuf,dwBytesToRead,&dwBytesRead);
		pBuf[dwBytesRead]=0;
		strHtml.ReleaseBuffer();
		if ( bRet==FALSE || dwBytesRead==0 ){
			break;
		}
		dwTotalLen=strHtml.GetLength();
		pBuf=strHtml.GetBuffer(dwTotalLen+dwMaxDataLength)+dwTotalLen;
	} while ( TRUE );

	//Delete the file being downloaded to if it is present and the download was aborted
	dwHttpStatus=0;

	//We're finished
	return dwHttpStatus;
}

#pragma warning(default:4102)


/*
HTTP/1.1 200 OK
Server: Tencent Login Server/2.0.0
Pragma: no-cache
Cache-Control: no-cache; must-revalidate
Expires: -1
Connection: Keep-Alive
Content-Type: application/x-javascript; charset=utf-8
Content-Length: 66



HTTP/1.1 200 OK
Connection: close
Server: QZHTTP-2.38.18
Date: Fri, 04 Dec 2015 13:28:22 GMT
Content-Type: application/x-javascript; charset=gbk
Content-Length: 7191
*/
HRESULT GetHttpFileContent(LPCTSTR lpszUrl,CString&strHtml,int nTimeOutSeconds) 
{
	const int BUFF_COUNT = 1024 * 1024 + 1;
	CString	strContentType;
	CString strContentLength;
	int		nRead=0;  
	CString strText;
	DWORD dwStatusCode = 0;
	HRESULT	hRes = 0x80000000;
	DWORD dwFileSize = 0;
	strHtml.Empty();

	CInternetSession session(Star::Common::RandFloatNum(10));
	session.SetOption(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, nTimeOutSeconds*1000);
	try{  
		CHttpFile *pHttpFile = (CHttpFile*)session.OpenURL(lpszUrl);  //使用默认的flag即可 否则肯会卡

		if ( pHttpFile!=NULL && pHttpFile->QueryInfoStatusCode(dwStatusCode)!=0 ){  
			if ( dwStatusCode>=200 && dwStatusCode<300 ){
				//Success
				hRes = S_OK;
				pHttpFile->QueryInfo(HTTP_QUERY_CONTENT_TYPE, strContentType);
				pHttpFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH, strContentLength);
				dwFileSize = (DWORD)_ttol(strContentLength);
				if ( (int)dwFileSize > 0 ) {
					pHttpFile->Read(strHtml.GetBuffer(dwFileSize), dwFileSize);
					strHtml.ReleaseBuffer();
				}else{
					char *buffer = new char[BUFF_COUNT];
					if ( buffer!=NULL ) {
						while( (nRead = pHttpFile->Read(buffer, BUFF_COUNT - 1)) > 0 ){
							buffer[nRead] = 0;
							strHtml += buffer;   
						}

						delete []buffer;
					}
				}
			}

			pHttpFile->Close();
			delete pHttpFile;

			if ( strContentType.Find("utf-8")!=-1 ) {
				strHtml = Star::Common::utf8s2ts(strHtml);
			}
		}  
	}catch(CInternetException*e){ 
		CString s;
		TCHAR szCause[MAX_PATH];
		e->GetErrorMessage(szCause,MAX_PATH);
		s.Format("InternetException：\n%s\n m_dwError%u,m_dwContextError%u",szCause,e->m_dwError,e->m_dwContext);
		//AfxMessageBox(s);
		//e->ReportError();
		e->Delete();  
		//可能是超时引起的
		hRes = 0x80000000;
	}

	return   hRes;  
}
//
////不采用改函数，不稳定，InternetReadFile随机返回： ERROR_INTERNET_CONNECTION_RESET导致页面读取的不全
//UINT GetHttpFileContent2(LPCTSTR lpszUrl,CString&strHtml)  
//{  
//	CString strText;
//	DWORD dwHttpStatus=0x80000000;
//	strHtml.Empty();
//	HINTERNET hInternet1=InternetOpen(NULL,PRE_CONFIG_INTERNET_ACCESS,NULL,INTERNET_INVALID_PORT_NUMBER,NULL);
//	if (NULL == hInternet1){
//		InternetCloseHandle(hInternet1);
//		return dwHttpStatus;
//	}
//	HINTERNET hInternet2=InternetOpenUrl(hInternet1,lpszUrl,NULL,NULL,
//		INTERNET_FLAG_TRANSFER_ASCII|INTERNET_FLAG_EXISTING_CONNECT|INTERNET_FLAG_RELOAD,1);
//	if (NULL == hInternet2){
//		InternetCloseHandle(hInternet2);
//		InternetCloseHandle(hInternet1);
//		return dwHttpStatus;
//	}
//	DWORD dwMaxDataLength =1024*1024*20;
//	//char*pBuf = new char[dwMaxDataLength];
//	// 	if (NULL == pBuf){
//	// 		InternetCloseHandle(hInternet2);
//	// 		InternetCloseHandle(hInternet1);
//	// 		return dwHttpStatus;
//	// 	}
//	DWORD dwReadDataLength = NULL;
//	DWORD dwTotalLen=0;
//	BOOL bRet = TRUE;
//	char*pBuf=strHtml.GetBuffer(dwMaxDataLength);
//	do
//	{
//		bRet = InternetReadFile(hInternet2,pBuf,dwMaxDataLength-1,&dwReadDataLength);
//		pBuf[dwReadDataLength]=0;
//		strHtml.ReleaseBuffer();
//		if ( bRet==FALSE || dwReadDataLength==0 ){
//			break;
//		}
//		dwTotalLen=strHtml.GetLength();
//		pBuf=strHtml.GetBuffer(dwTotalLen+dwMaxDataLength)+dwTotalLen;
//	} while ( TRUE );
//	//delete[]pBuf;
//	dwHttpStatus=0;
//
//	dwTotalLen=strHtml.GetLength();
//	return   dwHttpStatus;  
//}

BOOL URLDownloadToString(const CString&strUrl,CString&strHtml,int nTimeOutSeconds)
{
	CString strTempFile;
	BOOL bOK=FALSE;
	int nFileSize=0;

	strHtml.Empty();
	//strTempFile=Star::Common::GetMyTempPath();

	HRESULT hRet=URLDownloadToCacheFile(NULL,strUrl,strTempFile.GetBuffer(MAX_PATH),URLOSTRM_GETNEWESTVERSION,0,NULL);
	strTempFile.ReleaseBuffer();
	if ( hRet!=S_OK ){
		return bOK;  

		//strTempFile = Star::Common::GetSysTempFileName();
		//hRet=URLDownloadToFile(NULL,strUrl,strTempFile,0,NULL);
		//if ( hRet!=S_OK ){
		//	return bOK;  
		//}
	}

	CFile file;
	if ( file.Open(strTempFile,CFile::modeReadWrite) ){
		nFileSize=(int)file.GetLength();
		char*pBuf=strHtml.GetBuffer(nFileSize+1);
		file.Read(pBuf,nFileSize);
		pBuf[nFileSize]=0;
		strHtml.ReleaseBuffer();
		file.Close();
		::DeleteFile(strTempFile);

		bOK = TRUE;
	}

	return bOK;  
}

#include "stdafx.h"
#include <afxinet.h>
#include <afxwin.h>
#include "httppost.h"
#include "../gzipDecoder/gzipDecoder.h"


//ref: http://www.xuebuyuan.com/727742.html
//默认使用POST方式，默认需要解密
CString SendHttpData(const CString&strHost, const CString&strPath, const vector<CString>&vtHeaders, const CString&strSendData, CString&strNewCookie, 
					 BOOL bNeedDocode/* = TRUE*/, int nMethod/*=CHttpConnection::HTTP_VERB_POST*/)
{
	CInternetSession m_InetSession("session");
	CHttpConnection*pServer =NULL;
	CHttpFile*pFile =NULL;
	CString strResult;
	strNewCookie.Empty();
	int nSendDataSize = strSendData.GetLength();

	try{
		pServer =m_InetSession.GetHttpConnection(strHost);
		pFile =pServer->OpenRequest(nMethod, strPath, NULL, 1, NULL, NULL, INTERNET_FLAG_NO_COOKIES);
		for ( int i = 0; i < (int)vtHeaders.size(); ++i ) {
			pFile->AddRequestHeaders(vtHeaders[i]);
		}//endfor
		pFile->SendRequestEx(nSendDataSize);
		if ( nSendDataSize>0 ){
			pFile->WriteString(strSendData);
		}
		pFile->EndRequest();

		DWORD dwRet;
		pFile->QueryInfoStatusCode(dwRet);

		if ( dwRet==HTTP_STATUS_OK ){
			//int nLen=pFile->GetLength();//不可靠，返回的大小太小了
			char *buff=new char[1024*1024];
			int nReaded=0;
			int nTotalSize=0;
			while ( TRUE ){
				nReaded=pFile->Read(&buff[nTotalSize],1024);
				nTotalSize+=nReaded;
				if ( nReaded==0 ){
					break;
				}
			}
			buff[nTotalSize]=0;

			if ( bNeedDocode ){
				gzip_decoder gzip(nTotalSize*10);
				std::string str;
				gzip.ungzip((unsigned char *)buff,nTotalSize,str);
				strResult=str.c_str();
			}else{
				strResult = buff;
			}

			delete[] buff;

			//取返回的COOKIE
			pFile->QueryInfo(HTTP_QUERY_SET_COOKIE, strNewCookie, &dwRet) ;
		}
		delete pFile;
		delete pServer;

	}catch (CInternetException*e){
		CString s;
		TCHAR szCause[MAX_PATH];
		e->GetErrorMessage(szCause,MAX_PATH);
		s.Format("InternetException：\n%s\n m_dwError%u,m_dwContextError%u",szCause,e->m_dwError,e->m_dwContext);
		//AfxMessageBox(s);
		e->Delete();
		strResult.Empty();
	}

	return strResult;
}
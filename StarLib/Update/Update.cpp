#include "stdafx.h"
#include "Update.h"
#include <Shlwapi.h>
#include "../net/htmldown.h"
#include "../Common/common.h"
//#include "../../../lib/tinyxml/tinyxml.h"


//#ifdef _DEBUG
//#pragma comment(lib,"tinyxmld.lib")
//#else
//#pragma comment(lib,"tinyxml.lib")
//#endif


/*
下面的字符在 [XML]中被定义为 空白(whitespace)字符：
空格 (&#x0020;)
Tab (&#x0009;)
回车 (&#x000D;)
换行 (&#x000A;) 
*/
//BOOL CheckUpdateInfo(CString strAppName,CString strUrl,UPDATEINFO&stUpdateInfo)
//{
//	BOOL bSucceed=FALSE;
//	CString strHtml;
//	CString strText;
//	DWORD	dwHttpStatus=GetHttpFileContent(strUrl,strHtml);
//	if ( dwHttpStatus==0x80000000 ){
//		return bSucceed;
//	}
//	
//	TiXmlDocument myDocument;
//#ifdef _UNICODE
//	CStringA strHtmlA;
//	strHtmlA = strHtml;
//	myDocument.Parse(strHtmlA);
//#else
//	myDocument.Parse(strHtml);
//#endif
//	TiXmlElement* rootElement = myDocument.RootElement();  //data
//	TiXmlNode*pNodeTemp=NULL;
//	if ( rootElement!=NULL ){
//		TiXmlNode*pSoft=rootElement->FirstChild("soft");
//		while ( pSoft!=NULL ){
//			pNodeTemp=pSoft->FirstChild("name");
//			if ( pNodeTemp!=NULL ){
//				strText=pNodeTemp->ToElement()->GetText();
//				strText.Trim();
//				if ( strText.CompareNoCase(strAppName)==0 ){
//					stUpdateInfo.strName=strText;
//
//					pNodeTemp=pSoft->FirstChild("version");
//					if ( pNodeTemp!=NULL ){
//						stUpdateInfo.strVersion=pNodeTemp->ToElement()->GetText();
//						stUpdateInfo.strVersion.Trim();
//					}	
//					
//					pNodeTemp=pSoft->FirstChild("downurl");
//					if ( pNodeTemp!=NULL ){
//						stUpdateInfo.strDownUrl=pNodeTemp->ToElement()->GetText();
//						stUpdateInfo.strDownUrl.Trim();
//					}
//
//					pNodeTemp=pSoft->FirstChild("downurl2");
//					if ( pNodeTemp!=NULL ){
//						stUpdateInfo.strDownUrl2=pNodeTemp->ToElement()->GetText();
//						stUpdateInfo.strDownUrl2.Trim();
//					}
//
//					pNodeTemp=pSoft->FirstChild("regurl");
//					if ( pNodeTemp!=NULL ){
//						stUpdateInfo.strRegUrl=pNodeTemp->ToElement()->GetText();
//					}
//
//					pNodeTemp=pSoft->FirstChild("help");
//					if ( pNodeTemp!=NULL ){
//						stUpdateInfo.strHelpUrl=pNodeTemp->ToElement()->GetText();
//					}
//
//					pNodeTemp=pSoft->FirstChild("bbs");
//					if ( pNodeTemp!=NULL ){
//						stUpdateInfo.strBBS=pNodeTemp->ToElement()->GetText();
//					}
//
//					pNodeTemp=pSoft->FirstChild("news");
//					if ( pNodeTemp!=NULL ){
//						stUpdateInfo.strNews=pNodeTemp->ToElement()->GetText();
//					}	
//
//					pNodeTemp=pSoft->FirstChild("urgent");
//					if ( pNodeTemp!=NULL ){
//						stUpdateInfo.strUrgent=pNodeTemp->ToElement()->GetText();
//					}	
//
//					bSucceed=TRUE;
//					break;
//				}
//			}	
//
//			pSoft=pSoft->NextSibling("soft");
//		}//end while
//	}
//
//	return bSucceed;
//}

void GetScriptInfo(CString&strHtml,UPDATEINFO&stUpdateInfo)
{
	int nPos1 = 0;
	int nPos2 = 0;
	SCRIPTINFO stScript;

	nPos1 = strHtml.Find("path=\"");
	if ( nPos1!=-1 ) {
		nPos2 = strHtml.Find('\"',nPos1+6);
		stUpdateInfo.strScriptPath = strHtml.Mid(nPos1+6,nPos2-nPos1-6).Trim();
		Star::Common::PathWithSlash(stUpdateInfo.strScriptPath);
	}else{
		stUpdateInfo.strScriptPath = _T(".\\");
	}

	nPos1 = strHtml.Find("root=\"", nPos2);
	if ( nPos1!=-1 ) {
		nPos2 = strHtml.Find('\"', nPos1+6);
		stUpdateInfo.strScriptRoot = strHtml.Mid(nPos1+6, nPos2-nPos1-6).Trim();
	}
	nPos1 = strHtml.Find("root2=\"", nPos2);
	if ( nPos1!=-1 ) {
		nPos2 = strHtml.Find('\"', nPos1+7);
		stUpdateInfo.strScriptRoot2 = strHtml.Mid(nPos1+7, nPos2-nPos1-7).Trim();
	}

	nPos2 = 0;
	while ( TRUE ){
		nPos1 = strHtml.Find(_T("<script"),nPos2);
		if ( nPos1==-1 ) {
			break;
		}

		nPos1 = strHtml.Find(_T("name=\""),nPos1);
		if ( nPos1==-1 ) {
			break;
		}
		nPos2 = strHtml.Find('\"',nPos1+6);
		stScript.strScriptName = strHtml.Mid(nPos1+6,nPos2-nPos1-6);
		stScript.strLocalFileName = stScript.strScriptName + ".txt";

		nPos1 = strHtml.Find(_T("desc=\""),nPos1);
		if ( nPos1!=-1 ) {
			nPos2 = strHtml.Find('\"',nPos1+6);
			stScript.strScriptDesc = strHtml.Mid(nPos1+6,nPos2-nPos1-6);
		}else{
			nPos1 = nPos2;
			stScript.strScriptDesc = stScript.strScriptName;
		}

		nPos1 = strHtml.Find(_T("ver=\""),nPos1);
		nPos2 = strHtml.Find('\"',nPos1+5);
		stScript.strScriptVer = strHtml.Mid(nPos1+5,nPos2-nPos1-5);

		stScript.strScriptUrl = stUpdateInfo.strScriptRoot + stScript.strLocalFileName;
		stScript.strScriptUrl2 = stUpdateInfo.strScriptRoot2 + stScript.strLocalFileName;

		stUpdateInfo.vtScripts.push_back(stScript);
	}
}

BOOL CheckUpdateInfoIni(const CString&strUrl,UPDATEINFO&stUpdateInfo)
{
	BOOL bSucceed=FALSE;
	CString strHtml;
	CString strText;
	int nPos1=0;
	int nPos2=0;
	int nEnd=0;

	stUpdateInfo.vtScripts.clear();
	HRESULT hRes = GetHttpFileContentNoCache(strUrl, strHtml);
	if ( /*hRes == E_INVALIDARG*/FAILED(hRes) ) {
		//maybe https
		hRes = GetHttpFileContent(strUrl, strHtml);
		if ( FAILED(hRes) ) {
			if ( URLDownloadToString(strUrl, strHtml)==FALSE ) {
				return bSucceed;
			}
		}
	}
	if ( strHtml.IsEmpty()==TRUE ) {
		return FALSE;
	}

	//////////////////////////////////////////////////////////////////////////
	strText.Empty();
	nPos1=strHtml.Find(_T("<planb>"));
	if ( nPos1!=-1 ){
		nPos2=strHtml.Find(_T("<"), nPos1+7);
		if ( nPos2!=-1 ){
			strText = strHtml.Mid(nPos1+7, nPos2-nPos1-7);
			stUpdateInfo.bUsePlanB = (StrToInt(strText) != 0);
		}
	}
	nPos1=strHtml.Find(_T("<name>"));
	if ( nPos1!=-1 ){
		nPos2=strHtml.Find(_T("</"),nPos1+6);
		if ( nPos2!=-1 ){
			stUpdateInfo.strName=strHtml.Mid(nPos1+6,nPos2-nPos1-6);
		}
	}
	//////////////////////////////////////////////////////////////////////////
	nPos1=strHtml.Find(_T("<version>"));
	if ( nPos1!=-1 ){
		nPos2=strHtml.Find(_T("</"),nPos1+9);
		if ( nPos2!=-1 ){
			stUpdateInfo.strVersion=strHtml.Mid(nPos1+9,nPos2-nPos1-9);
		}
	}
	//////////////////////////////////////////////////////////////////////////
	nPos1=strHtml.Find(_T("<downurl>"));
	if ( nPos1!=-1 ){
		nPos2=strHtml.Find(_T("</"),nPos1+9);
		if ( nPos2!=-1 ){
			stUpdateInfo.strDownUrl=strHtml.Mid(nPos1+9,nPos2-nPos1-9);
		}
	}
	//////////////////////////////////////////////////////////////////////////
	nPos1=strHtml.Find(_T("<downurl2>"));
	if ( nPos1!=-1 ){
		nPos2=strHtml.Find(_T("</"),nPos1+10);
		if ( nPos2!=-1 ){
			stUpdateInfo.strDownUrl2=strHtml.Mid(nPos1+10,nPos2-nPos1-10);
		}
	}
	//////////////////////////////////////////////////////////////////////////
	nPos1=strHtml.Find(_T("<regurl>"));
	if ( nPos1!=-1 ){
		nPos2=strHtml.Find(_T("</"),nPos1+8);
		if ( nPos2!=-1 ){
			stUpdateInfo.strRegUrl=strHtml.Mid(nPos1+8,nPos2-nPos1-8);
		}
	}
	//////////////////////////////////////////////////////////////////////////
	nPos1=strHtml.Find(_T("<help>"));
	if ( nPos1!=-1 ){
		nPos2=strHtml.Find(_T("</"),nPos1+6);
		if ( nPos2!=-1 ){
			stUpdateInfo.strHelpUrl=strHtml.Mid(nPos1+6,nPos2-nPos1-6);
		}
	}
	//////////////////////////////////////////////////////////////////////////
	nPos1=strHtml.Find(_T("<bbs>"));
	if ( nPos1!=-1 ){
		nPos2=strHtml.Find(_T("</"),nPos1+5);
		if ( nPos2!=-1 ){
			stUpdateInfo.strBBS=strHtml.Mid(nPos1+5,nPos2-nPos1-5);
			stUpdateInfo.strHome=stUpdateInfo.strBBS;
		}
	}
	//////////////////////////////////////////////////////////////////////////
	nPos1=strHtml.Find(_T("<news>"));
	if ( nPos1!=-1 ){
		nPos2=strHtml.Find(_T("</"),nPos1+6);
		if ( nPos2!=-1 ){
			stUpdateInfo.strNews=strHtml.Mid(nPos1+6,nPos2-nPos1-6);
		}
	}
	//////////////////////////////////////////////////////////////////////////
	nPos1=strHtml.Find(_T("<ad>"));
	if ( nPos1!=-1 ){
		nPos2=strHtml.Find(_T("</"),nPos1+4);
		if ( nPos2!=-1 ){
			stUpdateInfo.strAdsUrl=strHtml.Mid(nPos1+4,nPos2-nPos1-4);
		}
	}
	//////////////////////////////////////////////////////////////////////////
	nPos1=strHtml.Find(_T("<mailme>"));
	if ( nPos1!=-1 ){
		nPos2=strHtml.Find(_T("</"),nPos1+8);
		if ( nPos2!=-1 ){
			stUpdateInfo.strMailMeUrl=strHtml.Mid(nPos1+8,nPos2-nPos1-8);
		}
	}
	//////////////////////////////////////////////////////////////////////////
	nPos1 = strHtml.Find(_T("<scripts"));
	if ( nPos1!=-1 ) {
		nPos2 = strHtml.Find(_T("</scripts"),nPos1);
		if ( nPos2!=-1 ) {
			GetScriptInfo(strHtml.Mid(nPos1,nPos2-nPos1),stUpdateInfo);
		}
	}
	//////////////////////////////////////////////////////////////////////////

	bSucceed=TRUE;
	return bSucceed;
}

void StartNewVersionApp(CString strNewApp)
{
	TCHAR szExePath[MAX_PATH];
	GetModuleFileName(NULL,szExePath,MAX_PATH);

	CString strCmdLine;
	strCmdLine.Format(_T("\"%s\" delete \"%s\""),strNewApp,szExePath);

	STARTUPINFO si={};
	PROCESS_INFORMATION pi={};
	si.cb=sizeof(si);
	CreateProcess(NULL,(LPTSTR)(LPCTSTR)strCmdLine,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
	::ExitProcess(0);
}
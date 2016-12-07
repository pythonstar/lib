// commonres.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "commonres.h"
#include "HttpDownloadDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BOOL ShowDownloadFileDlg(const CString&strURLToDownload, const CString&strFileToSave)
{
	if ( GetFileAttributes(strFileToSave)!=-1 ) {
		DeleteFile(strFileToSave);
	}

	CHttpDownloadDlg dlg;
	dlg.m_sURLToDownload = strURLToDownload;
	dlg.m_sFileToDownloadInto = strFileToSave;
	dlg.DoModal();

	return (GetFileAttributes(strFileToSave)!=-1);
}


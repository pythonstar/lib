
#pragma once

#include <afx.h>
#include <tchar.h>
#include <afxwin.h>
#include "shlobj.h"

class CPathDialog;

// CPathDialogSub - intercepts messages from child controls
class CPathDialogSub : public CWnd
{
 friend CPathDialog;
public:
 CPathDialog* m_pPathDialog;
protected:
    afx_msg void OnOK();              // OK button clicked
 afx_msg void OnChangeEditPath();
    DECLARE_MESSAGE_MAP()
private:
};

/////////////////////////////////////////////////////////////////////////////
// CPathDialog dialog

class CPathDialog
{
 friend CPathDialogSub;
// Construction
public:
 CPathDialog(LPCTSTR lpszCaption=NULL,
  LPCTSTR lpszTitle=NULL,
  LPCTSTR lpszInitialPath=NULL, 
  CWnd* pParent = NULL);

 CString GetPathName();
 virtual int DoModal();

 static int Touch(LPCTSTR lpPath, BOOL bValidate=TRUE);
 static int MakeSurePathExists(LPCTSTR lpPath);
 static BOOL IsFileNameValid(LPCTSTR lpFileName);

private:
 static int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg,LPARAM lParam, LPARAM pData);

 LPCTSTR m_lpszCaption;
 LPCTSTR m_lpszInitialPath;

 TCHAR m_szPathName[MAX_PATH];

 BROWSEINFO m_bi;
 HWND m_hWnd;
 CWnd* m_pParentWnd;
 BOOL m_bParentDisabled;
 BOOL m_bGetSuccess;

 CPathDialogSub m_PathDialogSub;

};
							 /////////////////////////////////////////////////////////////////////////////
// CustomizeDialog.cpp: implementation of the CCustomizeDialog class.
//
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 by Nikolay Denisov. All rights reserved.
//
// This code is free for personal and commercial use, providing this 
// notice remains intact in the source files and all eventual changes are
// clearly marked with comments.
//
// You must obtain the author's consent before you can include this code
// in a software library.
//
// No warrantee of any kind, express or implied, is included with this
// software; use at your own risk, responsibility for damages (if any) to
// anyone resulting from the use of this software rests entirely with the
// user.
//
// Please email bug reports, bug fixes, enhancements, requests and
// comments to: nick@actor.ru
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CustomizeDialog.h"
#include "ToolBarEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
/////////////////////////////////////////////////////////////////////////////
// CCustomizeDialog dialog



static const int cxPadding = 3;
static const int cyPadding = 3;

IMPLEMENT_DYNAMIC( CCustomizeDialog, CWnd )

CCustomizeDialog::CCustomizeDialog( CToolBarEx* pToolBar,
                                    TextOptions TextOptions,
                                    IconOptions IconOptions )
{
	ASSERT_VALID(pToolBar);
    m_pToolBar = pToolBar;

	m_eTextOptions=TextOptions;
	m_eIconOptions=IconOptions;

}

/////////////////////////////////////////////////////////////////////////////
// Operations

void CCustomizeDialog::SetTextOptions( TextOptions eTextOptions)
{
    m_eTextOptions = eTextOptions;
	if (IsWindow(m_wndTextOptions.m_hWnd))
		  m_wndTextOptions.SetCurSel(eTextOptions);
    
}

void CCustomizeDialog::SetIconOptions( IconOptions eIconOptions)
{
    m_eIconOptions = eIconOptions;

	if (IsWindow(m_wndIconOptions.m_hWnd))
	{
		  m_wndIconOptions.SetCurSel(eIconOptions);	  
	}
}


/////////////////////////////////////////////////////////////////////////////
// CCustomizeDialog message handlers

BEGIN_MESSAGE_MAP(CCustomizeDialog, CWnd)
    //{{AFX_MSG_MAP(CCustomizeDialog)
	//}}AFX_MSG_MAP
    ON_MESSAGE( WM_INITDIALOG, OnInitDialog )
    ON_CBN_SELENDOK(IDC_CB_TEXTOPTIONS, OnTextOptions)
    ON_CBN_SELENDOK(IDC_CB_ICONOPTIONS, OnIconOptions)
END_MESSAGE_MAP()

LRESULT CCustomizeDialog::OnInitDialog( WPARAM /*wParam*/, LPARAM /*lParam*/ )
{
    CRect rcClient;
    GetClientRect( rcClient );
    CRect rcWindow;
    GetWindowRect( rcWindow );

	// create  the windows	// sizes are hardcoded //ugly way


	CRect  rc;
	CFont *pFont =GetFont();
	ASSERT_VALID(pFont);

	
  
	//static wnd
	rc=CRect(4,2,49,8);
	MapDialogRect(m_hWnd,&rc);
	rc.left+=rcClient.left;
	rc.top+=rcClient.bottom;
	VERIFY(m_wndTextOptionsText.Create(_T("Te&xt options:"),WS_CHILD|WS_VISIBLE,rc,this));
 	m_wndTextOptionsText.SetFont(pFont);
	m_wndTextOptionsText.SetWindowPos(NULL,rc.left,rc.top,rc.right,rc.bottom,SWP_NOZORDER|SWP_NOACTIVATE);

	
	//Text options
	rc=CRect(52,0,123,50);
	MapDialogRect(m_hWnd,&rc);
	rc.left+=rcClient.left;
	rc.top+=rcClient.bottom;
	VERIFY(m_wndTextOptions.Create(WS_CHILD |WS_VISIBLE |CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,rc,this, IDC_CB_TEXTOPTIONS));
 	m_wndTextOptions.SetFont(pFont);

	m_wndTextOptions.AddString(_T("Show Text Labels"));
	m_wndTextOptions.AddString(_T("Selective Text on Right"));
	m_wndTextOptions.AddString(_T("No Text Labels"));

	SetTextOptions(m_eTextOptions);
	m_wndTextOptions.SetWindowPos(NULL,rc.left,rc.top,rc.right,rc.bottom,SWP_NOZORDER|SWP_NOACTIVATE);

 
	//static wnd
	rc=CRect(4,20,49,8);
	MapDialogRect(m_hWnd,&rc);
	rc.left+=rcClient.left;
	rc.top+=rcClient.bottom;
	VERIFY(m_wndIconOptionsText.Create(_T("Ico&n options:"),WS_CHILD|WS_VISIBLE,rc,this));
	m_wndIconOptionsText.SetFont(pFont);
	m_wndIconOptionsText.SetWindowPos(NULL,rc.left,rc.top,rc.right,rc.bottom,SWP_NOZORDER|SWP_NOACTIVATE);


	//icon combo
	rc=CRect(52,18,123,50);
	MapDialogRect(m_hWnd,&rc);
	rc.left+=rcClient.left;
	rc.top+=rcClient.bottom;
	VERIFY(m_wndIconOptions.Create(WS_CHILD |WS_VISIBLE |CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,rc,this, IDC_CB_ICONOPTIONS));
 	m_wndIconOptions.SetFont(pFont);

	m_wndIconOptions.AddString(_T("Small Icons"));
	m_wndIconOptions.AddString(_T("Large Icons"));
	
	SetIconOptions(m_eIconOptions);
	m_wndIconOptions.SetWindowPos(NULL,rc.left,rc.top,rc.right,rc.bottom,SWP_NOZORDER|SWP_NOACTIVATE);



	// final rect
	rc=CRect(0,0,179,34);
	MapDialogRect(m_hWnd,&rc);
	rc.left+=rcClient.left;
	rc.top+=rcClient.bottom;
	
 
    // Increase height of Customize Toolbar dialog accordingly
    rcWindow.bottom += (rc.bottom+cyPadding);
    VERIFY( SetWindowPos( 0, 0, 0, rcWindow.Width(), rcWindow.Height(),
        SWP_NOZORDER | SWP_NOMOVE ) );

	//Set Title
	ASSERT(m_pToolBar);
	CString strName,strOrgName;
	m_pToolBar->GetWindowText(strName);

	if (!strName.IsEmpty())
	{
		GetWindowText(strOrgName);
		SetWindowText(strOrgName + _T(" - ")+ strName);
	}

    return Default();

}




void CCustomizeDialog::OnTextOptions()
{
	m_eTextOptions=(TextOptions)m_wndTextOptions.GetCurSel();
	m_pToolBar->SetTextOptions(( TextOptions )m_eTextOptions,TRUE);
}

void CCustomizeDialog::OnIconOptions()
{
	m_eIconOptions=(IconOptions)m_wndIconOptions.GetCurSel();
}




LRESULT CCustomizeDialog::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	if (message==WM_HELP)
	{
	  LPHELPINFO lphi = (LPHELPINFO) lParam; 
	  if ( (lphi->iCtrlId != IDC_CB_TEXTOPTIONS) && (lphi->iCtrlId != IDC_CB_ICONOPTIONS) )
		return DefWindowProc(message,wParam,lParam);
	}
	return CWnd::WindowProc(message, wParam, lParam);
}




// ToolBarEx.cpp : implementation file
//

#include "stdafx.h"
#include "ToolBarEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CToolBarEx

IMPLEMENT_DYNCREATE( CToolBarEx,CToolBar);

//Image sizes
const CSize CToolBarEx::m_szImageSmall( 16, 15 );
const CSize CToolBarEx::m_szImageLarge( 24, 24 );

const TextOptions CToolBarEx:: m_eInitialTextOptions =  toNoTextLabels;
const IconOptions CToolBarEx:: m_eInitialIconOptions =  ioSmallIcons;
								  


int CToolBarEx::m_nBarNumber = 0;
CCustomizeDialog*   CToolBarEx::m_pCustomizeDlg = NULL;
HHOOK               CToolBarEx::m_hCBTHook      = NULL;


#define STR_CUSTOMIZE  _T("&Customize...")


// for determining version of COMCTL32.DLL
#define VERSION_WIN4        MAKELONG(0, 4)
#define VERSION_IE3         MAKELONG(70, 4)
#define VERSION_IE4         MAKELONG(71, 4)
#define VERSION_IE401       MAKELONG(72, 4)
#define VERSION_IE5			MAKELONG(80, 5)
#define VERSION_IE5_2000    MAKELONG(81, 5)

struct AFX_DLLVERSIONINFO
{
	DWORD cbSize;
	DWORD dwMajorVersion;                   // Major version
	DWORD dwMinorVersion;                   // Minor version
	DWORD dwBuildNumber;                    // Build number
	DWORD dwPlatformID;                     // DLLVER_PLATFORM_*
};

typedef HRESULT (CALLBACK* AFX_DLLGETVERSIONPROC)(AFX_DLLVERSIONINFO *);

static int _ComCtlVersion = -1;

static DWORD AFXAPI _GetComCtlVersion()
{
	// return cached version if already determined...
	if (_ComCtlVersion != -1)
		return _ComCtlVersion;
	
	// otherwise determine comctl32.dll version via DllGetVersion
	HINSTANCE hInst = ::GetModuleHandleA("COMCTL32.DLL");
	ASSERT(hInst != NULL);
	AFX_DLLGETVERSIONPROC pfn;
	pfn = (AFX_DLLGETVERSIONPROC)GetProcAddress(hInst, "DllGetVersion");
	DWORD dwVersion = VERSION_WIN4;
	if (pfn != NULL)
	{
		AFX_DLLVERSIONINFO dvi;
		memset(&dvi, 0, sizeof(dvi));
		dvi.cbSize = sizeof(dvi);
		HRESULT hr = (*pfn)(&dvi);
		if (SUCCEEDED(hr))
		{
			ASSERT(dvi.dwMajorVersion <= 0xFFFF);
			ASSERT(dvi.dwMinorVersion <= 0xFFFF);
			dwVersion = MAKELONG(dvi.dwMinorVersion, dvi.dwMajorVersion);
		}
		
		TRACE(_T("Using CommCtl32.dll Ver:%d.%d.%d\n"),dvi.dwMajorVersion,dvi.dwMinorVersion,dvi.dwBuildNumber);

	}
	_ComCtlVersion = dwVersion;
	return dwVersion;
}


CToolBarEx::CToolBarEx()
{
	m_pControls		 = NULL;
	m_pDropButtons	 = NULL;		// list of drop-down buttons
//	m_bShowDropdownArrowWhenVertical = FALSE;
	m_bHideChildWndOnVertical=TRUE;

	m_clrBtnHilight=::GetSysColor(COLOR_BTNHILIGHT);
	m_clrBtnShadow=::GetSysColor(COLOR_BTNSHADOW);
	m_clrBtnFace  = ::GetSysColor(COLOR_BTNFACE);

	++m_nBarNumber; //Increase the Bar number

	CWinApp  *pApp = AfxGetApp();
	ASSERT_VALID(pApp);
	m_strValueName.Format( _T("ToolBarEx%d"), m_nBarNumber );
	m_strSubKey.Format( _T("Software\\%s\\%s\\Settings"),
		pApp->m_pszRegistryKey, pApp->m_pszProfileName );

    m_eTextOptions = toNone;    // no options selected yet
    m_eIconOptions = ioNone;    // no options selected yet

	m_nResButtons=0;

}

CToolBarEx::~CToolBarEx()
{
	while (m_pDropButtons)
	{
		CDropDownButtonInfo* pnext = m_pDropButtons->pNext;
		delete m_pDropButtons;
		m_pDropButtons = pnext;
	}

	if( m_pControls ) 
	{
		for( POSITION pos = m_pControls->GetHeadPosition() ; pos ; ) 
		{
			delete m_pControls->GetNext(pos);
		}

		delete m_pControls;
	}

}


BEGIN_MESSAGE_MAP(CToolBarEx, CToolBar)
	//{{AFX_MSG_MAP(CToolBarEx)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_CONTEXTMENU()
	ON_WM_NCPAINT()
	//}}AFX_MSG_MAP
	ON_NOTIFY_REFLECT_EX(TBN_DROPDOWN,	 OnToolBarBtnDropDown)
	ON_NOTIFY_REFLECT(TBN_BEGINADJUST,	 OnToolBarBeginAdjust)
	ON_NOTIFY_REFLECT(TBN_CUSTHELP,		 OnToolBarCustomHelp)
	ON_NOTIFY_REFLECT(TBN_ENDADJUST,	 OnToolBarEndAdjust)
	ON_NOTIFY_REFLECT(TBN_GETBUTTONINFO, OnToolBarGetButtonInfo)
	ON_NOTIFY_REFLECT(TBN_QUERYDELETE,	 OnToolBarQueryDelete)
	ON_NOTIFY_REFLECT(TBN_QUERYINSERT,	 OnToolBarQueryInsert)
	ON_NOTIFY_REFLECT(TBN_RESET,		 OnToolBarReset)
	ON_NOTIFY_REFLECT(TBN_TOOLBARCHANGE, OnToolBarChange)
	ON_MESSAGE(TB_CUSTOMIZE, OnCustomize)
    ON_NOTIFY_REFLECT( TBN_INITCUSTOMIZE, OnInitCustomize )
 //   ON_NOTIFY_REFLECT( NM_CUSTOMDRAW , OnCustomDraw)

    // Saving and restoring toolbar
    ON_NOTIFY_REFLECT( TBN_SAVE, OnSave )
    ON_NOTIFY_REFLECT( TBN_RESTORE, OnRestore )
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CToolBarEx message handlers

//////////////////////////////////////////////////////////////////////
// 1999 Kirk Stowell - Inserts a control into the toolbar at the given button id.
//
CWnd* CToolBarEx::InsertControl( CRuntimeClass* pClass, LPCTSTR lpszWindowName, CRect& rect, UINT nID, DWORD dwStyle )
{
	CWnd *pCtrl = NULL;
	
	if( pClass->IsDerivedFrom( RUNTIME_CLASS( CComboBox )))	// CComboBox control.
	{
		pCtrl = new CComboBox;

		ASSERT_VALID( pCtrl );

		if(((CComboBox*)pCtrl)->Create( WS_CHILD|WS_VISIBLE|dwStyle, rect, this, nID ) == FALSE )
		{
			delete pCtrl;
			return NULL;
		}
	}
	else if( pClass->IsDerivedFrom( RUNTIME_CLASS( CEdit )))		// CEdit control.
	{
		pCtrl = new CEdit;
							
		ASSERT_VALID( pCtrl );

		if(((CEdit*)pCtrl)->Create( WS_CHILD|WS_VISIBLE|dwStyle, rect, this, nID ) == FALSE )
		{
			delete pCtrl;
			return NULL;
		}
	}
	else if( pClass->IsDerivedFrom( RUNTIME_CLASS( CButton )))		// CButton control.
	{
		pCtrl = new CButton;

		ASSERT_VALID( pCtrl );

		if(((CButton*)pCtrl)->Create( lpszWindowName, WS_CHILD|WS_VISIBLE|dwStyle, rect, this, nID ) == FALSE )
		{
			delete pCtrl;
			return NULL;
		}
	}
	else if( pClass->IsDerivedFrom( RUNTIME_CLASS( CWnd )))			// CWnd object.
	{
		pCtrl = new CWnd;
		
		ASSERT_VALID( pCtrl );

#ifdef _UNICODE
		TCHAR szClassName[ 256 ];

		MultiByteToWideChar( CP_ACP,
							 MB_PRECOMPOSED,
							 pClass->m_lpszClassName,
							 -1,
							 szClassName,
							 255 );

		if(((CWnd*)pCtrl)->Create( szClassName, lpszWindowName, WS_CHILD|WS_VISIBLE|dwStyle, rect, this, nID ) == FALSE )
		{
			delete pCtrl;
			return NULL;
		}
#else
		if(((CWnd*)pCtrl)->Create( pClass->m_lpszClassName, lpszWindowName, WS_CHILD|WS_VISIBLE|dwStyle, rect, this, nID ) == FALSE )
		{
			delete pCtrl;
			return NULL;
		}
#endif
	}
	else	// An invalid object was passed in
	{
		ASSERT( FALSE );
		return NULL;
	}

	// if our object list has not been allocated, do it now...
	if( m_pControls == NULL )
	{
		m_pControls = new CObList();
		ASSERT( m_pControls );
	}

	// we have to remember this control, so we can delete it later
	m_pControls->AddTail( pCtrl );

	return InsertControl( pCtrl, rect, nID );
}

CWnd* CToolBarEx::InsertControl(CWnd* pCtrl, CRect & rect, UINT nID)
{
	ASSERT_VALID( pCtrl );
	ASSERT(IsWindow(pCtrl->m_hWnd));

	// make sure the id is valid, and set the button 
	// style for a seperator.
	int nIndex = CommandToIndex( nID ) ;
	if (nIndex>-1)
	{
		ASSERT( nIndex >= 0 );
		SetButtonInfo( nIndex, nID, TBBS_SEPARATOR, rect.Width());
		// insert the control into the toolbar.
		GetItemRect( nIndex, &rect );
		CRect rt;
		pCtrl->GetWindowRect(&rt);
		rect.top+=max((rect.Height()-rt.Height())/2,0);	 //move to middle
		pCtrl->SetWindowPos(0, rect.left, rect.top, 0, 0,
			SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOCOPYBITS );
		pCtrl->SetFont( GetFont( ));

		BOOL bVert =  IsVertDocked(); //(m_dwStyle & CBRS_ORIENT_VERT) != 0;

		if (bVert && m_bHideChildWndOnVertical)
		{
		   	int nState=GetToolBarCtrl().GetState(nIndex);
		   	GetToolBarCtrl().SetState(nID,(nState | TBSTATE_HIDDEN));
			pCtrl->ShowWindow( SW_HIDE );
		}
		else
		{
		   	int nState=GetToolBarCtrl().GetState(nIndex);
		   	GetToolBarCtrl().SetState(nIndex,(nState & ~TBSTATE_HIDDEN));
			pCtrl->ShowWindow( SW_SHOW );
		}

	}
	else
	{
		pCtrl->ShowWindow( SW_HIDE);
	}

	ModifyStyle(0,WS_CLIPCHILDREN);
	return pCtrl;
}

BOOL CToolBarEx::AddDropDownButton(UINT nIDButton, UINT nIDMenu,BOOL bArrow)
{
	ASSERT_VALID(this);
	
	CDropDownButtonInfo* pb = FindDropDownButtonInfo(nIDButton);

	if (!pb) 
	{
		pb = new CDropDownButtonInfo;
		ASSERT(pb);
		pb->pNext = m_pDropButtons;
		m_pDropButtons = pb;
	}

	pb->idButton = nIDButton;
	pb->idMenu   = nIDMenu;
	
   return SetDropDownButton(nIDButton,bArrow);
}

CToolBarEx::CDropDownButtonInfo* CToolBarEx::FindDropDownButtonInfo(UINT nID)
{
	for (CDropDownButtonInfo* pb = m_pDropButtons; pb; pb = pb->pNext) 
	{
		if (pb->idButton == nID)
			return pb;
	}
	return NULL;
}

BOOL CToolBarEx::OnToolBarBtnDropDown(NMHDR* pNMHDR, LRESULT* pRes)
{
	UNUSED_ALWAYS( pRes );
	
	const NMTOOLBAR& nmtb = *(NMTOOLBAR*)pNMHDR;
														  
	// get location of button
	CRect rc;
	GetToolBarCtrl().GetRect(nmtb.iItem, rc);
	ClientToScreen(&rc);
	
	// call virtual function to display dropdown menu
	return OnDropDownButtonInfo(nmtb, nmtb.iItem, rc);
}

BOOL CToolBarEx::OnDropDownButtonInfo(const NMTOOLBAR& nmtb, UINT nID, CRect rc)
{
	UNUSED_ALWAYS( nID );

	CDropDownButtonInfo* pb = FindDropDownButtonInfo(nmtb.iItem);
	if (pb && pb->idMenu) 
	{
		
		// load and display popup menu
		CMenu menu;
		VERIFY(menu.LoadMenu(pb->idMenu));
		CMenu* pPopup = (CMenu*)menu.GetSubMenu(0);
		ASSERT(pPopup);
		pPopup->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_VERTICAL,
			rc.left, rc.bottom, AfxGetMainWnd(), &rc);
		return TRUE;
	}

	return FALSE;
}


// This function saves the state (visible buttons, toolbar position, etc.)
// of the toolbar, using the registry key provided to the Create(...) function.
void CToolBarEx::SaveState()
{
	// if there is an associated registry subkey
	if (m_strSubKey.GetLength())
	{
		// save the toolbar state to the registry
		GetToolBarCtrl().SaveState( HKEY_CURRENT_USER, m_strSubKey, m_strValueName );
	}
}

// This function restores the state (visible buttons, toolbar position, etc.)
// of the toolbar, using the registry key provided to the Create(...) function.
void CToolBarEx::RestoreState()
{
	IconOptions eIconOptions=GetIconOptions();
	TextOptions eTextOptions=GetTextOptions();

	// if there is an associated registry subkey
	if (m_strSubKey.GetLength())
	{
		// restore the toolbar state from the registry
		GetToolBarCtrl().RestoreState( HKEY_CURRENT_USER, m_strSubKey, m_strValueName );
	}

	//Set Icon/Text options if changed
	if (eTextOptions!=GetTextOptions()) SetTextOptions(GetTextOptions(),TRUE);
	if (eIconOptions!=GetIconOptions()) SetIconOptions(GetIconOptions(),TRUE);

}

// This function is called when the user begins dragging a toolbar
// button or when the customization dialog is being populated with
// toolbar information.  Basically, *pResult should be populated with
// your answer to the question, "is the user allowed to delete this
// button?".
void CToolBarEx::OnToolBarQueryDelete(NMHDR *pNMHDR, LRESULT *pResult)
{
	UNUSED_ALWAYS( pNMHDR );

	NMTOOLBAR * tbStruct=(TBNOTIFY *)pNMHDR;


	ASSERT(tbStruct);
	// do not allow hidden button to be deleted as they just do not go
	// to the Add listbox.
	if ((tbStruct->tbButton.idCommand) && 
		GetToolBarCtrl().IsButtonHidden(tbStruct->tbButton.idCommand))
			*pResult = FALSE;
	else					 
			*pResult = TRUE;  

}

// This function is called when the user begins dragging a toolbar
// button or when the customization dialog is being populated with
// toolbar information.  Basically, *pResult should be populated with
// your answer to the question, "is the user allowed to insert a
// button to the left of this one?".
void CToolBarEx::OnToolBarQueryInsert(NMHDR *pNMHDR, LRESULT *pResult)
{
	UNUSED_ALWAYS( pNMHDR );
	*pResult = TRUE;


}

// This function is called whenever the user makes a change to the
// layout of the toolbar.  Calling the mainframe's RecalcLayout forces
// the toolbar to repaint itself.
void CToolBarEx::OnToolBarChange(NMHDR *pNMHDR, LRESULT *pResult)
{
	UNUSED_ALWAYS( pNMHDR );
	UNUSED_ALWAYS( pResult );


	SetTextOptions(m_eTextOptions,FALSE);
	PositionControls();

	// force the frame window to recalculate the size
	GetParentFrame()->RecalcLayout();
	OnIdleUpdateCmdUI(TRUE, 0L);

}


// This function is called when the user initially calls up the toolbar
// customization dialog box.
void CToolBarEx::OnToolBarBeginAdjust(NMHDR *pNMHDR, LRESULT *pResult)
{
	UNUSED_ALWAYS( pNMHDR );
	UNUSED_ALWAYS( pResult );


    *pResult = 0;

}

LRESULT CToolBarEx::OnCustomize(WPARAM,LPARAM)
{
	LONG lResult;

	ASSERT(m_pCustomizeDlg==NULL);
    m_pCustomizeDlg = new CCustomizeDialog( this, m_eTextOptions, m_eIconOptions );
    m_hCBTHook = ::SetWindowsHookEx( WH_CBT, CBTProc, 0, ::GetCurrentThreadId() );
    ASSERT( m_hCBTHook != 0 );
	 
    lResult = Default();

    VERIFY( ::UnhookWindowsHookEx( m_hCBTHook ) );
    m_hCBTHook = 0;

	 // if custom draw is not done then update Icon Draw at end
	 if (GetIconOptions()!=m_pCustomizeDlg->GetIconOptions())
		SetIconOptions(m_pCustomizeDlg->GetIconOptions(),TRUE);

    delete m_pCustomizeDlg;
    m_pCustomizeDlg = NULL;

	SaveState();
	ASSERT(m_pCustomizeDlg==NULL);

	return 	lResult;

}

// This function is called when the user clicks on the help button on the
// toolbar customization dialog box.
void CToolBarEx::OnToolBarCustomHelp(NMHDR *pNMHDR, LRESULT *pResult)
{
	UNUSED_ALWAYS( pNMHDR );
	UNUSED_ALWAYS( pResult );

	TRACE(_T("Help on Customize Toolbar called.\n"));
}

// This function is called when the user dismisses the toolbar customization
// dialog box.
void CToolBarEx::OnToolBarEndAdjust(NMHDR *pNMHDR, LRESULT *pResult)
{
	UNUSED_ALWAYS( pNMHDR );
	UNUSED_ALWAYS( pResult );

}

// This function is called to populate the toolbar customization dialog box
// with information regarding all of the possible toolbar buttons.
void CToolBarEx::OnToolBarGetButtonInfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	UNUSED_ALWAYS( pResult );

	TBNOTIFY* tbStruct;		// data needed by customize dialog box

	// init the pointer
	tbStruct = (TBNOTIFY *)pNMHDR;

	// if the index is valid
	if ((0 <= tbStruct->iItem) && (tbStruct->iItem < m_ToolBarInfo.GetSize()))
	{
		// copy the stored button structure
		tbStruct->tbButton = m_ToolBarInfo[tbStruct->iItem].tbButton;

		// copy the text for the button label in the dialog
		_tcscpy(tbStruct->pszText, m_ToolBarInfo[tbStruct->iItem].btnText);

		// indicate valid data was sent
		*pResult = TRUE;
	}
	else  	// else there is no button for this index
	{
		*pResult = FALSE;
	}
}

// This function is called when the user clicks on the reset button on the
// toolbar customization dialog box.
void CToolBarEx::OnToolBarReset(NMHDR *pNMHDR, LRESULT *pResult)
{
	UNUSED_ALWAYS( pNMHDR );
	UNUSED_ALWAYS( pResult );


      int         nCount, i;
	  CToolBarCtrl &  tbCtrl =GetToolBarCtrl();
	
      // Remove all of the existing buttons 
      nCount = tbCtrl.GetButtonCount();

      for(i = nCount - 1; i >= 0; i--)
		 tbCtrl.DeleteButton(i);
      
      // Restore the buttons that were saved.
	  for (i=0;i<m_ToolBarInfo.GetSize();i++)
	  {
		  if (m_ToolBarInfo[i].bInitiallyVisible)
			 tbCtrl.AddButtons(1,&m_ToolBarInfo[i].tbButton);	
	  }

	if (m_pCustomizeDlg)
	{
	   ASSERT_VALID(m_pCustomizeDlg);
	   m_pCustomizeDlg->SetTextOptions(m_eInitialTextOptions);
	   m_pCustomizeDlg->SetIconOptions(m_eInitialIconOptions);
	}


	SetTextOptions(m_eInitialTextOptions);	//Default Values



	PositionControls();

	// force the frame window to recalculate the size
	GetParentFrame()->RecalcLayout();
	OnIdleUpdateCmdUI(TRUE, 0L);


	*pResult = TRUE;
}



void CToolBarEx::SetToolBarInfoForCustomization(const CToolBarEx::ToolBarInfoArray * pAdditional)
{
	 m_ToolBarInfo.RemoveAll();
	 const int nCount =	GetToolBarCtrl().GetButtonCount();
	 m_ToolBarInfo.SetSize(nCount); //SetSize


	 CToolBarCtrl & tbCtrl = GetToolBarCtrl();

	 for (int i=0;i<nCount;i++)
	 {
		CToolBarButtonInfo tbButtonInfo;

		tbCtrl.GetButton(i,&tbButtonInfo.tbButton);
		CString str;
		str.LoadString(tbButtonInfo.tbButton.idCommand);

		int nPos= str.ReverseFind(_T('\n'));
		tbButtonInfo.btnText=str.Right(str.GetLength()-nPos-1);
		tbButtonInfo.bInitiallyVisible=TRUE;
		
		m_ToolBarInfo.SetAt(i,tbButtonInfo);
	 }

	m_nResButtons =m_ToolBarInfo.GetSize();

	 // add the addiotnal buttons
	 if (pAdditional)
	 {
	   ASSERT_VALID(pAdditional);
	   m_ToolBarInfo.Append(*pAdditional);
	 }
	
	m_ToolBarInfo.FreeExtra();


	ModifyStyle(0, CCS_ADJUSTABLE);

	//SetTextOptions(m_eInitialTextOptions);	//Default Values
	//SetIconOptions(m_eInitialIconOptions);   //Default Values 
}

/////////////////////////////////////////////////////////////////////////////////////////
// Overrides 

int CToolBarEx::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CToolBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	_GetComCtlVersion();
	ASSERT(_ComCtlVersion>=VERSION_IE5);

	//set version
//	int nVer=5;
//	DefWindowProc(CCM_SETVERSION,nVer,0);
//	ASSERT(nVer==DefWindowProc(CCM_GETVERSION,nVer,0));

	return 0;
}


void CToolBarEx::PositionControls()
{
	GetToolBarCtrl().AutoSize();
	m_bDelayedButtonLayout=TRUE;
}


// the all important function
CSize CToolBarEx::GetButtonSize(TBBUTTON* pData, int iButton, DWORD dwMode)
{
	ASSERT(_ComCtlVersion > 0);

	// Get the actual size of the button, not what's in m_sizeButton.
	// Make sure to do SendMessage instead of calling MFC's GetItemRect,
	// which has all sorts of bad side-effects!
	// 
	CRect rc;
	SendMessage(TB_GETITEMRECT, iButton, (LPARAM)&rc);
	CSize sz = rc.Size();

	DWORD dwStyle = pData[iButton].fsStyle;
	BOOL bVertDocked= (!(dwMode&LM_HORZ) && !(dwMode&LM_STRETCH) ) ;

	// do not allow sepearot to be greater than Button height ( Have to check this)
	if (dwStyle & TBSTYLE_SEP)
		sz.cy = min(sz.cy ,HIWORD(GetToolBarCtrl().GetButtonSize()));



	// special cas  for custom controls
	if (m_bHideChildWndOnVertical)
	{
		if ((dwStyle & TBSTYLE_SEP) && 
				(pData[iButton].idCommand!=0))
		{

			if (bVertDocked)
			{
				sz.cx=sz.cy=0;
			}
			else
			{
		  		 // we will get 0,0 on hidden things
				if (GetToolBarCtrl().IsButtonHidden(pData[iButton].idCommand))
				{
				   CWnd * pWnd =GetDlgItem(pData[iButton].idCommand);

					ASSERT_VALID(pWnd);
					CRect rt;
					pWnd->GetWindowRect(rt);
					sz=rt.Size();
				}

			}
		}
	}  
		////////////////
	// Now must do special case for various versions of comctl32.dll,
	//
	if ((pData[iButton].fsState & TBSTATE_WRAP)) 
	{
		if (dwStyle & TBSTYLE_SEP) 
		{
			CWnd *pWnd =GetDlgItem(pData[iButton].idCommand);

			// Check seperator is child window
			if (!pWnd)
			{
				// this is the last separator in the row (eg vertically docked)
				// fudge the height, and ignore the width. TB_GETITEMRECT will return
				// size = (8 x 22) even for a separator in vertical toolbar
				//
				if (_ComCtlVersion <= VERSION_IE3)
					sz.cy -= 3;		// empircally good fudge factor
				else if (_ComCtlVersion != VERSION_IE4)
					sz.cy = sz.cx;
			
				sz.cx = 0;			// separator takes no width if it's the last one
			}
			else
			{
			  // Do not set value in case of the child window
			  //WE should never get here 
			  ASSERT_VALID(pWnd);
			  ASSERT(FALSE);
			}

		} 


	}
//	 // drop down arrow check
//	 if ((dwStyle & TBSTYLE_DROPDOWN) && 
//		  (bVertDocked) &&	
//		   !m_bShowDropdownArrowWhenVertical ) 
//	 {
//		// ignore width of dropdown
//		sz.cx = sz.cy;
//	 }

	return sz;
}



////////////////////////////////////////////////////////////

#define CX_OVERLAP  0

CSize CToolBarEx::CalcSize(TBBUTTON* pData, int nCount,DWORD dwMode)
{
	ASSERT(pData != NULL && nCount > 0);

	CPoint cur(0,0);
	CSize sizeResult(0,0);
	int cyTallestOnRow = 0;
	int nButtons=0;


	for (int i = 0; i < nCount; i++)
	{

		// also calculate for hidden custom controls
		if (  (pData[i].fsState & TBSTATE_HIDDEN)  &&
			!((pData[i].fsStyle & TBSTYLE_SEP) && (pData[i].idCommand!=0)))
			continue;

		// Load actual size of button into a local variable
		//
		CSize m_sizeButton = GetButtonSize(pData, i,dwMode);


		//  changed the logic below to be more correct.
		cyTallestOnRow = max(cyTallestOnRow, m_sizeButton.cy);
		sizeResult.cx = max(cur.x + m_sizeButton.cx, sizeResult.cx);
		sizeResult.cy = max(cur.y + m_sizeButton.cy, sizeResult.cy);

		cur.x += m_sizeButton.cx - CX_OVERLAP;
		if (!(pData[i].fsState & TBSTATE_HIDDEN)) nButtons++;

		if (pData[i].fsState & TBSTATE_WRAP)
		{
			//only seperator is present
			if ((nButtons==1) && (pData[i].fsStyle & TBSTYLE_SEP))
			{
				cyTallestOnRow = HIWORD(GetToolBarCtrl().GetButtonSize());
			}

			cur.x = 0;
			cur.y += cyTallestOnRow;
			cyTallestOnRow = 0;
			if (pData[i].fsStyle & TBSTYLE_SEP)
				cur.y += m_sizeButton.cy;
			 nButtons=0;
		}

	}
	return sizeResult;
}

int CToolBarEx::WrapToolBar(TBBUTTON* pData, int nCount, int nWidth, DWORD dwMode)
{
	ASSERT(pData != NULL && nCount > 0);

	int nResult = 0;
	int x = 0;
	for (int i = 0; i < nCount; i++)
	{
		pData[i].fsState &= ~TBSTATE_WRAP;

		// also calculate for hidden custom controls
		if (  (pData[i].fsState & TBSTATE_HIDDEN)  &&
			!((pData[i].fsStyle & TBSTYLE_SEP) && (pData[i].idCommand!=0)))
			continue;

		int dx, dxNext;

		// Load actual size of button into a local variable
		CSize m_sizeButton = GetButtonSize(pData, i,dwMode);

		dx = m_sizeButton.cx;
		dxNext = dx - CX_OVERLAP;

		if (x + dx > nWidth)
		{
			BOOL bFound = FALSE;
			for (int j = i; j >= 0  &&  !(pData[j].fsState & TBSTATE_WRAP); j--)
			{
				// Find last separator that isn't hidden
				// a separator that has a command ID is not
				// a separator, but a custom control.
				if ((pData[j].fsStyle & TBSTYLE_SEP) &&
					(pData[j].idCommand == 0)
					&&	!(pData[j].fsState & TBSTATE_HIDDEN))
				{
					bFound = TRUE; i = j; x = 0;
					pData[j].fsState |= TBSTATE_WRAP;
					nResult++;
					break;
				}
			}
			if (!bFound)
			{
				for (int j = i - 1; j >= 0 && !(pData[j].fsState & TBSTATE_WRAP); j--)
				{
					// Never wrap anything that is hidden,
					// or any custom controls
					if ((pData[j].fsState & TBSTATE_HIDDEN) ||
						((pData[j].fsStyle & TBSTYLE_SEP) &&
						(pData[j].idCommand != 0)))
						continue;

					bFound = TRUE; i = j; x = 0;
					pData[j].fsState |= TBSTATE_WRAP;
					nResult++;
					break;
				}
				if (!bFound)
					x += dxNext;
			}
		}
		else
			x += dxNext;
	}
	return nResult + 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CToolBarEx::SizeToolBar(TBBUTTON* pData, int nCount, int nLength, BOOL bVert, DWORD dwMode)
{
	ASSERT(pData != NULL && nCount > 0);

	if (!bVert)
	{
		int nMin, nMax, nTarget, nCurrent, nMid;

		// Wrap ToolBar as specified
		nMax = nLength;
		nTarget = WrapToolBar(pData, nCount, nMax,dwMode);

		// Wrap ToolBar vertically
		nMin = 0;
		nCurrent = WrapToolBar(pData, nCount, nMin,dwMode);

		if (nCurrent != nTarget)
		{
			while (nMin < nMax)
			{
				nMid = (nMin + nMax) / 2;
				nCurrent = WrapToolBar(pData, nCount, nMid,dwMode);

				if (nCurrent == nTarget)
					nMax = nMid;
				else
				{
					if (nMin == nMid)
					{
						WrapToolBar(pData, nCount, nMax,dwMode);
						break;
					}
					nMin = nMid;
				}
			}
		}
		CSize size = CalcSize(pData, nCount,dwMode);
		WrapToolBar(pData, nCount, size.cx,dwMode);
	}
	else
	{
		CSize sizeMax, sizeMin, sizeMid;

		// Wrap ToolBar vertically
		WrapToolBar(pData, nCount, 0,dwMode);
		sizeMin = CalcSize(pData, nCount,dwMode);

		// Wrap ToolBar horizontally
		WrapToolBar(pData, nCount, 32767,dwMode);
		sizeMax = CalcSize(pData, nCount,dwMode);

		while (sizeMin.cx < sizeMax.cx)
		{
			sizeMid.cx = (sizeMin.cx + sizeMax.cx) / 2;
			WrapToolBar(pData, nCount, sizeMid.cx,dwMode);
			sizeMid = CalcSize(pData, nCount,dwMode);

			if (nLength < sizeMid.cy)
			{
				if (sizeMin == sizeMid)
				{
					WrapToolBar(pData, nCount, sizeMax.cx,dwMode);
					return;
				}
				sizeMin = sizeMid;
			}
			else if (nLength > sizeMid.cy)
			{
				if (sizeMax == sizeMid)
				{
					WrapToolBar(pData, nCount, sizeMin.cx,dwMode);
					return;
				}
				sizeMax = sizeMid;
			}
			else
				return;
		}
	}
}


CSize CToolBarEx::CalcLayout(DWORD dwMode, int nLength)
{
	ASSERT_VALID(this);
	ASSERT(::IsWindow(m_hWnd));
	if (dwMode & LM_HORZDOCK)
		ASSERT(dwMode & LM_HORZ);

	int nCount;
	TBBUTTON* pData = NULL;
	CSize sizeResult(0,0);

	//BLOCK: Load Buttons
	{
		nCount = DefWindowProc(TB_BUTTONCOUNT, 0, 0);
		if (nCount != 0)
		{
			int i;
			pData = new TBBUTTON[nCount];
			for (i = 0; i < nCount; i++)
				_GetButton(i, &pData[i]);
		}
	}

	if (nCount > 0)
	{
		if (!(m_dwStyle & CBRS_SIZE_FIXED))
		{
			BOOL bDynamic = m_dwStyle & CBRS_SIZE_DYNAMIC;

			if (bDynamic && (dwMode & LM_MRUWIDTH))
				SizeToolBar(pData, nCount, m_nMRUWidth,FALSE,dwMode);
			else if (bDynamic && (dwMode & LM_HORZDOCK))
				SizeToolBar(pData, nCount, 32767,FALSE,dwMode);
			else if (bDynamic && (dwMode & LM_VERTDOCK))
				SizeToolBar(pData, nCount, 0,FALSE,dwMode);
			else if (bDynamic && (nLength != -1))
			{
				CRect rect; rect.SetRectEmpty();
				CalcInsideRect(rect, (dwMode & LM_HORZ));
				BOOL bVert = (dwMode & LM_LENGTHY);
				int nLen = nLength + (bVert ? rect.Height() : rect.Width());

				SizeToolBar(pData, nCount, nLen, bVert,dwMode);
			}
			else if (bDynamic && (m_dwStyle & CBRS_FLOATING))
				SizeToolBar(pData, nCount, m_nMRUWidth,FALSE,dwMode);
			else
				SizeToolBar(pData, nCount, (dwMode & LM_HORZ) ? 32767 : 0,FALSE,dwMode);
		}

		sizeResult = CalcSize(pData, nCount,dwMode);

		if (dwMode & LM_COMMIT)
		{
			int nControlCount = 0;
			BOOL bIsDelayed = m_bDelayedButtonLayout;
			m_bDelayedButtonLayout = FALSE;
			BOOL bVert = (m_dwStyle & CBRS_ORIENT_VERT) != 0;


			for (int i = 0; i < nCount; i++)
				if ((pData[i].fsStyle & TBSTYLE_SEP) && (pData[i].idCommand != 0))
					nControlCount++;


			if (nControlCount > 0)
			{

				for(int i = 0; i < nCount; i++)
				{
					if ((pData[i].fsStyle & TBSTYLE_SEP) && (pData[i].idCommand != 0))
					{
						
						CRect rt;
						CWnd* pWnd = GetDlgItem(pData[i].idCommand);
						if (pWnd != NULL)
						{
							ASSERT_VALID(pWnd);
  						    pWnd->GetWindowRect(rt);
							pData[i].iBitmap=rt.Width(); //width 

							if (bVert && m_bHideChildWndOnVertical)
	   							pData[i].fsState |= TBSTATE_HIDDEN;
							else	   			
								pData[i].fsState &= ~TBSTATE_HIDDEN;
						}

					}
				}
			}
		

			if ((m_dwStyle & CBRS_FLOATING) && (m_dwStyle & CBRS_SIZE_DYNAMIC))
				m_nMRUWidth = sizeResult.cx;
			for (int i = 0; i < nCount; i++)
					_SetButton(i, &pData[i]);

			{  
					//Now place the windows
					CWnd * pWnd = GetWindow(GW_CHILD);
					while(pWnd)
					{
					  ASSERT_VALID(pWnd);

					  int id =pWnd->GetDlgCtrlID();
						  ///////////////////////////
						// make sure the id is valid, and set the button 
						// style for a seperator.
						int nIndex = CommandToIndex( id ) ;
						if (nIndex>-1)
						{
							ASSERT( nIndex >= 0 );
							// insert the control into the toolbar.
							CRect rect;
							GetItemRect( nIndex, &rect );

							CRect rt;
							pWnd->GetWindowRect(&rt);
							rect.top+=max((rect.Height()-rt.Height())/2,0);

							pWnd->SetWindowPos(0, rect.left, rect.top, 0, 0,
								SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOCOPYBITS );
							//pWnd->SetFont( GetFont( ));


							if (bVert && m_bHideChildWndOnVertical)
							{
								ASSERT(GetToolBarCtrl().IsButtonHidden(id));
								pWnd->ShowWindow( SW_HIDE );
							}
							else
							{
								ASSERT(!GetToolBarCtrl().IsButtonHidden(id));
								pWnd->ShowWindow( SW_SHOW );
							}

						}
						else
						{
							pWnd->ShowWindow( SW_HIDE);
						}
					  //////////////////////////
					  pWnd=pWnd->GetNextWindow();
					}
			}

			m_bDelayedButtonLayout = bIsDelayed;
		}

		delete[] pData;
	}

	//BLOCK: Adjust Margins
	{
		CRect rect; rect.SetRectEmpty();
		CalcInsideRect(rect, (dwMode & LM_HORZ));
		sizeResult.cy -= rect.Height();
		sizeResult.cx -= rect.Width();

		CSize size = CControlBar::CalcFixedLayout((dwMode & LM_STRETCH), (dwMode & LM_HORZ));
		sizeResult.cx = max(sizeResult.cx, size.cx);
		sizeResult.cy = max(sizeResult.cy, size.cy);
	}
	return sizeResult;
}

CSize CToolBarEx::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	DWORD dwMode = bStretch ? LM_STRETCH : 0;
	dwMode |= bHorz ? LM_HORZ : 0;

	return CalcLayout(dwMode);
}

CSize CToolBarEx::CalcDynamicLayout(int nLength, DWORD dwMode)
{
	if ((nLength == -1) && !(dwMode & LM_MRUWIDTH) && !(dwMode & LM_COMMIT) &&
		((dwMode & LM_HORZDOCK) || (dwMode & LM_VERTDOCK)))
	{
		return CalcFixedLayout(dwMode & LM_STRETCH, dwMode & LM_HORZDOCK);
	}
	return CalcLayout(dwMode, nLength);
}

/////////////////////////////////////////////////////////////////////////////
// CToolBarEx attribute access

void CToolBarEx::_GetButton(int nIndex, TBBUTTON* pButton) const
{
	CToolBarEx* pBar = (CToolBarEx*)this;
	VERIFY(pBar->DefWindowProc(TB_GETBUTTON, nIndex, (LPARAM)pButton));
	// TBSTATE_ENABLED == TBBS_DISABLED so invert it
	pButton->fsState ^= TBSTATE_ENABLED;
}

void CToolBarEx::_SetButton(int nIndex, TBBUTTON* pButton)
{
	// get original button state
	TBBUTTON button;
	VERIFY(DefWindowProc(TB_GETBUTTON, nIndex, (LPARAM)&button));

	// prepare for old/new button comparsion
	button.bReserved[0] = 0;
	button.bReserved[1] = 0;
	// TBSTATE_ENABLED == TBBS_DISABLED so invert it
	pButton->fsState ^= TBSTATE_ENABLED;
	pButton->bReserved[0] = 0;
	pButton->bReserved[1] = 0;

	// nothing to do if they are the same
	if (memcmp(pButton, &button, sizeof(TBBUTTON)) != 0)
	{
		// don't redraw everything while setting the button
		DWORD dwStyle = GetStyle();
		ModifyStyle(WS_VISIBLE, 0);
		VERIFY(DefWindowProc(TB_DELETEBUTTON, nIndex, 0));
		
		// Force a recalc of the toolbar's layout to work around a comctl bug
		int iTextRows = (int)::SendMessage(m_hWnd, TB_GETTEXTROWS, 0, 0);
		::SendMessage(m_hWnd, WM_SETREDRAW, FALSE, 0);
		::SendMessage(m_hWnd, TB_SETMAXTEXTROWS, iTextRows+1, 0);
		::SendMessage(m_hWnd, TB_SETMAXTEXTROWS, iTextRows, 0);
		::SendMessage(m_hWnd, WM_SETREDRAW, TRUE, 0);
		

		VERIFY(DefWindowProc(TB_INSERTBUTTON, nIndex, (LPARAM)pButton));
		ModifyStyle(0, dwStyle & WS_VISIBLE);

		// invalidate appropriate parts
		if (((pButton->fsStyle ^ button.fsStyle) & TBSTYLE_SEP) ||
			((pButton->fsStyle & TBSTYLE_SEP) && pButton->iBitmap != button.iBitmap))
		{
			// changing a separator
			Invalidate();
		}
		else
		{
			// invalidate just the button
			CRect rect;
			if (DefWindowProc(TB_GETITEMRECT, nIndex, (LPARAM)&rect))
				InvalidateRect(rect);
		}
	}
}








BOOL CToolBarEx::SetDropDownButton(UINT nID, BOOL bArrow)
{
		
	int iButton = CommandToIndex(nID);
	
	DWORD dwStyle = GetButtonStyle(iButton);
	dwStyle |= TBSTYLE_DROPDOWN|( (!bArrow) ?BTNS_WHOLEDROPDOWN :0);
	SetButtonStyle(iButton, dwStyle);								

	GetToolBarCtrl().SetExtendedStyle(GetToolBarCtrl().GetExtendedStyle() |TBSTYLE_EX_DRAWDDARROWS);

	return TRUE;
}

BOOL CToolBarEx::IsVertDocked()
{
	UINT nID =GetParent()->GetDlgCtrlID();
	return ( (nID ==AFX_IDW_DOCKBAR_LEFT) || (nID== AFX_IDW_DOCKBAR_RIGHT));
}

void CToolBarEx::OnPaint() 
{
	CToolBar::OnPaint();

	//I have to use this, as there is not customdraw mesasges for seperators
	// draw the controls seperator again to hide it
	for (int i=0;i<GetToolBarCtrl().GetButtonCount();i++)
	{
		TBBUTTON tb;
		GetToolBarCtrl().GetButton(i,&tb);

		//only for seperators controls
		if  ( ((tb.fsStyle & TBSTYLE_SEP) == TBSTYLE_SEP) &&
			   (tb.idCommand!=0) )
		{
			CRect rc;
			GetItemRect(i,rc);
			CClientDC dc(this);

			CWnd *pWnd = GetDlgItem(tb.idCommand);
			ASSERT_VALID(pWnd);
			dc.FillSolidRect(rc, m_clrBtnFace);

			if (pWnd)
			{
				pWnd->RedrawWindow();
			}
		}
	}

	 
}

void CToolBarEx::SetTextOptions(TextOptions eTextOptions, BOOL bUpdate)
{
    ASSERT( ::IsWindow( m_hWnd ) );
    ASSERT( eTextOptions != toNone );

    m_eTextOptions = eTextOptions;

	    // Modify toolbar style according to new text options
    ModifyStyle(
        ( eTextOptions == toTextOnRight ) ? 0 : TBSTYLE_LIST,
        ( eTextOptions == toTextOnRight ) ? TBSTYLE_LIST : 0 );

    CToolBarCtrl& tbCtrl = GetToolBarCtrl();

    DWORD dwStyleEx = tbCtrl.GetExtendedStyle();

	tbCtrl.SetExtendedStyle(
        ( eTextOptions == toTextOnRight ) ?
            ( dwStyleEx |  TBSTYLE_EX_MIXEDBUTTONS ) :
            ( dwStyleEx & ~TBSTYLE_EX_MIXEDBUTTONS ) );


	
	for(int i=0;i<m_ToolBarInfo.GetSize();i++)
 		{ 
 		//	if ((m_ToolBarInfo[i].tbButton.fsStyle & TBSTYLE_SEP) !=  TBSTYLE_SEP)
 			{
 				int nIndex= CommandToIndex(m_ToolBarInfo[i].tbButton.idCommand);
 				
 				if (nIndex>-1) 	// buton is there on bar
				{

					 TBBUTTON tbinfo;

					_GetButton(nIndex,&tbinfo);

					if ( !( tbinfo.fsStyle & TBSTYLE_SEP ) )  //check again
					{
 							// attempt to lookup string index in map
							int nString = -1;
							if ((eTextOptions==toTextLabels) || ((eTextOptions==toTextOnRight)))
							{
								void* p=NULL;
								CString strTemp(m_ToolBarInfo[i].btnText);
								if (m_pStringMap != NULL && m_pStringMap->Lookup(strTemp, p))
									nString = (int)p;

								// add new string if not already in map
								if (nString == -1)
								{
									// initialize map if necessary
									if (m_pStringMap == NULL)
									{
										m_pStringMap = new CMapStringToPtr;
										ASSERT(m_pStringMap);
									}

									// add new string to toolbar list
									nString = (int)DefWindowProc(TB_ADDSTRING, 0, (LPARAM)(LPCTSTR)strTemp);
									if (nString != -1)
									{
										// cache string away in string map
										m_pStringMap->SetAt(strTemp, (void*)nString);
										ASSERT(m_pStringMap->Lookup(strTemp, p));
									}
								}

							}

						

							switch ( eTextOptions )
							{
								case toTextLabels:
									{
										tbinfo.iString =nString;
										tbinfo.fsStyle &= ~BTNS_SHOWTEXT;
										tbinfo.fsStyle |= TBSTYLE_AUTOSIZE;
									}
									break;

								case toTextOnRight:
									{
										if ((nString!=-1) && 
												(HasButtonText(m_ToolBarInfo[i].tbButton.idCommand)))
										{
											tbinfo.iString =nString;
											tbinfo.fsStyle |= ( TBSTYLE_AUTOSIZE |BTNS_SHOWTEXT );
										}
										else
										{
											tbinfo.fsStyle &= ~BTNS_SHOWTEXT;
											tbinfo.fsStyle |= TBSTYLE_AUTOSIZE;
										}
									}
									break;

								case toNoTextLabels:
									{
 										tbinfo.iString= -1;   //Set string to zero
										tbinfo.fsStyle &= ~BTNS_SHOWTEXT;
										tbinfo.fsStyle |= TBSTYLE_AUTOSIZE;
									}
									break;
								default:
									ASSERT(FALSE); // anew type added
							}

							//Delete & add the button
							{
								tbinfo.fsState ^= TBSTATE_ENABLED;
								m_ToolBarInfo[i].tbButton=tbinfo;
								tbinfo.fsState ^= TBSTATE_ENABLED;
								_SetButton(nIndex,&tbinfo);
							}
 					}
				}
			}
	}

	int iTextRows = 0;
	// now total setting
	switch ( eTextOptions )
	{
		case toTextLabels:
			{
  				iTextRows=1;
			}
			break;

		case toTextOnRight:
			{
  				iTextRows=1;
			}
			break;

		case toNoTextLabels:
			{
  				iTextRows=0;
			}
			break;
		default:
			ASSERT(FALSE); // anew type added
	}

	::SendMessage(m_hWnd, TB_SETMAXTEXTROWS, iTextRows, 0);
	


    // If requested, reflect changes immediately
    if ( bUpdate )
    {
		PositionControls();
		CFrameWnd *pFrameWnd = GetParentFrame();
		if ( pFrameWnd!=NULL ) {
			pFrameWnd->RecalcLayout();
		}
		OnIdleUpdateCmdUI(TRUE, 0L);
    }

}

 

void CToolBarEx::SetIconOptions(IconOptions eIconOptions, BOOL bUpdate)
{
    ASSERT( ::IsWindow( m_hWnd ) );
    //ASSERT( eIconOptions != ioNone );

    m_eIconOptions = eIconOptions;


	switch(eIconOptions)
	{
	 case ioSmallIcons:
		 m_sizeImage= m_szImageSmall ;
		 SetIconOptionBitmap(ioSmallIcons);
		 GetToolBarCtrl().SetBitmapSize(m_szImageSmall);
		break;

	 case ioLargeIcons:
		 m_sizeImage= m_szImageLarge ;
		 SetIconOptionBitmap(ioLargeIcons);
		 GetToolBarCtrl().SetBitmapSize(m_szImageLarge);
		 break;
	}

			// Force a recalc of the toolbar's layout to work around a comctl bug
		int iTextRows = (int)::SendMessage(m_hWnd, TB_GETTEXTROWS, 0, 0);
		::SendMessage(m_hWnd, WM_SETREDRAW, FALSE, 0);
		::SendMessage(m_hWnd, TB_SETMAXTEXTROWS, iTextRows+1, 0);
		::SendMessage(m_hWnd, TB_SETMAXTEXTROWS, iTextRows, 0);
		::SendMessage(m_hWnd, WM_SETREDRAW, TRUE, 0);


    // If requested, reflect changes immediately
    if ( bUpdate )
    {
		PositionControls();
		CFrameWnd *pFrameWnd = GetParentFrame();
		if ( pFrameWnd!=NULL ) {
			pFrameWnd->RecalcLayout();
		}
		OnIdleUpdateCmdUI(TRUE, 0L);
    }

}

//static ftn for Hook

LRESULT CALLBACK CToolBarEx::CBTProc( int nCode, WPARAM wParam, LPARAM lParam )
{
    ASSERT( m_pCustomizeDlg != NULL );

	try
	{
		if ( ( nCode == HCBT_CREATEWND ) && (m_pCustomizeDlg) && ( m_pCustomizeDlg->m_hWnd == 0 ) )
		{
			// This is where we have a good chance to subclass standard
			// "Customize toolbar" dialog in order to add to it some of
			// our new features (ie. text and icon options selectors)
			HWND hWnd = ( HWND )wParam;
			VERIFY( m_pCustomizeDlg->SubclassWindow( hWnd ) );
		}
	}
	catch(...)	// if exception is not caught then we can get in infinite loop as assert window is opened.
	{
		TRACE0(_T("Exception thrown while Hooking-Extra Custom features unavaialble.\n"));

		//Call next
		LRESULT lResult=::CallNextHookEx( m_hCBTHook, nCode, wParam, lParam );
		VERIFY( ::UnhookWindowsHookEx( m_hCBTHook ) );
		m_hCBTHook = 0;
		if (m_pCustomizeDlg) delete m_pCustomizeDlg;
		m_pCustomizeDlg = NULL;
		return lResult;
	}

    return ::CallNextHookEx( m_hCBTHook, nCode, wParam, lParam );
}


// has to override this
BOOL CToolBarEx::HasButtonText(int nID)
{
	nID;
    return TRUE;
}

/*
	DIBs use RGBQUAD format:
		0xbb 0xgg 0xrr 0x00

	Reasonably efficient code to convert a COLORREF into an
	RGBQUAD is byte-order-dependent, so we need different
	code depending on the byte order we're targeting.
*/
#define RGB_TO_RGBQUAD(r,g,b)   (RGB(b,g,r))
#define CLR_TO_RGBQUAD(clr)     (RGB(GetBValue(clr), GetGValue(clr), GetRValue(clr)))

static COLORREF MapToSysColor (COLORREF color, BOOL bUseRGBQUAD)
{
	struct COLORMAP
	{
		// use DWORD instead of RGBQUAD so we can compare two RGBQUADs easily
		DWORD rgbqFrom;
		int iSysColorTo;
	};
	static const COLORMAP sysColorMap[] =
	{
		// mapping from color in DIB to system color
		{ RGB_TO_RGBQUAD(0x00, 0x00, 0x00),  COLOR_BTNTEXT },       // black
		{ RGB_TO_RGBQUAD(0x80, 0x80, 0x80),  COLOR_BTNSHADOW },     // dark grey
		{ RGB_TO_RGBQUAD(0xC0, 0xC0, 0xC0),  COLOR_BTNFACE },       // bright grey
		{ RGB_TO_RGBQUAD(0xFF, 0xFF, 0xFF),  COLOR_BTNHIGHLIGHT }   // white
	};
	const int nMaps = 4;

	// look for matching RGBQUAD color in original
	for (int i = 0; i < nMaps; i++)
	{
		if (color == sysColorMap[i].rgbqFrom)
		{
			return bUseRGBQUAD ? 
				CLR_TO_RGBQUAD(::GetSysColor(sysColorMap[i].iSysColorTo)) :
				::GetSysColor(sysColorMap[i].iSysColorTo);
		}
	}

	return color;
}

static HBITMAP LoadSysColorBitmapSize(HINSTANCE hInst, HRSRC hRsrc,CSize sz)
{
	HGLOBAL hglb;

	if ((hglb = LoadResource(hInst, hRsrc)) == NULL)
		return NULL;

	LPBITMAPINFOHEADER lpBitmap = (LPBITMAPINFOHEADER)LockResource(hglb);
	if (lpBitmap == NULL)
		return NULL;

	UINT nSize=lpBitmap->biSize;
	const int nColorTableSize = 16;

	if (lpBitmap->biBitCount  <=8)
	{
		// make copy of BITMAPINFOHEADER so we can modify the color table
		nSize = lpBitmap->biSize + nColorTableSize * sizeof(RGBQUAD);
	}


	LPBITMAPINFOHEADER lpBitmapInfo = (LPBITMAPINFOHEADER)::malloc(nSize);
	if (lpBitmapInfo == NULL)	return NULL;

	memcpy(lpBitmapInfo, lpBitmap, nSize);
	HBITMAP hbm	=NULL;
	if (lpBitmapInfo->biBitCount  <=8)
	{
		// color table is in RGBQUAD DIB format
		DWORD* pColorTable =
			(DWORD*)(((LPBYTE)lpBitmapInfo) + (UINT)lpBitmapInfo->biSize);

		for (int iColor = 0; iColor < nColorTableSize; iColor++)
		{
			pColorTable[iColor] = MapToSysColor(pColorTable[iColor],TRUE) ;
		}	

		int nWidth = (int)lpBitmapInfo->biWidth;
		int nHeight = (int)lpBitmapInfo->biHeight;
		HDC hDCScreen = ::GetDC(NULL);
		hbm = ::CreateCompatibleBitmap(hDCScreen, sz.cx, sz.cy);

		if (hbm != NULL)
		{
			HDC hDCGlyphs = ::CreateCompatibleDC(hDCScreen);
			HBITMAP hbmOld = (HBITMAP)::SelectObject(hDCGlyphs, hbm);

			LPBYTE lpBits;
			lpBits = (LPBYTE)(lpBitmap + 1);
			lpBits += (1 << (lpBitmapInfo->biBitCount)) * sizeof(RGBQUAD);

			StretchDIBits(hDCGlyphs, 0, 0, sz.cx, sz.cy, 0, 0, nWidth, nHeight,
				lpBits, (LPBITMAPINFO)lpBitmapInfo, DIB_RGB_COLORS, SRCCOPY);
			SelectObject(hDCGlyphs, hbmOld);
			::DeleteDC(hDCGlyphs);
		}
		::ReleaseDC(NULL, hDCScreen);


	}
	else
	{

		int nWidth = (int)lpBitmapInfo->biWidth;
		int nHeight = (int)lpBitmapInfo->biHeight;
		HDC hDCScreen = ::GetDC(NULL);
		hbm = ::CreateCompatibleBitmap(hDCScreen, sz.cx, sz.cy);

		if (hbm != NULL)
		{
			HDC hDCGlyphs = ::CreateCompatibleDC(hDCScreen);
			HBITMAP hbmOld = (HBITMAP)::SelectObject(hDCGlyphs, hbm);

			LPBYTE lpBits;
			lpBits = (LPBYTE)(lpBitmap + 1);

			StretchDIBits(hDCGlyphs, 0, 0, sz.cx, sz.cy, 0, 0, nWidth, nHeight,
				lpBits, (LPBITMAPINFO)lpBitmapInfo, DIB_RGB_COLORS, SRCCOPY);

			// Change a specific colors to system:
			for (int x = 0; x < sz.cx; x ++)
			{
				for (int y = 0; y < sz.cy; y ++)
				{
					COLORREF clrOrig = ::GetPixel (hDCGlyphs, x, y);
					COLORREF clrNew = MapToSysColor (clrOrig,FALSE);

					if (clrOrig != clrNew)
					{
						::SetPixel (hDCGlyphs, x, y, clrNew);
					}
				}
			}

			SelectObject(hDCGlyphs, hbmOld);
			::DeleteDC(hDCGlyphs);
		}
		::ReleaseDC(NULL, hDCScreen);

	}


	// free copy of bitmap info struct and resource itself
	::free(lpBitmapInfo);
	::FreeResource(hglb);

	return hbm;
}


BOOL CToolBarEx::SetIconOptionBitmap(IconOptions eIconOptions)
{
	if ( eIconOptions==ioNone ) {
		SetBitmap(NULL);
		return TRUE;
	}

	if ((m_hInstImageWell != NULL) && (m_hRsrcImageWell != NULL))
	{

		HBITMAP hBitmap=NULL;
		HINSTANCE  hInstImageWell= m_hInstImageWell	;
		HRSRC  hRsrcImageWell	=  m_hRsrcImageWell	;

		int nCount = 0;

		for(UINT i=0;i<m_nResButtons;i++)
		{
		 
			if ((m_ToolBarInfo[i].tbButton.idCommand !=0))
				nCount++;
		}

		if (eIconOptions==ioSmallIcons)
		{
			CSize  sizeSmallImage(nCount *m_szImageSmall.cx,m_szImageSmall.cy);
			hBitmap= LoadSysColorBitmapSize(m_hInstImageWell, m_hRsrcImageWell,sizeSmallImage);
		}
		else
		{
			ASSERT(eIconOptions==ioLargeIcons);
			CSize  sizeLargeImage(nCount *m_szImageLarge.cx,m_szImageLarge.cy);
			hBitmap= LoadSysColorBitmapSize(m_hInstImageWell, m_hRsrcImageWell,sizeLargeImage);
		}

		ASSERT(hBitmap);
		SetBitmap(hBitmap);

		m_hInstImageWell = hInstImageWell; // SetBitmap resets these
		m_hRsrcImageWell = hRsrcImageWell; // SetBitmap resets these
		return TRUE;
	}


	return FALSE;
}

void CToolBarEx::OnSysColorChange() 
{
	m_clrBtnShadow=::GetSysColor(COLOR_BTNSHADOW);
	m_clrBtnHilight=::GetSysColor(COLOR_BTNHILIGHT);
	m_clrBtnFace  = ::GetSysColor(COLOR_BTNFACE);
	SetIconOptions(m_eIconOptions);	//load bitmap again
}

void CToolBarEx::OnSave( NMHDR* pNMHDR, LRESULT* pResult )
{
    NMTBSAVE* lpnmtb = ( NMTBSAVE* )pNMHDR;
    if ( lpnmtb->iItem == -1 )
    {
        lpnmtb->cbData  += sizeof( DWORD ) * 2;
        lpnmtb->pData    = ( LPDWORD )::GlobalAlloc( GMEM_FIXED, lpnmtb->cbData );  //freed by Toolbar ( Not sure)
        lpnmtb->pCurrent = lpnmtb->pData;

        *lpnmtb->pCurrent++ = (DWORD)m_eIconOptions;
        *lpnmtb->pCurrent++ = (DWORD)m_eTextOptions;
    }

    *pResult = 0;
}

void CToolBarEx::OnRestore( NMHDR* pNMHDR, LRESULT* pResult )
{
    NMTBRESTORE* lpnmtb = ( NMTBRESTORE* )pNMHDR;

    if ( lpnmtb->iItem == -1 )
    {
        lpnmtb->cButtons = ( lpnmtb->cbData - sizeof( DWORD ) * 2 ) / lpnmtb->cbBytesPerRecord;
        lpnmtb->pCurrent = lpnmtb->pData;

        m_eIconOptions=(IconOptions)*lpnmtb->pCurrent++ ;
        m_eTextOptions=(TextOptions)*lpnmtb->pCurrent++ ;
    }

    *pResult = 0;
}


BOOL CToolBarEx::ShowContextMenu(CPoint point)
{

	if( GetToolBarCtrl().GetButtonCount() && m_ToolBarInfo.GetSize())
	{
		CPoint pt( point );
		ClientToScreen( &pt );
		
		// load and display popup menu
		CMenu popupMenu;
		VERIFY(popupMenu.CreatePopupMenu());

		CFrameWnd * pFrame=(CFrameWnd *)AfxGetMainWnd();

		if (pFrame && pFrame->IsKindOf(RUNTIME_CLASS(CFrameWnd)) )
		{
			ASSERT_VALID(pFrame);
			POSITION pos = pFrame->m_listControlBars.GetHeadPosition();

			while (pos != NULL)
			{
				const CControlBar* pBar = (CControlBar*)pFrame->m_listControlBars.GetNext(pos);
				ASSERT(pBar != NULL);
				
				if (pBar && 
					pBar->IsKindOf(RUNTIME_CLASS(CToolBar))) // Put any other type

				{
					ASSERT_VALID(pBar);
					CString strName;
					pBar->GetWindowText(strName);
					popupMenu.AppendMenu( (pBar->GetStyle()&WS_VISIBLE?MF_CHECKED:MF_UNCHECKED)|MF_STRING,
											pBar->GetDlgCtrlID(),
											strName);
				}
			}
			
			//Add a Seperator
			popupMenu.AppendMenu(MF_SEPARATOR) ;
		}

		
		//Finally a Customize
		popupMenu.AppendMenu(MF_STRING ,ID_CUSTOMIZE_BAR, STR_CUSTOMIZE);

		int nResult = popupMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_VERTICAL|TPM_RETURNCMD,pt.x, pt.y, this );
	
		switch(nResult)
		{
			case 0:		//Nothing clicked  on menu
				break;
			case ID_CUSTOMIZE_BAR  :
				// open the customization dialog.
				Customize();
			break;
			default:
				{
					ASSERT_VALID(pFrame);
					if (pFrame && pFrame->IsKindOf(RUNTIME_CLASS(CFrameWnd)) )
					{	
						pFrame->OnBarCheck(nResult);
					}
				}
		}
		return TRUE;
	}


	return FALSE;

}

void CToolBarEx::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	ASSERT_VALID(pWnd);
	CPoint pt(point);

	if (WindowFromPoint(pt)!=this) return;

	ScreenToClient(&pt);
	if (!ShowContextMenu(pt))
		CToolBar::OnContextMenu(pWnd,point);
}

void CToolBarEx::Customize()
{
	GetToolBarCtrl().Customize();
}

void CToolBarEx::OnInitCustomize( NMHDR* /*pNMHDR*/, LRESULT* pResult )
{
    *pResult = TBNRF_HIDEHELP;  // help button  is hidden      
}

void CToolBarEx::OnCustomDraw( NMHDR* pNMHDR, LRESULT* pResult )
{
	LPNMTBCUSTOMDRAW lpNMCustomDraw = (LPNMTBCUSTOMDRAW) pNMHDR;

	ASSERT_POINTER(lpNMCustomDraw,NMTBCUSTOMDRAW);
	switch(lpNMCustomDraw->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW    ;
		break;
	case CDDS_ITEMPREPAINT:
		// draw the button
		{
			if (lpNMCustomDraw->nmcd.uItemState & CDIS_HOT)
			{
				//draw button
				CDC dc;
				CRect rt(lpNMCustomDraw->nmcd.rc);
				TBBUTTONINFO bbi;
				int xOffset=5,yOffset=3;

				dc.Attach(lpNMCustomDraw->nmcd.hdc);
				//Init button info
				memset(&bbi,0,sizeof(bbi));
				bbi.cbSize =sizeof(bbi);
				bbi.dwMask = TBIF_IMAGE;

				GetToolBarCtrl().GetButtonInfo(lpNMCustomDraw->nmcd.dwItemSpec,&bbi);
				
				


				//draw shadow
				CDC  dcGlyphs;
				dcGlyphs.CreateCompatibleDC(NULL);
				HBITMAP hbmOld = (HBITMAP)dcGlyphs.SelectObject(m_hbmImageWell);

				dc.BitBlt(rt.left+xOffset+1,rt.top+yOffset+1,m_sizeImage.cx,m_sizeImage.cy,&dcGlyphs,m_sizeImage.cx*bbi.iImage,0,SRCCOPY);
				dcGlyphs.SelectObject(hbmOld);
				dcGlyphs.DeleteDC();
			
				
				// darken the drawn thing
				long r,g,b;
				float  perc=0.07f;
				for (int x= rt.left+1;x< rt.right;x++)
					for (int y= rt.top+1;y< rt.bottom;y++)
					{
						COLORREF cr = dc.GetPixel(CPoint(x,y));
						if (cr == m_clrBtnFace)
						{
							r = GetRValue(cr);
							g = GetGValue(cr);
							b = GetBValue(cr);

							r -=(BYTE)(r*perc);
							g -=(BYTE)(g*perc);
							b -=(BYTE)(b*perc);
							dc.SetPixel(CPoint(x,y),RGB(r,g,b));
						}
						else
							dc.SetPixel(CPoint(x,y),m_clrBtnShadow);  // this creates shadow

					}



				dc.Draw3dRect(&rt,m_clrBtnShadow,m_clrBtnShadow);



				dc.Detach();

				*pResult = CDRF_SKIPDEFAULT;
			}
			else  // for normal cases
			{
				*pResult = CDRF_DODEFAULT ;
			}
		}
		break;
	case CDRF_NOTIFYPOSTPAINT :
		*pResult = CDRF_DODEFAULT ;
		ASSERT(FALSE);
		break;
	default:
		*pResult = CDRF_DODEFAULT ;
	}
}




void CToolBarEx::OnNcPaint() 
{

	if( GetToolBarCtrl().GetStyle()& TBSTYLE_FLAT ) 
	{
		// get window DC that is clipped to the non-client area
		CWindowDC dc(this);
		CRect rectClient;
		GetClientRect(rectClient);
		CRect rectWindow;
		GetWindowRect(rectWindow);
		ScreenToClient(rectWindow);
		rectClient.OffsetRect(-rectWindow.left, -rectWindow.top);
		dc.ExcludeClipRect(rectClient);

		// draw borders in non-client area
		rectWindow.OffsetRect(-rectWindow.left, -rectWindow.top);

		Draw3DBorders(&dc, rectWindow);

		dc.IntersectClipRect(rectWindow);

		// erase parts not drawn
		SendMessage(WM_ERASEBKGND, (WPARAM)dc.m_hDC);

		DrawGripper(&dc,rectWindow);
	} 
	else
	{
		CToolBar::OnNcPaint();
	}
}

void CToolBarEx :: Draw3DBorders(CDC * pDC, CRect & rect) 
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	if( GetToolBarCtrl().GetStyle()& TBSTYLE_FLAT ) 
	{
		DWORD dwStyle = m_dwStyle;
		if (!(dwStyle & CBRS_BORDER_ANY))
			return;

		COLORREF clr = (m_dwStyle & CBRS_BORDER_3D) ? m_clrBtnHilight : m_clrBtnShadow;
		if(m_dwStyle & CBRS_BORDER_LEFT)
			pDC->FillSolidRect(0, 0, 1, rect.Height() - 1, clr);
		if(m_dwStyle & CBRS_BORDER_TOP)
			pDC->FillSolidRect(0, 0, rect.Width()-1 , 1, clr);
		
		
		// this for ver 6.0
		if (dwStyle & CBRS_BORDER_ANY)
		{
			CRect rctBk;
			rctBk.left = 0;
			rctBk.right = rect.right;
			rctBk.top = rect.top;
			rctBk.bottom = rect.bottom;

			pDC->FillSolidRect((LPRECT)rctBk, pDC->GetBkColor());
		}

		if(m_dwStyle & CBRS_BORDER_RIGHT)
			pDC->FillSolidRect(rect.right, 1, -1, rect.Height() - 1, m_clrBtnShadow);
		if(m_dwStyle & CBRS_BORDER_BOTTOM)
			pDC->FillSolidRect(0, rect.bottom, rect.Width()-1, -1, m_clrBtnShadow);



		// if undockable toolbar at top of frame, apply special formatting to mesh
		// properly with frame menu
		if(!m_pDockContext) 
		{
			pDC->FillSolidRect(0,0,rect.Width(),1,m_clrBtnShadow);
			pDC->FillSolidRect(0,1,rect.Width(),1,m_clrBtnHilight);
		}

		if (dwStyle & CBRS_BORDER_LEFT)
			++rect.left;
		if (dwStyle & CBRS_BORDER_TOP)
			++rect.top;
		if (dwStyle & CBRS_BORDER_RIGHT)
			--rect.right;
		if (dwStyle & CBRS_BORDER_BOTTOM)
			--rect.bottom;
	} 
	else
	{
		DrawBorders(pDC, rect);
	}


}
	 
BOOL CToolBarEx::LoadBitmap(LPCTSTR lpszResourceName)
{
	ASSERT_VALID(this);
	ASSERT(lpszResourceName != NULL);

	// determine location of the bitmap in resource fork
	HINSTANCE hInstImageWell = AfxFindResourceHandle(lpszResourceName, RT_BITMAP);
	HRSRC hRsrcImageWell = ::FindResource(hInstImageWell, lpszResourceName, RT_BITMAP);
	if (hRsrcImageWell == NULL)
		return FALSE;

	// delete last one
	if (hRsrcImageWell) ::DeleteObject(hRsrcImageWell);



	// remember the resource handles so the bitmap can be recolored if necessary
	m_hInstImageWell = hInstImageWell;
	m_hRsrcImageWell = hRsrcImageWell;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CDockBarEx

CDockBarEx::CDockBarEx()
{
	// TODO: add construction code here.
}

CDockBarEx::~CDockBarEx()
{
	// TODO: add destruction code here.
}

IMPLEMENT_DYNAMIC(CDockBarEx, CDockBar)

BEGIN_MESSAGE_MAP(CDockBarEx, CDockBar)
	//{{AFX_MSG_MAP(CDockBarEx)				  
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDockBarEx message handlers

CSize CDockBarEx::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	ASSERT_VALID(this);

	CSize sizeFixed = CControlBar::CalcFixedLayout(bStretch, bHorz);

	// get max size
	CSize sizeMax;
	if (!m_rectLayout.IsRectEmpty())
	{
		CRect rect = m_rectLayout;
		CalcInsideRect(rect, bHorz);
		sizeMax = rect.Size();
	}
	else
	{
		CRect rectFrame;
		CFrameWnd* pFrame = GetParentFrame();
		pFrame->GetClientRect(&rectFrame);
		sizeMax = rectFrame.Size();
	}

	// prepare for layout
	AFX_SIZEPARENTPARAMS layout;
	layout.hDWP = m_bLayoutQuery ?
		NULL : ::BeginDeferWindowPos(m_arrBars.GetSize());
	int cxBorder = 2, cyBorder = 2;
	CPoint pt(-cxBorder, -cyBorder);
	int nWidth = 0;

	BOOL bWrapped = FALSE;

	// layout all the control bars
	for (int nPos = 0; nPos < m_arrBars.GetSize(); nPos++)
	{
		CControlBar* pBar = GetDockedControlBar(nPos);
		void* pVoid = m_arrBars[nPos];

		if (pBar != NULL)
		{
			if(pBar->IsKindOf(RUNTIME_CLASS(CToolBarEx)))
			{
			  if (pBar->GetStyle() & TBSTYLE_FLAT)
				cxBorder = cyBorder = 0;
			  else
				cxBorder = cyBorder = 2;
			}
			else
				cxBorder = cyBorder = 2;

			if (pBar->IsVisible())
			{
				// get ideal rect for bar
				DWORD dwMode = 0;
				if ((pBar->m_dwStyle & CBRS_SIZE_DYNAMIC) &&
					(pBar->m_dwStyle & CBRS_FLOATING))
					dwMode |= LM_HORZ | LM_MRUWIDTH;
				else if (pBar->m_dwStyle & CBRS_ORIENT_HORZ)
					dwMode |= LM_HORZ | LM_HORZDOCK;
				else
					dwMode |=  LM_VERTDOCK;

				CSize sizeBar = pBar->CalcDynamicLayout(-1, dwMode);

				CRect rect(pt, sizeBar);

				// get current rect for bar
				CRect rectBar;
				pBar->GetWindowRect(&rectBar);
				ScreenToClient(&rectBar);

				if (bHorz)
				{
					// Offset Calculated Rect out to Actual
					if (rectBar.left > rect.left && !m_bFloating)
						rect.OffsetRect(rectBar.left - rect.left, 0);

					// If ControlBar goes off the right, then right justify
					if (rect.right > sizeMax.cx && !m_bFloating)
					{
						int x = rect.Width() - cxBorder;
						x = max(sizeMax.cx - x, pt.x);
						rect.OffsetRect(x - rect.left, 0);
					}

					// If ControlBar has been wrapped, then left justify
					if (bWrapped)
					{
						bWrapped = FALSE;
						rect.OffsetRect(-(rect.left + cxBorder), 0);
					}
					// If ControlBar is completely invisible, then wrap it
					else if ((rect.left >= (sizeMax.cx - cxBorder)) &&
						(nPos > 0) && (m_arrBars[nPos - 1] != NULL))
					{
						m_arrBars.InsertAt(nPos, (CObject*)NULL);
						pBar = NULL; pVoid = NULL;
						bWrapped = TRUE;
					}
					if (!bWrapped)
					{
						if (rect != rectBar)
						{
							if (!m_bLayoutQuery &&
								!(pBar->m_dwStyle & CBRS_FLOATING))
							{
								pBar->m_pDockContext->m_rectMRUDockPos = rect;
							}
							AfxRepositionWindow(&layout, pBar->m_hWnd, &rect);
						}
						pt.x = rect.left + sizeBar.cx - cxBorder;
						nWidth = max(nWidth, sizeBar.cy);
					}
				}
				else
				{
					// Offset Calculated Rect out to Actual
					if (rectBar.top > rect.top && !m_bFloating)
						rect.OffsetRect(0, rectBar.top - rect.top);

					// If ControlBar goes off the bottom, then bottom justify
					if (rect.bottom > sizeMax.cy && !m_bFloating)
					{
						int y = rect.Height() - cyBorder;
						y = max(sizeMax.cy - y, pt.y);
						rect.OffsetRect(0, y - rect.top);
					}

					// If ControlBar has been wrapped, then top justify
					if (bWrapped)
					{
						bWrapped = FALSE;
						rect.OffsetRect(0, -(rect.top + cyBorder));
					}
					// If ControlBar is completely invisible, then wrap it
					else if ((rect.top >= (sizeMax.cy - cyBorder)) &&
						(nPos > 0) && (m_arrBars[nPos - 1] != NULL))
					{
						m_arrBars.InsertAt(nPos, (CObject*)NULL);
						pBar = NULL; pVoid = NULL;
						bWrapped = TRUE;
					}
					if (!bWrapped)
					{
						if (rect != rectBar)
						{
							if (!m_bLayoutQuery &&
								!(pBar->m_dwStyle & CBRS_FLOATING))
							{
								pBar->m_pDockContext->m_rectMRUDockPos = rect;
							}
							AfxRepositionWindow(&layout, pBar->m_hWnd, &rect);
						}
						pt.y = rect.top + sizeBar.cy - cyBorder;
						nWidth = max(nWidth, sizeBar.cx);
					}
				}
			}
			if (!bWrapped)
			{
				// handle any delay/show hide for the bar
				pBar->RecalcDelayShow(&layout);
			}
		}
		if (pBar == NULL && pVoid == NULL && nWidth != 0)
		{
			// end of row because pBar == NULL
			if (bHorz)
			{
				pt.y += nWidth - cyBorder;
				sizeFixed.cx = max(sizeFixed.cx, pt.x);
				sizeFixed.cy = max(sizeFixed.cy, pt.y);
				pt.x = -cxBorder;
			}
			else
			{
				pt.x += nWidth - cxBorder;
				sizeFixed.cx = max(sizeFixed.cx, pt.x);
				sizeFixed.cy = max(sizeFixed.cy, pt.y);
				pt.y = -cyBorder;
			}
			nWidth = 0;
		}
	}
	if (!m_bLayoutQuery)
	{
		// move and resize all the windows at once!
		if (layout.hDWP == NULL || !::EndDeferWindowPos(layout.hDWP))
			TRACE0("Warning: DeferWindowPos failed - low system resources.\n");
	}

	// adjust size for borders on the dock bar itself
	CRect rect;
	rect.SetRectEmpty();
	CalcInsideRect(rect, bHorz);

	if ((!bStretch || !bHorz) && sizeFixed.cx != 0)
		sizeFixed.cx += -rect.right + rect.left;
	if ((!bStretch || bHorz) && sizeFixed.cy != 0)
		sizeFixed.cy += -rect.bottom + rect.top;

	return sizeFixed;
}



// dwDockBarMap
const DWORD dwDockBarMap[4][2] =
{
	{ AFX_IDW_DOCKBAR_TOP,      CBRS_TOP    },
	{ AFX_IDW_DOCKBAR_BOTTOM,   CBRS_BOTTOM },
	{ AFX_IDW_DOCKBAR_LEFT,     CBRS_LEFT   },
	{ AFX_IDW_DOCKBAR_RIGHT,    CBRS_RIGHT  },
};


void FrameEnableDocking(CFrameWnd * pFrame, DWORD dwDockStyle) 
{
	ASSERT_VALID(pFrame);

	// must be CBRS_ALIGN_XXX or CBRS_FLOAT_MULTI only
	ASSERT((dwDockStyle & ~(CBRS_ALIGN_ANY|CBRS_FLOAT_MULTI)) == 0);

	pFrame->EnableDocking(dwDockStyle);

	for (int i = 0; i < 4; i++) {
		if (dwDockBarMap[i][1] & dwDockStyle & CBRS_ALIGN_ANY) {
			CDockBar* pDock = (CDockBar*)pFrame->GetControlBar(dwDockBarMap[i][0]);
			
			// make sure the dock bar is of correct type
			if( pDock == 0 || ! pDock->IsKindOf(RUNTIME_CLASS(CDockBarEx)) ) {
				BOOL bNeedDelete = ! pDock->m_bAutoDelete;
				pDock->m_pDockSite->RemoveControlBar(pDock);
				pDock->m_pDockSite = 0;	// avoid problems in destroying the dockbar
				pDock->DestroyWindow();
				if( bNeedDelete )
					delete pDock;
				pDock = 0;
			}

			if( pDock == 0 ) {
				pDock = new CDockBarEx;
				ASSERT_VALID(pDock);
				if ((!pDock) || (!pDock->Create(pFrame,
					WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_CHILD|WS_VISIBLE |
						dwDockBarMap[i][1], dwDockBarMap[i][0]))) {
					AfxThrowResourceException();
				}
			}
		}
	}
}


BOOL CToolBarEx::MarkDefaultState()
{

	 CToolBarCtrl & tbCtrl = GetToolBarCtrl();

		 //Set intial Visible info
	for (int i=0;i<m_ToolBarInfo.GetSize();i++)
	{
		if  ( (m_ToolBarInfo[i].tbButton.fsStyle & TBSTYLE_SEP) == 0) 	//   seperator	?

		{	 // Not a seperator
			 if  (tbCtrl.CommandToIndex(m_ToolBarInfo[i].tbButton.idCommand)==-1)	// button exists on toolbar
			{
				m_ToolBarInfo[i].bInitiallyVisible=FALSE;
			}
			else
			{
				m_ToolBarInfo[i].bInitiallyVisible=TRUE;
			}
		}
		else
		{
			if (m_ToolBarInfo[i].tbButton.idCommand>0)
			{
				if (tbCtrl.CommandToIndex(m_ToolBarInfo[i].tbButton.idCommand)==-1)
				{
					m_ToolBarInfo[i].bInitiallyVisible=FALSE;
				}
				else
				{
					m_ToolBarInfo[i].bInitiallyVisible=TRUE;
				}
			}
			else
			{
					m_ToolBarInfo[i].bInitiallyVisible=TRUE;	// simple seperatores are visible
			}
		}
	}
	return TRUE;
}



#if !defined(_TOOLBAREX_H_)
#define _TOOLBAREX_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ToolBarEx.h : header file
//
#include <afxtempl.h>
#ifndef __AFXPRIV_H__
	#include <afxpriv.h>	
#endif

#include "CustomizeDialog.h"
////////////////////////////////////////////////////////////////////////////
// I got some pieces of this code from  MSJ Articles of Paul Dilascia
//


/////////////////////////////////////////////////////////////////////////////
// CToolBarEx window

#define ID_CUSTOMIZE_BAR 0xFF


/////////////////////////////////////////////////////////////////////////////
// COMCTL32 v5.81 specific stuff:

#ifndef TBSTYLE_EX_MIXEDBUTTONS
#define TBSTYLE_EX_MIXEDBUTTONS         0x00000008
#define TBSTYLE_EX_HIDECLIPPEDBUTTONS   0x00000010
#define CCM_SETVERSION       (CCM_FIRST+7)
#define CCM_GETVERSION       (CCM_FIRST+8)
#endif  // !TBSTYLE_EX_MIXEDBUTTONS

#ifndef BTNS_SHOWTEXT
#define BTNS_SHOWTEXT           0x0040
#endif  // !BTNS_SHOWTEXT

#ifndef BTNS_WHOLEDROPDOWN
#define BTNS_WHOLEDROPDOWN      0x0080
#endif  // !BTNS_WHOLEDROPDOWN

#ifndef TBN_SAVE
#define TBN_SAVE                (TBN_FIRST - 22)
#define TBN_RESTORE             (TBN_FIRST - 21)


#ifndef TBN_INITCUSTOMIZE
#define TBN_INITCUSTOMIZE       (TBN_FIRST - 23)
#define TBNRF_HIDEHELP          0x00000001
#endif  // !TBN_INITCUSTOMIZE


typedef struct tagNMTBSAVE
{
    NMHDR hdr;
    DWORD* pData;
    DWORD* pCurrent;
    UINT cbData;
    int iItem;
    int cButtons;
    TBBUTTON tbButton;
} NMTBSAVE, *LPNMTBSAVE;

typedef struct tagNMTBRESTORE
{
    NMHDR hdr;
    DWORD* pData;
    DWORD* pCurrent;
    UINT cbData;
    int iItem;
    int cButtons;
    int cbBytesPerRecord;
    TBBUTTON tbButton;
} NMTBRESTORE, *LPNMTBRESTORE;

#endif  // !TBN_SAVE




class CToolBarEx : public CToolBar
{
protected:
	friend class CCustomizeDialog;	
	// Inner structs
	struct CToolBarButtonInfo
	{
		TBBUTTON		 tbButton;		// information regarding the button
		CString			 btnText;		// text for the button
		BOOL			 bInitiallyVisible ; //  Button intially vible	

	//	BOOL			 bShowOnRight;  // Visible on Selective  Right
	};

	// One of these for each drop-down button
	struct CDropDownButtonInfo
	{
		CDropDownButtonInfo * pNext;
		UINT			idButton;		// command ID of button
		UINT			idMenu;			// popup menu to display
	} ;

// Construction
public:
	CToolBarEx();

	typedef CArray<CToolBarButtonInfo,CToolBarButtonInfo &> ToolBarInfoArray;

// Attributes
public:
	const static CSize m_szImageSmall;
	const static CSize m_szImageLarge;
	const static TextOptions  m_eInitialTextOptions;
	const static IconOptions  m_eInitialIconOptions;



// Operations
public:

	// this member function is called to insert a control into the toolbar
	// and returns a pointer to the newly inserted control
	//
	CWnd* InsertControl(CRuntimeClass* pClass,LPCTSTR lpszWindowName,CRect& rect,UINT nID,DWORD dwStyle );
	CWnd* InsertControl(CWnd* pCtrl,CRect& rect,UINT nID);
	
	// call to add drop-down buttons
	BOOL AddDropDownButton(UINT nIDButton,UINT nIDMenu,BOOL bArrow=TRUE);
	
	// state functions
	void SaveState();
	void RestoreState();

	//Call this function to set the customization, after you have done the changes to the toolbar.
	void SetToolBarInfoForCustomization(const ToolBarInfoArray * pAdditional=NULL);



// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CToolBarEx)
	//}}AFX_VIRTUAL

// Implementation
public:
	BOOL MarkDefaultState();
	void Customize();
	TextOptions GetTextOptions() { return m_eTextOptions;  }
	void SetTextOptions(TextOptions txOptions,BOOL bUpdate=TRUE);
	IconOptions GetIconOptions() { return m_eIconOptions;  }
	void SetIconOptions(IconOptions txOptions,BOOL bUpdate=TRUE);
	BOOL SetDropDownButton(UINT nID,BOOL bArrow);
	BOOL LoadBitmap(LPCTSTR lpszResourceName);
	BOOL LoadBitmap(UINT nIDResource)	{ return LoadBitmap(MAKEINTRESOURCE(nIDResource)); }

	virtual ~CToolBarEx();
//	BOOL		m_bShowDropdownArrowWhenVertical;		// true to display a drop arrow when docked vertical
	BOOL		m_bHideChildWndOnVertical;	// Hide the windows when docked vertical			




	// Generated message map functions
protected:
	BOOL ShowContextMenu(CPoint pt);
	UINT m_nResButtons;
	BOOL  SetIconOptionBitmap(IconOptions eIconOptions);
	virtual BOOL HasButtonText(int nID);
    static CCustomizeDialog*    m_pCustomizeDlg;
    static HHOOK                m_hCBTHook;

	TextOptions  m_eTextOptions;
	IconOptions  m_eIconOptions;
	BOOL IsVertDocked();
	void PositionControls();
	CObList*			m_pControls;			// CObject pointer list which contains controls added to the toolbar
	CDropDownButtonInfo*		m_pDropButtons;			// list of dropdown button/menu pairs
	static int			m_nBarNumber;			// used for customization, holds bar number for restoration.
	CString				m_strSubKey;			// used for customization, holds registry subkey name.
	CString				m_strValueName;			// used for customization, holds registry value.
	ToolBarInfoArray   m_ToolBarInfo;
	COLORREF m_clrBtnHilight;
	COLORREF m_clrBtnShadow;
	COLORREF m_clrBtnFace;



    static LRESULT CALLBACK CBTProc( int nCode, WPARAM wParam, LPARAM lParam );

		// Find buttons structure for given ID
	CDropDownButtonInfo * FindDropDownButtonInfo(UINT nID);

	BOOL OnToolBarBtnDropDown(NMHDR* pNMHDR, LRESULT* pRes);
	BOOL OnDropDownButtonInfo(const NMTOOLBAR& nmtb, UINT nID, CRect rc);



	//Chenged Functions
	CSize GetButtonSize(TBBUTTON* pData, int iButton,DWORD dwMode);
	CSize CalcSize(TBBUTTON* pData, int nCount,DWORD dwMode);
	int WrapToolBar(TBBUTTON* pData, int nCount, int nWidth,DWORD dwMode);

	//Copied Functions
	void SizeToolBar(TBBUTTON* pData, int nCount, int nLength, BOOL bVert, DWORD dwMode);
	CSize CalcLayout(DWORD dwMode, int nLength=-1);
	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	virtual CSize CalcDynamicLayout(int nLength, DWORD dwMode);
	void _GetButton(int nIndex, TBBUTTON* pButton) const;
	void _SetButton(int nIndex, TBBUTTON* pButton);
	void Draw3DBorders(CDC * pDC, CRect & rect) ;


	virtual BOOL OnChildNotify( UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult )
	{


		if (message == WM_COMMAND)
		{

		}

		return CToolBar::OnChildNotify( message, wParam, lParam, pLResult );

	}
		   


	//{{AFX_MSG(CToolBarEx)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnSysColorChange();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnNcPaint();
	//}}AFX_MSG
	afx_msg void OnToolBarQueryDelete(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnToolBarQueryInsert(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnToolBarChange(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnToolBarBeginAdjust(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnToolBarCustomHelp(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnToolBarEndAdjust(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnToolBarGetButtonInfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnToolBarReset(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg LRESULT OnCustomize(WPARAM,LPARAM) ;
    afx_msg void OnInitCustomize( NMHDR* pNMHDR, LRESULT* pResult );
    afx_msg void OnCustomDraw( NMHDR* pNMHDR, LRESULT* pResult );



	    // Saving and restoring toolbar
    afx_msg void OnSave( NMHDR* pNMHDR, LRESULT* pResult );
    afx_msg void OnRestore( NMHDR* pNMHDR, LRESULT* pResult );


	DECLARE_MESSAGE_MAP()
	DECLARE_DYNCREATE(CToolBarEx)  


};


// CDockBarEx Class

class  CDockBarEx : public CDockBar
{
	DECLARE_DYNAMIC(CDockBarEx)

public:

	// Default constructor
	//
	CDockBarEx();

	// Virtual destructor
	//
	virtual ~CDockBarEx();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDockBarEx)
	public:
	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	//}}AFX_VIRTUAL

// Generated message map functions
protected:
	//{{AFX_MSG(CDockBarEx)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

void FrameEnableDocking(CFrameWnd * pFrame, DWORD dwDockStyle);

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(_TOOLBAREX_H_)

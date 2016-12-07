#include "StdAfx.h"
#include "AnchorCtrl.h"

CAnchorCtrl::CAnchorCtrl(void)
{
	m_pParent = NULL;
}

CAnchorCtrl::~CAnchorCtrl(void)
{
}

void CAnchorCtrl::SetParent(CWnd *pParent)
{
	m_pParent = pParent;
}

void CAnchorCtrl::Fix(CWnd *pCtrl, int nAnchorFlag)
{
	if ( pCtrl==NULL || m_pParent==NULL ) {
		return;
	}

	AnchorStyle stAnchor;
	AnchorStyle *pAnchor = &stAnchor;

	//先查找是否已经存在过了
	for ( list<AnchorStyle>::iterator i = m_ctrls.begin(); i !=m_ctrls.end() ; ++i ) {
		if ( i->m_pCtrl==pCtrl ) {
			i = m_ctrls.erase(i);
			break;
		}
	}//endfor

	RECT rcWnd, rcCtrl;
	m_pParent->GetClientRect(&rcWnd);
	pCtrl->GetWindowRect(&rcCtrl);
	m_pParent->ScreenToClient(&rcCtrl);
	stAnchor.m_nMarginLeft = rcCtrl.left - rcWnd.left;
	stAnchor.m_nMarginTop = rcCtrl.top - rcWnd.top;
	stAnchor.m_nMarginRight = rcCtrl.right - rcWnd.right;
	stAnchor.m_nMarginBottom = rcCtrl.bottom - rcWnd.bottom;
	stAnchor.m_nWidth = rcCtrl.right - rcCtrl.left;
	stAnchor.m_nHeight = rcCtrl.bottom - rcCtrl.top;
	stAnchor.m_pCtrl = pCtrl;
	stAnchor.m_dwAnchorFlag = nAnchorFlag;
	m_ctrls.push_back(stAnchor);
}

void CAnchorCtrl::Fix(int nIDC, int nAnchorFlag)
{
	if ( m_pParent!=NULL ) {
		Fix(m_pParent->GetDlgItem(nIDC), nAnchorFlag);
	}
}

void CAnchorCtrl::Fix(HWND hWnd, int nAnchorFlag)
{
	return Fix(CWnd::FromHandle(hWnd), nAnchorFlag);
}

void CAnchorCtrl::FixAll(int nAnchorFlag)
{
	if ( m_pParent==NULL ) {
		return;
	}

	CWnd *pCtrl = m_pParent->GetTopWindow();
	while ( pCtrl ) {
		Fix(pCtrl, nAnchorFlag);
		pCtrl = pCtrl->GetNextWindow(GW_HWNDNEXT);
	}
}

void CAnchorCtrl::Resize()
{
	if ( m_pParent==NULL || IsWindow(m_pParent->GetSafeHwnd())==FALSE ) {
		return;
	}

	CWnd *pCtrl = NULL;
	DWORD dwFlag = 0;
	RECT rcWnd, rcCtrl;
	int nWidth = 0;
	int nHeight = 0;
	m_pParent->GetClientRect(&rcWnd);

	for ( list<AnchorStyle>::iterator i = m_ctrls.begin(); i !=m_ctrls.end() ; ++i ) {
		dwFlag = i->m_dwAnchorFlag;
		if ( dwFlag & Left ) {
			rcCtrl.left = rcWnd.left + i->m_nMarginLeft;
			if ( dwFlag & Right ) {
				rcCtrl.right = rcWnd.right + i->m_nMarginRight;
			}else{
				rcCtrl.right = rcCtrl.left + i->m_nWidth;
			}
		}else{
			if ( dwFlag & Right ) {
				rcCtrl.right = rcWnd.right + i->m_nMarginRight;
			}
			rcCtrl.left = rcCtrl.right - i->m_nWidth;
		}

		if ( dwFlag & Top ) {
			rcCtrl.top = rcWnd.top + i->m_nMarginTop;
			if ( dwFlag & Bottom ) {
				rcCtrl.bottom = rcWnd.bottom + i->m_nMarginBottom;
			}else{
				rcCtrl.bottom = rcCtrl.top + i->m_nHeight;
			}
		}else{
			if ( dwFlag & Bottom ) {
				rcCtrl.bottom = rcWnd.bottom + i->m_nMarginBottom;
			}
			rcCtrl.top = rcCtrl.bottom - i->m_nHeight;
		}

		i->m_pCtrl->MoveWindow(&rcCtrl, TRUE);
	}//endfor
}
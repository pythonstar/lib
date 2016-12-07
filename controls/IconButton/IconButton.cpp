// IconButton.cpp : 实现文件
//

#include "stdafx.h" 
#include "IconButton.h"


// CIconButton


IMPLEMENT_DYNAMIC(CIconButton, CButton)


CIconButton::CIconButton()
{
	m_bAutoSize = TRUE;
	m_bMouseOver = FALSE;
	m_bIsFlat = TRUE;
}

CIconButton::~CIconButton()
{
}

void CIconButton::SetIcon(int nIcon, int cx, int cy)
{
	m_hIcon = (HICON)::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(nIcon), IMAGE_ICON, cx, cy, 0);
	m_nImageWidth = cx;
	m_nImageHeight = cy;

	int iHeight = 16; 
	CDC *pdc = GetDC();
	if ( pdc ){
		TEXTMETRIC tm={};
		pdc->GetTextMetrics(&tm);
		iHeight = tm.tmHeight + tm.tmExternalLeading; 
	}
	iHeight+=4;
	//iHeight*=2;
	::SetWindowPos(m_hWnd,NULL,0,0,cy+iHeight,cy+iHeight,SWP_NOZORDER|SWP_NOMOVE);
}


void CIconButton::SetAutoSize(BOOL bAutoSize)
{
	m_bAutoSize = bAutoSize;
}


BEGIN_MESSAGE_MAP(CIconButton, CButton)
	ON_WM_DRAWITEM()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_MOUSELEAVE,&CIconButton::OnMouseLeave)
	ON_MESSAGE(WM_MOUSEHOVER,&CIconButton::OnMouseHover)
END_MESSAGE_MAP()



// CIconButton 消息处理程序

BOOL CIconButton::PreCreateWindow(CREATESTRUCT& cs)
{
	if ( m_bAutoSize==TRUE ){
		cs.cx = m_nImageWidth;
		cs.cy = m_nImageHeight;
	}
	cs.style|=BS_OWNERDRAW;
	return CButton::PreCreateWindow(cs);
}

void CIconButton::PreSubclassWindow( )
{
	CButton::PreSubclassWindow();
	ModifyStyle(0,BS_OWNERDRAW);
}


void CIconButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CString strText;
	CDC     dc;
	CRect   rect;

	dc.Attach(lpDrawItemStruct->hDC);
	rect=lpDrawItemStruct->rcItem;
	m_bIsPressed = (lpDrawItemStruct->itemState & ODS_SELECTED);
	m_bIsDisabled = (lpDrawItemStruct->itemState & ODS_DISABLED);

	// Draw button border
	if ( m_bMouseOver ){
		OnDrawBorder(&dc, &rect);
	}

	CPoint pt;
	pt.x = ( rect.Width() - m_nImageWidth )/2;
	pt.y = 0;
	dc.DrawState(pt,rect.Size(),m_hIcon,(IsWindowEnabled() ? DSS_NORMAL : DSS_DISABLED),(CBrush*)NULL);

	//画按钮文本
	this->GetWindowText(strText);
	if ( strText.IsEmpty()==FALSE ){
		dc.SetBkMode(TRANSPARENT);
		if (this->IsWindowEnabled()){
			dc.SetTextColor(RGB(0,0,0));
		}else{
			//CPen pen(GrayString)
			dc.SetTextColor(RGB(120,127,127));
		}
		rect.top += m_nImageHeight+2;
		dc.DrawText(strText, strText.GetLength(),&rect,DT_SINGLELINE|DT_VCENTER|DT_CENTER);
	}
	
	dc.Detach();
}

DWORD CIconButton::OnDrawBorder(CDC* pDC, CRect* pRect)
{
	// Draw pressed button
	if (m_bIsPressed)
	{
		if (m_bIsFlat)
		{
			if (m_bDrawBorder)
				pDC->Draw3dRect(pRect, ::GetSysColor(COLOR_BTNSHADOW), ::GetSysColor(COLOR_BTNHILIGHT));
		}
		else    
		{
			CBrush brBtnShadow(GetSysColor(COLOR_BTNSHADOW));
			pDC->FrameRect(pRect, &brBtnShadow);
		}
	}
	else // ...else draw non pressed button
	{
		CPen penBtnHiLight(PS_SOLID, 0, GetSysColor(COLOR_BTNHILIGHT)); // White
		CPen pen3DLight(PS_SOLID, 0, GetSysColor(COLOR_3DLIGHT));       // Light gray
		CPen penBtnShadow(PS_SOLID, 0, GetSysColor(COLOR_BTNSHADOW));   // Dark gray
		CPen pen3DDKShadow(PS_SOLID, 0, GetSysColor(COLOR_3DDKSHADOW)); // Black

		if (m_bIsFlat)
		{
			if (m_bMouseOver)
				pDC->Draw3dRect(pRect, ::GetSysColor(COLOR_BTNHILIGHT), ::GetSysColor(COLOR_BTNSHADOW));
		}
		else
		{
			// Draw top-left borders
			// White line
			CPen* pOldPen = pDC->SelectObject(&penBtnHiLight);
			pDC->MoveTo(pRect->left, pRect->bottom-1);
			pDC->LineTo(pRect->left, pRect->top);
			pDC->LineTo(pRect->right, pRect->top);
			// Light gray line
			pDC->SelectObject(pen3DLight);
			pDC->MoveTo(pRect->left+1, pRect->bottom-1);
			pDC->LineTo(pRect->left+1, pRect->top+1);
			pDC->LineTo(pRect->right, pRect->top+1);
			// Draw bottom-right borders
			// Black line
			pDC->SelectObject(pen3DDKShadow);
			pDC->MoveTo(pRect->left, pRect->bottom-1);
			pDC->LineTo(pRect->right-1, pRect->bottom-1);
			pDC->LineTo(pRect->right-1, pRect->top-1);
			// Dark gray line
			pDC->SelectObject(penBtnShadow);
			pDC->MoveTo(pRect->left+1, pRect->bottom-2);
			pDC->LineTo(pRect->right-2, pRect->bottom-2);
			pDC->LineTo(pRect->right-2, pRect->top);
			//
			pDC->SelectObject(pOldPen);
		} // else
	} // else

	return 0;
} // End of OnDrawBorder

void CIconButton::OnMouseMove(UINT nFlags, CPoint point)
{//鼠标在按钮上移动，则置m_bMouseOver标志位（为FALSE时置位）
	if (!m_bMouseOver)
	{
		m_bMouseOver=TRUE;
		this->Invalidate();
		TRACKMOUSEEVENT tme;
		tme.cbSize=sizeof(tme);
		tme.dwFlags=TME_HOVER|TME_LEAVE;
		tme.hwndTrack=this->m_hWnd;
		tme.dwHoverTime=HOVER_DEFAULT;
		::TrackMouseEvent(&tme);
	}
}

LRESULT CIconButton::OnMouseLeave(WPARAM wParam,LPARAM lParam)
{//鼠标移走，绘画Default图像
	m_bMouseOver=FALSE;
	this->Invalidate();
	return 0;
}

LRESULT CIconButton::OnMouseHover(WPARAM wParam,LPARAM lParam)
{
	TRACKMOUSEEVENT tme;
	tme.cbSize=sizeof(tme);
	tme.dwFlags=TME_HOVER|TME_LEAVE;
	tme.hwndTrack=this->m_hWnd;
	tme.dwHoverTime=HOVER_DEFAULT;
	::TrackMouseEvent(&tme);
	return 0;
}

BOOL CIconButton::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
}


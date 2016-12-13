// enedit.cpp : implementation file
//

#include "stdafx.h"
#include "enedit.h"
#include "themed.h"
#include "dlgunits.h"
#include "graphicsmisc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEnEdit

int MENUSIZE = -1;

CEnEdit::CEnEdit(BOOL bComboStyle, LPCTSTR szMask, DWORD dwFlags) : 
					CMaskEdit(szMask, dwFlags),
					m_bComboStyle(bComboStyle),
					m_bFirstShow(TRUE), 
					m_nButtonDown(-1),
					m_nBottomBorder(0),
					m_nTopBorder(0)
{
}

CEnEdit::~CEnEdit()
{
	// cleanup fonts
	int nBtn = m_aButtons.GetSize();
	
	while (nBtn--)
		::DeleteObject(m_aButtons[nBtn].hFont);
}


BEGIN_MESSAGE_MAP(CEnEdit, CMaskEdit)
	//{{AFX_MSG_MAP(CEnEdit)
	ON_WM_NCCALCSIZE()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
	ON_WM_NCPAINT()
	ON_WM_CREATE()
	ON_WM_NCHITTEST()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_REGISTERED_MESSAGE(WM_HTHOTCHANGE, OnHotChange)
	ON_WM_ENABLE()
	ON_MESSAGE(EM_SETREADONLY, OnSetReadOnly)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEnEdit message handlers

BOOL CEnEdit::AddButton(UINT nID, LPCTSTR szCaption, LPCTSTR szTip, int nWidth, LPCTSTR szFont)
{
	return InsertButton(m_aButtons.GetSize(), nID, szCaption, szTip, nWidth, szFont, FALSE);
}

BOOL CEnEdit::AddButton(UINT nID, UINT nChar, LPCTSTR szTip, int nWidth, LPCTSTR szFont)
{
	// Valik - cast to TCHAR is necessary or the compiler complains under VC 7.1
	return InsertButton(m_aButtons.GetSize(), nID, CString(static_cast<TCHAR>(nChar)), szTip, nWidth, szFont, TRUE);
}

BOOL CEnEdit::InsertButton(int nPos, UINT nID, LPCTSTR szCaption, LPCTSTR szTip, int nWidth, LPCTSTR szFont)
{
	return InsertButton(nPos, nID, szCaption, szTip, nWidth, szFont, FALSE);
}

BOOL CEnEdit::InsertButton(int nPos, UINT nID, UINT nChar, LPCTSTR szTip, int nWidth, LPCTSTR szFont)
{
	// Valik - cast to TCHAR is necessary or the compiler complains under VC 7.1
	return InsertButton(nPos, nID, CString(static_cast<TCHAR>(nChar)), szTip, nWidth, szFont, TRUE);
}

BOOL CEnEdit::InsertButton(int nPos, UINT nID, LPCTSTR szCaption, LPCTSTR szTip, int nWidth, LPCTSTR szFont, BOOL bSymbolFont)
{
	if (nWidth < CALC_BTNWIDTH || !nID)
		return FALSE;

	nPos = max(nPos, 0);
	nPos = min(nPos, m_aButtons.GetSize());

	EDITBTN eb;

	eb.nID = nID;
	eb.sCaption = szCaption;
	eb.sTip = szTip;
	eb.nWidth = nWidth;
	eb.hFont = NULL;
	eb.bChecked = FALSE;
	eb.bEnabled = TRUE;
	eb.bDropMenu = FALSE;

	if (szFont)
	{
		eb.hFont = GraphicsMisc::CreateFont(szFont, -1, bSymbolFont ? MFS_SYMBOL : 0);
		eb.bSymbol = bSymbolFont;
	
		if (!eb.hFont)
			return FALSE;
	}
	else
		eb.bSymbol = FALSE;

	m_aButtons.InsertAt(nPos, eb);

	// add rect to hot tracker
	m_hotTrack.AddRect(CRect(0, 0, 0, 0));

	if (GetSafeHwnd())
	{
		if (!eb.sTip.IsEmpty())
			m_tooltip.AddTool(this, eb.sTip, CRect(0, 0, 10, 10), nID);

		RecalcBtnRects();

		// force WM_NCCALCSIZE
		if (!m_bFirstShow)
			SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER); 
	}

	return TRUE;
}

void CEnEdit::SetBorders(int nTop, int nBottom)
{
	nTop = max(nTop, 0);
	nBottom = max(nBottom, 0);

	if (m_nTopBorder != nTop || m_nBottomBorder != nBottom)
	{
		m_nBottomBorder = nBottom;
		m_nTopBorder = nTop;

		// force WM_NCCALCSIZE
		if (GetSafeHwnd())
			SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER); 
	}
}

void CEnEdit::PreSubclassWindow() 
{
	CMaskEdit::PreSubclassWindow();

}

BOOL CEnEdit::PreTranslateMessage(MSG* pMsg) 
{
   if (m_tooltip.GetSafeHwnd())
	   m_tooltip.RelayEvent(pMsg);
	
	return CMaskEdit::PreTranslateMessage(pMsg);
}

void CEnEdit::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	if (!(GetStyle() & ES_MULTILINE))
	{
		if (bCalcValidRects)
		{
			m_bFirstShow = FALSE; // in case we get here before OnNcPaint()
		
			int nBtn = m_aButtons.GetSize();

			while (nBtn--)
				lpncsp->rgrc[0].right -= GetButtonWidth(nBtn);

			lpncsp->rgrc[0].top += m_nTopBorder;
			lpncsp->rgrc[0].bottom -= m_nBottomBorder;
		}
	}
	
	CMaskEdit::OnNcCalcSize(bCalcValidRects, lpncsp);
}

void CEnEdit::OnNcLButtonDown(UINT nHitTest, CPoint point) 
{
	CMaskEdit::OnNcLButtonDown(nHitTest, point);

	if (IsWindowEnabled())
	{
		int nBtn = ButtonHitTest(point);

		if (nBtn >= 0)
		{
			if (m_aButtons[nBtn].bEnabled)
			{
				SetCapture();
				m_nButtonDown = nBtn;
				
				SendMessage(WM_NCPAINT);
			}
		}
		else
			SetFocus();
	}
}

void CEnEdit::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CMaskEdit::OnLButtonUp(nFlags, point);

	if (m_nButtonDown == -1)
		return;

	ClientToScreen(&point);
	int nBtnDown = m_nButtonDown;
	int nBtnUp = ButtonHitTest(point);

	// update UI
	ReleaseCapture();
	m_nButtonDown = -1;
	
	SendMessage(WM_NCPAINT);

	// process
	if (nBtnDown == nBtnUp)
	{
		// call derived class first
		OnBtnClick(m_aButtons[nBtnUp].nID);

		// then parent
		GetParent()->SendMessage(WM_EE_BTNCLICK, GetDlgCtrlID(), m_aButtons[nBtnUp].nID);
	}
	
	SendMessage(WM_NCPAINT);
}

void CEnEdit::OnMouseMove(UINT nFlags, CPoint point) 
{
	CMaskEdit::OnMouseMove(nFlags, point);

	if (m_nButtonDown != -1)
	{
		ClientToScreen(&point);

		if (ButtonHitTest(point) != m_nButtonDown)
		{
			ReleaseCapture();
			m_nButtonDown = -1;
			SendMessage(WM_NCPAINT);
		}
	}
}

void CEnEdit::OnSize(UINT nType, int cx, int cy) 
{
	CMaskEdit::OnSize(nType, cx, cy);
	
	// create tooltip
	if (!m_tooltip.GetSafeHwnd() && m_tooltip.Create(this))
	{
		// add any existing buttons
		int nBtn = m_aButtons.GetSize();

		while (nBtn--)
		{
			const EDITBTN& eb = m_aButtons[nBtn];
			m_tooltip.AddTool(this, eb.sTip, CRect(), eb.nID);
		}

		// hot tracking
		if (CThemed().AreControlsThemed())
			m_hotTrack.Initialize(this);
	}

	// update tool rects
	RecalcBtnRects();
}

void CEnEdit::RecalcBtnRects()
{
	if (!m_tooltip.GetSafeHwnd())
		return;
	
	int nBtn = m_aButtons.GetSize();
	
	while (nBtn--)
	{
		CRect rBtn = GetButtonRectByIndex(nBtn);
		ScreenToClient(rBtn);

		m_tooltip.SetToolRect(this, m_aButtons[nBtn].nID, rBtn);

		// update hottracker too
		m_hotTrack.UpdateRect(nBtn, rBtn);
	}
}

CRect CEnEdit::GetButtonRectByIndex(int nBtn) const
{
	int nOffset = 0;

	for (int nIndex = 0; nIndex < nBtn; nIndex++)
		nOffset += GetButtonWidth(nIndex);

	CRect rButton;
	GetClientRect(rButton);

	rButton.left = rButton.right + nOffset;
	rButton.right += nOffset + GetButtonWidth(nBtn);
	rButton.top -= m_nTopBorder;
	rButton.bottom += m_nBottomBorder;

	ClientToScreen(rButton);

	return rButton;
}

void CEnEdit::OnNcPaint() 
{
	if (m_bFirstShow)
	{
		m_bFirstShow = FALSE;
		SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW); 
	}

	Default();

	// draw the icon and browse button
	CWindowDC dc(this);

	CRect rWindow;
	GetWindowRect(rWindow);

	NcPaint(&dc, rWindow);
}

void CEnEdit::NcPaint(CDC* pDC, const CRect& rWindow)
{
	CPoint ptCursor(::GetMessagePos());
	int nBtn = m_aButtons.GetSize();
	
	while (nBtn--)
		DrawButton(pDC, rWindow, nBtn, ptCursor);
}

int CEnEdit::ButtonHitTest(CPoint ptScreen) const
{
	int nBtn = m_aButtons.GetSize();
	
	while (nBtn--)
	{
		if (GetButtonRectByIndex(nBtn).PtInRect(ptScreen))
			return nBtn;
	}

	return -1;
}

int CEnEdit::ButtonHitTest(UINT nID) const
{
	int nBtn = m_aButtons.GetSize();
	
	while (nBtn--)
	{
		if (m_aButtons[nBtn].nID == nID)
			return nBtn;
	}

	return -1;
}

BOOL CEnEdit::EnableButton(UINT nID, BOOL bEnable)
{
	int nBtn = ButtonHitTest(nID);

	if (nBtn < 0)
		return FALSE;

	if (m_aButtons[nBtn].bEnabled != bEnable)
	{
		m_aButtons[nBtn].bEnabled = bEnable;
		SendMessage(WM_NCPAINT);
	}

	return TRUE;
}

BOOL CEnEdit::SetDropMenuButton(UINT nID, BOOL bDropMenu)
{
	int nBtn = ButtonHitTest(nID);

	if (nBtn < 0)
		return FALSE;

	if (m_aButtons[nBtn].bDropMenu != bDropMenu)
	{
		m_aButtons[nBtn].bDropMenu = bDropMenu;
	
		if (GetSafeHwnd())
			SendMessage(WM_NCPAINT);
	}

	return TRUE;
}

BOOL CEnEdit::CheckButton(UINT nID, BOOL bChecked)
{
	int nBtn = ButtonHitTest(nID);

	if (nBtn < 0)
		return FALSE;

	if (m_aButtons[nBtn].bChecked != bChecked)
	{
		m_aButtons[nBtn].bChecked = bChecked;
		SendMessage(WM_NCPAINT);
	}

	return TRUE;
}

BOOL CEnEdit::SetButtonTip(UINT nID, LPCTSTR szTip)
{
	int nBtn = ButtonHitTest(nID);

	if (nBtn < 0)
		return FALSE;

	if (m_aButtons[nBtn].sTip.Compare(szTip) != 0)
	{
		m_aButtons[nBtn].sTip = szTip;

		if (GetSafeHwnd())
		{
			m_tooltip.UpdateTipText(szTip, this, m_aButtons[nBtn].nID);
			SendMessage(WM_NCPAINT);
		}
	}

	return TRUE;
}

BOOL CEnEdit::SetButtonCaption(UINT nID, LPCTSTR szCaption)
{
	int nBtn = ButtonHitTest(nID);

	if (nBtn < 0)
		return FALSE;

	EDITBTN& eb = m_aButtons[nBtn];

	if (eb.sCaption.Compare(szCaption) != 0)
	{
		eb.sCaption = szCaption;

		// recalc width?
		if (eb.nWidth == CALC_BTNWIDTH)
			RecalcBtnRects();

		if (GetSafeHwnd())
		{
			// force WM_NCCALCSIZE
			SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER); 
			SendMessage(WM_NCPAINT);
		}
	}

	return TRUE;
}

CRect CEnEdit::GetButtonRect(UINT nID)
{
	int nBtn = ButtonHitTest(nID);

	if (nBtn >= 0)
		return GetButtonRectByIndex(nBtn);

	return CRect(0, 0, 0, 0);
}

void CEnEdit::DrawButton(CDC* pDC, const CRect& rWindow, int nBtn, const CPoint& ptCursor) const
{
	const EDITBTN& eb = m_aButtons[nBtn];

	CRect rBtn = GetButtonRectByIndex(nBtn);

	if (rBtn.IsRectEmpty())
		return;

	BOOL bThemed = CThemed().AreControlsThemed();
	BOOL bHot = rBtn.PtInRect(ptCursor);
	BOOL bEnabled = (IsWindowEnabled() && eb.bEnabled);
	BOOL bDown = (m_nButtonDown == nBtn || eb.bChecked);

	rBtn.OffsetRect(-rWindow.TopLeft());

	// nasty business here because the API function DrawThemeEdge() is not theme aware!
	// and drawing a themed combostyle button will also draw the arrow which we don't want
	if (!m_bComboStyle || bThemed)	// draw as button type (for now)
	{
		UINT nFlags = DFCS_ADJUSTRECT | DFCS_BUTTONPUSH;
		
		// note: we do not take account of ES_READONLY as the effect of this
		// is not deterministic at this level so we assume derived classes or 
		// parents have handled it
		if (!bEnabled)
			nFlags |= DFCS_INACTIVE;
		
		else if (bDown)
			nFlags |= DFCS_PUSHED;
		
		else if (bHot)
			nFlags |= DFCS_HOT;
		
		// clip the drawing rect to prevent window getting the parent bkgnd color wrong
		CRect rClip(rBtn);

		if (bThemed)
			rBtn.InflateRect(1, 1);
		
		// for now
		CThemed::DrawFrameControl(this, pDC, rBtn, DFC_BUTTON, nFlags, rClip);
	}
	else // unthemed combo style
	{
		if (bEnabled && bDown)
			pDC->DrawEdge(rBtn, BDR_RAISEDOUTER, BF_ADJUST | BF_RECT | BF_MIDDLE | BF_FLAT);
		else
		{
			pDC->DrawEdge(rBtn, BDR_RAISEDOUTER, BF_ADJUST | BF_RECT | BF_MIDDLE);
			pDC->DrawEdge(rBtn, BDR_RAISEDINNER, BF_ADJUST | BF_RECT | BF_MIDDLE);
		}
	}

	// drop menu arrow
	if (eb.bDropMenu)
	{
		CRect rArrow(rBtn);
		
		if (bDown)
			rArrow.OffsetRect(1, 1);

		UINT nFlags = DT_END_ELLIPSIS | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX;

		if (eb.sCaption.IsEmpty())
			nFlags |= DT_CENTER;
		else
		{
			nFlags |= DT_RIGHT;
			rArrow.right -= 2;
		}

		GraphicsMisc::DrawSymbol(pDC, 'u', rArrow, nFlags, &GraphicsMisc::Marlett());
	}
	
	CString sCaption(eb.sCaption);

	if (!sCaption.IsEmpty())
	{
		// draw custom caption
		if (m_nButtonDown == nBtn || eb.bChecked)
			rBtn.OffsetRect(1, 1);

		CFont* pOld = NULL;
		
		if (eb.hFont)
			pOld = pDC->SelectObject(CFont::FromHandle(eb.hFont));
		else
		{
			CFont* pFont = GetFont();
			pOld = (CFont*)pDC->SelectObject(pFont);
		}
		
		pDC->SetTextAlign(TA_CENTER | TA_TOP);
		pDC->SetBkMode(TRANSPARENT);
		
		BOOL bEnabled = IsWindowEnabled() && eb.bEnabled;

		int CHAR_HEIGHT = pDC->GetTextExtent("A").cy;

		int nVOffset = (rBtn.Height() - CHAR_HEIGHT + 1) / 2 - 1;
		int nHOffset = (rBtn.Width() + 1) / 2;

		// if the button has a drop menu position the text in the 
		// center of what remains
		if (eb.bDropMenu)
			nHOffset = (rBtn.Width() - MENUSIZE + 4) / 2; 

		CPoint ptText(rBtn.left + nHOffset, rBtn.top + nVOffset);

		if (bEnabled)
		{
			pDC->SetTextColor(GetSysColor(COLOR_BTNTEXT));
			DrawText(pDC, ptText, sCaption, rBtn, eb.bSymbol);
		}
		else
		{
			// draw embossed: dark over pale
			ptText.Offset(1, 1);
			pDC->SetTextColor(GetSysColor(COLOR_3DHIGHLIGHT));
			DrawText(pDC, ptText, sCaption, rBtn, eb.bSymbol);

			ptText.Offset(-1, -1);
			pDC->SetTextColor(GetSysColor(COLOR_3DSHADOW));
			DrawText(pDC, ptText, sCaption, rBtn, eb.bSymbol);
		}

		// cleanup
		if (pOld)
			pDC->SelectObject(pOld);
	}
}

void CEnEdit::DrawText(CDC* pDC, const CPoint& ptTopLeft, const CString& sText, const CRect& rect, BOOL bSymbol)
{
	if (bSymbol)
	{
		CRect rSymbol(rect);
		rSymbol.left = ptTopLeft.x + 3;
		rSymbol.top = ptTopLeft.y;

		GraphicsMisc::DrawSymbol(pDC, (char)sText[0], rSymbol, DT_CENTER | DT_TOP | DT_NOCLIP);
	}
	else
		pDC->ExtTextOut(ptTopLeft.x, ptTopLeft.y, 0, rect, sText, NULL);
}

#if _MSC_VER >= 1400
LRESULT CEnEdit::OnNcHitTest(CPoint point)
#else
UINT CEnEdit::OnNcHitTest(CPoint point)
#endif
{
	int nBtn = ButtonHitTest(point);

	if (nBtn >= 0)
		return HTBORDER;

	return (LRESULT)CMaskEdit::OnNcHitTest(point);
}

LRESULT CEnEdit::OnHotChange(WPARAM wp, LPARAM lp)
{
	LRESULT lr = Default();

	// wp has prev hot rect index
	// lp has new hot rect index
	ASSERT (((int)wp != -1 || (int)lp != -1) && (int)wp != (int)lp);

	CWindowDC dc(this);
	CPoint ptCursor(::GetMessagePos());

	CRect rWindow;
	GetWindowRect(rWindow);

	if ((int)wp != -1 && m_aButtons[wp].bEnabled)
		DrawButton(&dc, rWindow, wp, ptCursor);

	if ((int)lp != -1 && m_aButtons[lp].bEnabled)
		DrawButton(&dc, rWindow, lp, ptCursor);

	return lr;
}

BOOL CEnEdit::SetButtonWidth(UINT nID, int nWidth)
{
	int nBtn = ButtonHitTest(nID);

	if (nBtn < 0)
		return FALSE;

	if (m_aButtons[nBtn].nWidth != nWidth)
	{
		m_aButtons[nBtn].nWidth = nWidth;

		if (GetSafeHwnd())
		{
			// force WM_NCCALCSIZE
			SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER); 

			RecalcBtnRects();
			SendMessage(WM_NCPAINT);
		}
	}

	return TRUE;
}

int CEnEdit::GetButtonWidth(int nBtn) const
{
	if (nBtn < 0 || nBtn >= m_aButtons.GetSize() || !GetSafeHwnd())
		return 0;

	const EDITBTN& eb = m_aButtons[nBtn];

	if (eb.nWidth > 0)
		return eb.nWidth;
	
	// else calc
	CWindowDC dc(GetParent());
	CFont* pFont = CFont::FromHandle(eb.hFont ? eb.hFont : (HFONT)GetStockObject(DEFAULT_GUI_FONT));
	CFont* pOldFont = dc.SelectObject(pFont);

	int nWidth = dc.GetTextExtent(eb.sCaption).cx + 6;
	dc.SelectObject(pOldFont);

	if (eb.bDropMenu)
	{
		if (MENUSIZE == -1)
		{
			pOldFont = dc.SelectObject(&GraphicsMisc::Marlett());
			MENUSIZE = dc.GetTextExtent("u").cx - 1;
			dc.SelectObject(pOldFont);
		}

		nWidth += MENUSIZE;
	}

	return nWidth;
}

BOOL CEnEdit::SetButtonWidthDLU(UINT nID, int nDLU)
{
	ASSERT (GetSafeHwnd());

	int nWidth = CDlgUnits(*GetTopLevelParent()).ToPixelsX(nDLU);
	return SetButtonWidth(nID, nWidth);
}

LRESULT CEnEdit::OnSetReadOnly(WPARAM wp, LPARAM /*lp*/)
{
	LRESULT lr = Default();

	OnSetReadOnly((BOOL)wp);
	SendMessage(WM_NCPAINT);

	return lr;
}

void CEnEdit::OnEnable(BOOL bEnable) 
{
	CEdit::OnEnable(bEnable);
	
	SendMessage(WM_NCPAINT);	
}




int CEnEdit::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMaskEdit::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	return 0;
}

void CEnEdit::OnDestroy() 
{
	CMaskEdit::OnDestroy();
	
	m_tooltip.DestroyWindow();
}

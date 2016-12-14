// SundyButton.cpp : ʵ���ļ�
//

#include "stdafx.h" 
#include "SundyButton.h"


// CSundyButton

IMPLEMENT_DYNAMIC(CSundyButton, CButton)

CSundyButton::CSundyButton()
{
	bDown=false;
	m_bMouseOver=false;
}

CSundyButton::~CSundyButton()
{
}

void CSundyButton::SetBitmap(DWORD dwDefaultBmp,DWORD dwHoverBmp,DWORD dwDownBmp)
{
	m_dwDefaultBmp=dwDefaultBmp;
	m_dwHoverBmp=dwHoverBmp;
	m_dwDownBmp=dwDownBmp;

	bmpDefault.LoadBitmap(dwDefaultBmp);
	bmpHover.LoadBitmap(dwHoverBmp);
	bmpDown.LoadBitmap(dwDownBmp);
	mdcDefault.CreateCompatibleDC(NULL);
	mdcHover.CreateCompatibleDC(NULL);
	mdcDown.CreateCompatibleDC(NULL);
	mdcDefault.SelectObject(&bmpDefault);
	mdcHover.SelectObject(&bmpHover);
	mdcDown.SelectObject(&bmpDown);
}



BEGIN_MESSAGE_MAP(CSundyButton, CButton)
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_MOUSELEAVE,&CSundyButton::OnMouseLeave)
	ON_MESSAGE(WM_MOUSEHOVER,&CSundyButton::OnMouseHover)
END_MESSAGE_MAP()



// CSundyButton ��Ϣ�������

BOOL CSundyButton::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style|=BS_OWNERDRAW;
	return CButton::PreCreateWindow(cs);
}

void CSundyButton::PreSubclassWindow( )
{
	BITMAP Bmp;
	CRect rect;

	CButton::PreSubclassWindow();
	ModifyStyle(BS_PUSHBUTTON,BS_OWNERDRAW);

	bmpDefault.GetBitmap(&Bmp);
	GetWindowRect(&rect);
	//ScreenToClient(&rect);
	rect.left-=GetSystemMetrics(SM_CXBORDER);
	rect.top-=GetSystemMetrics(SM_CYBORDER)+GetSystemMetrics(SM_CYCAPTION);
	rect.right=rect.left+Bmp.bmWidth;
	rect.bottom=rect.top+Bmp.bmHeight;
	//MoveWindow(&rect,true);
}


void CSundyButton::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	//CRect rect;

	//GetClientRect(&rect);
	//lpMeasureItemStruct->itemWidth=rect.Width();
	//lpMeasureItemStruct->itemHeight=rect.Height();
}

void CSundyButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CString strText;
	CDC     dc;
	CRect   rect;

	dc.Attach(lpDrawItemStruct->hDC);
	rect=lpDrawItemStruct->rcItem;

	if (m_bMouseOver&&!bDown)
	{//������ڰ�ť�Ϸ���û�б�����
		dc.BitBlt(rect.left,rect.top,rect.Width(),rect.Height(),&mdcHover,0,0,SRCCOPY);
	}
	if (m_bMouseOver&&bDown)
	{//������ڰ�ť�Ϸ��ұ�����
		dc.BitBlt(rect.left,rect.top,rect.Width(),rect.Height(),&mdcDown,0,0,SRCCOPY);
	}
	else if(!m_bMouseOver)
	{//��겻�ڰ�ť�����ڲ�
		dc.BitBlt(rect.left,rect.top,rect.Width(),rect.Height(),&mdcDefault,0,0,SRCCOPY);
	}
	//����ť�ı�
	this->GetWindowText(strText);
	dc.SetBkMode(TRANSPARENT);
	if (this->IsWindowEnabled())
		dc.SetTextColor(RGB(0,0,0));
	else
	{
		//CPen pen(GrayString)
		dc.SetTextColor(RGB(120,127,127));
	}
	dc.DrawText(strText, strText.GetLength(),&rect,DT_SINGLELINE|DT_VCENTER|DT_CENTER);

	dc.Detach();
}

void CSundyButton::OnLButtonDown(UINT nFlags,CPoint point)
{//��ť���£����ñ�־λ���滭��ť����ʱ��ͼ��
	CButton::OnNcLButtonDown(nFlags,point);
	bDown=true;
	this->Invalidate();
}

void CSundyButton::OnLButtonUp(UINT nFlags,CPoint point)
{//��ţ̌�����ñ�־λ���滭��ţ̌��ʱ��ͼ��
	CButton::OnNcLButtonUp(nFlags,point);
	bDown=false;
	this->Invalidate();
}

void CSundyButton::OnMouseMove(UINT nFlags, CPoint point)
{//����ڰ�ť���ƶ�������m_bMouseOver��־λ��ΪFALSEʱ��λ��
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

LRESULT CSundyButton::OnMouseLeave(WPARAM wParam,LPARAM lParam)
{//������ߣ��滭Defaultͼ��
	m_bMouseOver=FALSE;
	bDown=FALSE;
	this->Invalidate();
	return 0;
}

LRESULT CSundyButton::OnMouseHover(WPARAM wParam,LPARAM lParam)
{
	TRACKMOUSEEVENT tme;
	tme.cbSize=sizeof(tme);
	tme.dwFlags=TME_HOVER|TME_LEAVE;
	tme.hwndTrack=this->m_hWnd;
	tme.dwHoverTime=HOVER_DEFAULT;
	::TrackMouseEvent(&tme);
	return 0;
}
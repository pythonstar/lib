#pragma once
// CIconButton


class CIconButton : public CButton
{
	DECLARE_DYNAMIC(CIconButton)
public:
	void SetIcon(int nIcon, int cx, int cy);
	void SetAutoSize(BOOL bAutoSize);

protected:
	BOOL m_bMouseOver;

	HICON m_hIcon;

	BOOL m_bAutoSize;

	BOOL		m_bIsFlat;			// Is a flat button?
	BOOL		m_bIsPressed;		// Is button pressed?
	BOOL		m_bIsDisabled;		// Is button disabled?
	BOOL		m_bDrawBorder;		// Is button disabled?
public:
	int m_nImageWidth;
	int m_nImageHeight;
public:
	CIconButton();
	virtual ~CIconButton();

	DWORD OnDrawBorder(CDC* pDC, CRect* pRect);

protected:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnMouseHover(WPARAM wParam,LPARAM lParam);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void PreSubclassWindow( );

protected:
	DECLARE_MESSAGE_MAP()
};



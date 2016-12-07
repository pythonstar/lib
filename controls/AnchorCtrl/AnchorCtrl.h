#pragma once

#include <list>
using namespace std;


class CAnchorCtrl
{
public:
	CAnchorCtrl(void);
	~CAnchorCtrl(void);

public:
	enum {
		Left = 1, Top = 2, Right = 4, Bottom = 8, 
		LeftTop = Left | Top,
		LeftRight = Left | Right,
		LeftBottom = Left | Bottom,
		TopRight = Top | Right,
		TopBottom = Top | Bottom,
		RightBottom = Right | Bottom,
		LeftTopRight = Left | Top | Right,
		LeftTopBottom = Left | Top | Bottom,
		LeftRightBottom = Left | Right | Bottom,
		TopRightBottom = Top | Right | Bottom,
		LeftTopRightBottom = Left | Top | Right | Bottom,
	};

public:
	void SetParent(CWnd *pParent);
	void Fix(CWnd *pCtrl, int nAnchorFlag);
	void Fix(int nIDC, int nAnchorFlag);
	void Fix(HWND hWnd, int nAnchorFlag);
	void FixAll(int nAnchorFlag);

	void Resize();

private:
	CWnd	*m_pParent;

	struct AnchorStyle {
		int		m_nMarginLeft;
		int		m_nMarginTop;
		int		m_nMarginRight;
		int		m_nMarginBottom;
		int		m_nWidth;
		int		m_nHeight;
		CWnd	*m_pCtrl;
		DWORD	m_dwAnchorFlag;
		AnchorStyle(){
			m_nMarginLeft = m_nMarginTop = m_nMarginRight = m_nMarginBottom = 0;
			m_nWidth = m_nHeight = 0;
			m_pCtrl = NULL;
			m_dwAnchorFlag = 0;
		}
	};

	list<AnchorStyle> m_ctrls;
};

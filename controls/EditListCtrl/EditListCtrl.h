#include <afxcmn.h>
#pragma once


#define ORANGE_LVN_ENDEDIT WM_USER + 1000


// CEditListCtrl dialog

enum TYPE_CONLUMN
{
	type_Header,
	type_Image
};

//////////////////////////////////////////////////////////////////////////

class CEditEx :public CEdit
{
public:
	CEditEx(void);
	virtual ~CEditEx(void);
protected:

	void OnKillFocus(CWnd* pNewWnd);

	DECLARE_MESSAGE_MAP()

};

//////////////////////////////////////////////////////////////////////////

class CEditListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CEditListCtrl)

public:
	CEditListCtrl();   // standard constructor
	virtual ~CEditListCtrl();

	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

    void DelCheckedItem();
	void InsertOneItem();

	void GetColumnData(int nColumn,CStringArray & strArryData);


	void SetEditable(int nItem, int nSubItem);

	int SetItemHeight(int nHeight);
private:
	int		m_nItemHeight;


protected:
	int m_nLastItem;
	int m_nItem;
    int m_nSubItem;
	CFont m_FontItem;
	CEditEx m_Edit_ctlItem;

	void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg LRESULT OnEndEdit(WPARAM wParam,LPARAM lParam);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);

	DECLARE_MESSAGE_MAP()
};

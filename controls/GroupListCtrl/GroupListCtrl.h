#pragma once
#include <afxcmn.h>

class CGroupListCtrl : public CListCtrl
{
public:
	CGroupListCtrl()
		:m_SortCol(-1)
		,m_Ascending(false)
	{}

	LRESULT InsertGroupHeader(int nIndex, int nGroupID, const CString& strHeader, DWORD dwState = LVGS_NORMAL, DWORD dwAlign = LVGA_HEADER_LEFT);

	CString GetGroupHeader(int nGroupID);
	int GetRowGroupId(int nRow);
	BOOL SetRowGroupId(int nRow, int nGroupID);
	int GroupHitTest(const CPoint& point);

	BOOL GroupByColumn(int nCol);
	void DeleteEntireGroup(int nGroupId);
	BOOL HasGroupState(int nGroupID, DWORD dwState);
	BOOL SetGroupState(int nGroupID, DWORD dwState);
	BOOL IsGroupStateEnabled();

	void CheckEntireGroup(int nGroupId, bool bChecked);

	bool SortColumn(int columnIndex, bool ascending);

	void CollapseAllGroups();
	void ExpandAllGroups();

	BOOL SetGroupFooter(int nGroupID, const CString& footer, DWORD dwAlign = LVGA_FOOTER_CENTER);
	BOOL SetGroupTask(int nGroupID, const CString& task);
	BOOL SetGroupSubtitle(int nGroupID, const CString& subtitle);
	BOOL SetGroupTitleImage(int nGroupID, int nImage, const CString& topDesc, const CString& bottomDesc);

protected:
	virtual void PreSubclassWindow();

	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg BOOL OnHeaderClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnGroupTaskClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()

	int m_SortCol;
	bool m_Ascending;
};
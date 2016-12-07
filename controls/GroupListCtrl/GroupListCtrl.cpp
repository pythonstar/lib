#include "stdafx.h"
#include "GroupListCtrl.h"

#include <shlwapi.h>

BEGIN_MESSAGE_MAP(CGroupListCtrl, CListCtrl)
	//ON_WM_CONTEXTMENU()	// OnContextMenu
	//ON_WM_LBUTTONDBLCLK()
	//ON_NOTIFY_REFLECT_EX(LVN_COLUMNCLICK, OnHeaderClick)	// Column Click
#if _WIN32_WINNT >= 0x0600
	ON_NOTIFY_REFLECT_EX(LVN_LINKCLICK, OnGroupTaskClick)	// Column Click
#endif
END_MESSAGE_MAP()

LRESULT CGroupListCtrl::InsertGroupHeader(int nIndex, int nGroupID, const CString& strHeader, DWORD dwState /* = LVGS_NORMAL */, DWORD dwAlign /*= LVGA_HEADER_LEFT*/)
{
	LVGROUP lg = {0};
	lg.cbSize = sizeof(lg);
	lg.iGroupId = nGroupID;
	lg.state = dwState;
	lg.mask = LVGF_GROUPID | LVGF_HEADER | LVGF_STATE | LVGF_ALIGN;
	lg.uAlign = dwAlign;

	// Header-title must be unicode (Convert if necessary)
#ifdef UNICODE
	lg.pszHeader = strHeader.GetBuffer();
	lg.cchHeader = strHeader.GetLength();
#else
	CComBSTR header = strHeader;
	lg.pszHeader = header;
	lg.cchHeader = header.Length();
#endif

	return InsertGroup(nIndex, (PLVGROUP)&lg );
}

BOOL CGroupListCtrl::SetRowGroupId(int nRow, int nGroupID)
{
	//OBS! Rows not assigned to a group will not show in group-view
	LVITEM lvItem = {0};
	lvItem.mask = LVIF_GROUPID;
	lvItem.iItem = nRow;
	lvItem.iSubItem = 0;
	lvItem.iGroupId = nGroupID;
	return SetItem( &lvItem );
}

int CGroupListCtrl::GetRowGroupId(int nRow)
{
	LVITEM lvi = {0};
    lvi.mask = LVIF_GROUPID;
    lvi.iItem = nRow;
	VERIFY( GetItem(&lvi) );
    return lvi.iGroupId;
}

CString CGroupListCtrl::GetGroupHeader(int nGroupID)
{
	LVGROUP lg = {0};
	lg.cbSize = sizeof(lg);
	lg.iGroupId = nGroupID;
	lg.mask = LVGF_HEADER | LVGF_GROUPID;
	VERIFY( GetGroupInfo(nGroupID, (PLVGROUP)&lg) != -1 );
#ifdef UNICODE
	return lg.pszHeader;
#else
	CComBSTR header( lg.pszHeader );
	return (LPCTSTR)COLE2T(header);
#endif
}

BOOL CGroupListCtrl::IsGroupStateEnabled()
{
	if (!IsGroupViewEnabled())
		return FALSE;

	OSVERSIONINFO osver = {0};
	osver.dwOSVersionInfoSize = sizeof(osver);
	GetVersionEx(&osver);
	WORD fullver = MAKEWORD(osver.dwMinorVersion, osver.dwMajorVersion);
	if (fullver < 0x0600)
		return FALSE;

	return TRUE;
}

// Vista SDK - ListView_GetGroupState / LVM_GETGROUPSTATE
BOOL CGroupListCtrl::HasGroupState(int nGroupID, DWORD dwState)
{
	LVGROUP lg = {0};
	lg.cbSize = sizeof(lg);
	lg.mask = LVGF_STATE;
	lg.stateMask = dwState;
	if ( GetGroupInfo(nGroupID, (PLVGROUP)&lg) == -1)
		return FALSE;

	return lg.state==dwState;
}

// Vista SDK - ListView_SetGroupState / LVM_SETGROUPINFO
BOOL CGroupListCtrl::SetGroupState(int nGroupID, DWORD dwState)
{
	if (!IsGroupStateEnabled())
		return FALSE;

	LVGROUP lg = {0};
	lg.cbSize = sizeof(lg);
	lg.mask = LVGF_STATE;
	lg.state = dwState;
	lg.stateMask = dwState;

#ifdef LVGS_COLLAPSIBLE
	if (HasGroupState(nGroupID, LVGS_COLLAPSIBLE))
		lg.state |= LVGS_COLLAPSIBLE;
#endif

	if (SetGroupInfo(nGroupID, (PLVGROUP)&lg)==-1)
		return FALSE;

	return TRUE;
}

int CGroupListCtrl::GroupHitTest(const CPoint& point)
{
	if (!IsGroupViewEnabled())
		return -1;

	if (HitTest(point)!=-1)
		return -1;

	if (IsGroupStateEnabled())
	{
#ifdef ListView_HitTestEx
#ifdef LVHT_EX_GROUP_HEADER
		LVHITTESTINFO lvhitinfo = {0};
		lvhitinfo.pt = point;
		ListView_HitTestEx(m_hWnd, &lvhitinfo);
		if ((lvhitinfo.flags & LVHT_EX_GROUP)==0)
			return -1;
#endif
#endif

#ifdef ListView_GetGroupCount
#ifdef ListView_GetGroupRect
#ifdef ListView_GetGroupInfoByIndex
		bool foundGroup = false;
		LRESULT groupCount = ListView_GetGroupCount(m_hWnd);
		if (groupCount <= 0)
			return -1;
		for(int i = 0 ; i < groupCount; ++i)
		{
			LVGROUP lg = {0};
			lg.cbSize = sizeof(lg);
			lg.mask = LVGF_GROUPID;
			VERIFY( ListView_GetGroupInfoByIndex(m_hWnd, i, &lg) );

			CRect rect(0,0,0,0);
			VERIFY( ListView_GetGroupRect(m_hWnd, lg.iGroupId, 0, &rect) );

			if (rect.PtInRect(point))
				return lg.iGroupId;
		}
		// Don't try other ways to find the group
		if (groupCount > 0)
			return -1;
#endif
#endif
#endif
	}	// IsGroupStateEnabled()

	// We require that each group contains atleast one item
	if (GetItemCount()==0)
		return -1;

	CRect gridRect(0,0,0,0);
	GetClientRect(&gridRect);

	int nRowAbove = -1, nRowBelow = 0;
	for(nRowBelow = GetTopIndex(); nRowBelow < GetItemCount(); nRowBelow++)
	{
		GetRowGroupId(nRowBelow);

		CRect rectRowBelow;
		if (GetItemRect(nRowBelow, rectRowBelow, LVIR_BOUNDS)==FALSE)
			continue;	// Found invisible row

		rectRowBelow.right = gridRect.right;
		if (rectRowBelow.PtInRect(point))
			return -1;	// Hit a row
		if (rectRowBelow.top > point.y)
			break;		// Found row just below the point

		nRowAbove = nRowBelow;
	}

	if (nRowBelow < GetItemCount())
	{
		// Probably hit the group just above this row
		return GetRowGroupId(nRowBelow);
	}

	return -1;
}

void CGroupListCtrl::CheckEntireGroup(int nGroupId, bool bChecked)
{
	for (int nRow=0; nRow<GetItemCount(); ++nRow)
	{
		if (GetRowGroupId(nRow) == nGroupId)
		{
			SetCheck(nRow, bChecked ? TRUE : FALSE);
		}
	}
}

void CGroupListCtrl::DeleteEntireGroup(int nGroupId)
{
	for (int nRow=0; nRow<GetItemCount(); ++nRow)
	{
		if (GetRowGroupId(nRow) == nGroupId)
		{
			DeleteItem(nRow);
			nRow--;
		}
	}
	RemoveGroup(nGroupId);
}

BOOL CGroupListCtrl::GroupByColumn(int nCol)
{
	RemoveAllGroups();

	EnableGroupView( GetItemCount() > 0 );

	if (IsGroupViewEnabled())
	{
		CSimpleMap<CString,CSimpleArray<int> > groups;

		// Loop through all rows and find possible groups
		for(int nRow=0; nRow<GetItemCount(); ++nRow)
		{
			const CString& cellText = GetItemText(nRow, nCol);

			int nGroupId = groups.FindKey(cellText);
			if (nGroupId==-1)
			{
				CSimpleArray<int> rows;
				rows.Add(nRow);
				groups.Add(cellText, rows);
			}
			else
				groups.GetValueAt(nGroupId).Add(nRow);
		}

		// Look through all groups and assign rows to group
		for(int nGroupId = 0; nGroupId < groups.GetSize(); ++nGroupId)
		{
			const CSimpleArray<int>& groupRows = groups.GetValueAt(nGroupId);
			DWORD dwState = LVGS_NORMAL;

#ifdef LVGS_COLLAPSIBLE
			if (IsGroupStateEnabled())
				dwState = LVGS_COLLAPSIBLE;
#endif

			VERIFY( InsertGroupHeader(nGroupId, nGroupId, groups.GetKeyAt(nGroupId), dwState) != -1);
			SetGroupTask(nGroupId, _T("Task: Check Group"));
			CString subtitle;
			subtitle.Format(_T("Subtitle: %i rows"), groupRows.GetSize());
			SetGroupSubtitle(nGroupId, subtitle );
			SetGroupFooter(nGroupId, _T("Group Footer"));
			
			for(int groupRow = 0; groupRow < groupRows.GetSize(); ++groupRow)
			{
				VERIFY( SetRowGroupId(groupRows[groupRow], nGroupId) );
			}
		}
		return TRUE;
	}

	return FALSE;
}

void CGroupListCtrl::CollapseAllGroups()
{
	// Loop through all rows and find possible groups
	for(int nRow=0; nRow<GetItemCount(); ++nRow)
	{
		int nGroupId = GetRowGroupId(nRow);
		if (nGroupId!=-1)
		{
			if (!HasGroupState(nGroupId,LVGS_COLLAPSED))
			{
				SetGroupState(nGroupId,LVGS_COLLAPSED);
			}
		}
	}
}

void CGroupListCtrl::ExpandAllGroups()
{
	// Loop through all rows and find possible groups
	for(int nRow=0; nRow<GetItemCount(); ++nRow)
	{
		int nGroupId = GetRowGroupId(nRow);
		if (nGroupId!=-1)
		{
			if (HasGroupState(nGroupId,LVGS_COLLAPSED))
			{
				SetGroupState(nGroupId,LVGS_NORMAL);
			}
		}
	}
}

void CGroupListCtrl::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CPoint pt = point;
	ScreenToClient(&pt);

	CRect headerRect;
	GetHeaderCtrl()->GetClientRect(&headerRect);
	if (headerRect.PtInRect(pt))
	{
		HDHITTESTINFO hdhti = {0};
		hdhti.pt = pt;
		::SendMessage(GetHeaderCtrl()->GetSafeHwnd(), HDM_HITTEST, 0, (LPARAM) &hdhti);
		if (hdhti.iItem!=-1)
		{
			// Retrieve column-title
			LVCOLUMN lvc = {0};
			lvc.mask = LVCF_TEXT;
			TCHAR sColText[256];
			lvc.pszText = sColText;
			lvc.cchTextMax = sizeof(sColText)-1;
			VERIFY( GetColumn(hdhti.iItem, &lvc) );

			CMenu menu;
			UINT uFlags = MF_BYPOSITION | MF_STRING;
			VERIFY( menu.CreatePopupMenu() );
			menu.InsertMenu(0, uFlags, 1, CString(_T("Group by: ")) + lvc.pszText);
			if (IsGroupViewEnabled())
				menu.InsertMenu(1, uFlags, 2, _T("Disable grouping"));
			int nResult = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, point.x, point.y, this, 0);
			switch(nResult)
			{
				case 1:	GroupByColumn(hdhti.iItem); break;
				case 2: RemoveAllGroups(); EnableGroupView(FALSE); break;
			}
		}
	}
	else
	{
		if ( IsGroupViewEnabled() )
		{
			CMenu menu;
			UINT uFlags = MF_BYPOSITION | MF_STRING;
			VERIFY( menu.CreatePopupMenu() );
			
			int nGroupId = GroupHitTest(pt);
			if (nGroupId >= 0)
			{
				const CString& groupHeader = GetGroupHeader(nGroupId);

#ifndef LVGS_COLLAPSIBLE
				if (IsGroupStateEnabled())
				{
					if (HasGroupState(nGroupId,LVGS_COLLAPSED))
					{
						CString menuText = CString(_T("Expand group: ")) + groupHeader;
						menu.InsertMenu(0, uFlags, 1, menuText);
					}
					else
					{
						CString menuText = CString(_T("Collapse group: ")) + groupHeader;
						menu.InsertMenu(0, uFlags, 2, menuText);
					}
				}
#endif
				CString menuText = CString(_T("Check group: ")) + groupHeader;
				menu.InsertMenu(1, uFlags, 3, menuText);
				menuText = CString(_T("Uncheck group: ")) + groupHeader;
				menu.InsertMenu(2, uFlags, 4, menuText);
				menuText = CString(_T("Delete group: ")) + groupHeader;
				menu.InsertMenu(3, uFlags, 5, menuText);
			
				menu.InsertMenu(4, uFlags | MF_SEPARATOR, 6, _T(""));
			}

			int nRow = HitTest(pt);
			if (nRow==-1)
			{
				if (IsGroupStateEnabled())
				{
					menu.InsertMenu(5, uFlags, 7, _T("Expand all groups"));
					menu.InsertMenu(6, uFlags, 8, _T("Collapse all groups"));
				}
				menu.InsertMenu(7, uFlags, 9, _T("Disable grouping"));
			}

			int nResult = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, point.x, point.y, this, 0);
			switch(nResult)
			{
				case 1: SetGroupState(nGroupId,LVGS_NORMAL); break;
				case 2: SetGroupState(nGroupId,LVGS_COLLAPSED); break;
				case 3: CheckEntireGroup(nGroupId, true); break;
				case 4: CheckEntireGroup(nGroupId, false); break;
				case 5: DeleteEntireGroup(nGroupId); break;
				case 7: ExpandAllGroups(); break;
				case 8: CollapseAllGroups(); break;
				case 9: RemoveAllGroups(); EnableGroupView(FALSE); break;
			}
		}
	}
}

namespace {
	struct PARAMSORT
	{
		PARAMSORT(HWND hWnd, int columnIndex, bool ascending)
			:m_hWnd(hWnd)
			,m_ColumnIndex(columnIndex)
			,m_Ascending(ascending)
		{}

		HWND m_hWnd;
		int  m_ColumnIndex;
		bool m_Ascending;
		CSimpleMap<int,CString> m_GroupNames;
	};

	// Comparison extracts values from the List-Control
	int CALLBACK SortFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
	{
		PARAMSORT& ps = *(PARAMSORT*)lParamSort;

		TCHAR left[256] = _T(""), right[256] = _T("");
		ListView_GetItemText(ps.m_hWnd, lParam1, ps.m_ColumnIndex, left, sizeof(left));
		ListView_GetItemText(ps.m_hWnd, lParam2, ps.m_ColumnIndex, right, sizeof(right));

		if (ps.m_Ascending)
			return _tcscmp( left, right );
		else
			return _tcscmp( right, left );			
	}

	int CALLBACK SortFuncGroup(int nGroupId1, int nGroupId2, void* lParamSort)
	{
		PARAMSORT& ps = *(PARAMSORT*)lParamSort;

		const CString& left = ps.m_GroupNames.Lookup(nGroupId1);
		const CString& right = ps.m_GroupNames.Lookup(nGroupId2);

		if (ps.m_Ascending)
			return _tcscmp( left, right );
		else
			return _tcscmp( right, left );	
	}
}

bool CGroupListCtrl::SortColumn(int columnIndex, bool ascending)
{
	PARAMSORT paramsort(m_hWnd, columnIndex, ascending);
	if (IsGroupViewEnabled())
	{
		GroupByColumn(columnIndex);

		// Cannot use GetGroupInfo during sort
		for(int nRow=0 ; nRow < GetItemCount() ; ++nRow)
		{
			int nGroupId = GetRowGroupId(nRow);
			if (nGroupId!=-1 && paramsort.m_GroupNames.Lookup(nGroupId).IsEmpty())
				paramsort.m_GroupNames.Add(nGroupId, GetGroupHeader(nGroupId));
		}

		// Avoid bug in CListCtrl::SortGroups() which differs from ListView_SortGroups
		ListView_SortGroups(m_hWnd, SortFuncGroup, &paramsort);
	}

	// Always sort the rows, so the handicapped GroupHitTest() will work
	//	- Must ensure that the rows are reordered along with the groups.
	ListView_SortItemsEx(m_hWnd, SortFunc, &paramsort);
	return true;
}

BOOL CGroupListCtrl::SetGroupFooter(int nGroupID, const CString& footer, DWORD dwAlign /*= LVGA_FOOTER_CENTER*/)
{
	if (!IsGroupStateEnabled())
		return FALSE;

#if _WIN32_WINNT >= 0x0600
	LVGROUP lg = {0};
	lg.cbSize = sizeof(lg);
	lg.mask = LVGF_FOOTER;
#ifdef UNICODE
	lg.pszFooter = footer.GetBuffer();
	lg.cchFooter = footer.GetLength();
#else
	CComBSTR bstrFooter = footer;
	lg.pszFooter = bstrFooter;
	lg.cchFooter = bstrFooter.Length();
#endif

	if (SetGroupInfo(nGroupID, (PLVGROUP)&lg)==-1)
		return FALSE;

	return TRUE;
#else
	return FALSE;
#endif
}

BOOL CGroupListCtrl::SetGroupTask(int nGroupID, const CString& task)
{
	if (!IsGroupStateEnabled())
		return FALSE;

#if _WIN32_WINNT >= 0x0600
	LVGROUP lg = {0};
	lg.cbSize = sizeof(lg);
	lg.mask = LVGF_TASK;
#ifdef UNICODE
	lg.pszTask = task.GetBuffer();
	lg.cchTask = task.GetLength();
#else
	CComBSTR bstrTask = task;
	lg.pszTask = bstrTask;
	lg.cchTask = bstrTask.Length();
#endif

	if (SetGroupInfo(nGroupID, (PLVGROUP)&lg)==-1)
		return FALSE;

	return TRUE;
#else
	return FALSE;
#endif
}

BOOL CGroupListCtrl::SetGroupSubtitle(int nGroupID, const CString& subtitle)
{
	if (!IsGroupStateEnabled())
		return FALSE;

#if _WIN32_WINNT >= 0x0600
	LVGROUP lg = {0};
	lg.cbSize = sizeof(lg);
	lg.mask = LVGF_SUBTITLE;
#ifdef UNICODE
	lg.pszSubtitle = subtitle.GetBuffer();
	lg.cchSubtitle = subtitle.GetLength();
#else
	CComBSTR bstrSubtitle = subtitle;
	lg.pszSubtitle = bstrSubtitle;
	lg.cchSubtitle = bstrSubtitle.Length();
#endif

	if (SetGroupInfo(nGroupID, (PLVGROUP)&lg)==-1)
		return FALSE;

	return TRUE;
#else
	return FALSE;
#endif
}

BOOL CGroupListCtrl::SetGroupTitleImage(int nGroupID, int nImage, const CString& topDesc, const CString& bottomDesc)
{
	if (!IsGroupStateEnabled())
		return FALSE;

#if _WIN32_WINNT >= 0x0600
	LVGROUP lg = {0};
	lg.cbSize = sizeof(lg);
	lg.mask = LVGF_TITLEIMAGE;
	lg.iTitleImage = nImage;	// Index of the title image in the control imagelist.

#ifdef UNICODE
	if (!topDesc.IsEmpty())
	{
		// Top description is drawn opposite the title image when there is
		// a title image, no extended image, and uAlign==LVGA_HEADER_CENTER.
		lg.mask |= LVGF_DESCRIPTIONTOP;
		lg.pszDescriptionTop = topDesc;
		lg.cchDescriptionTop = topDesc.GetLength();
	}
	if (!bottomDesc.IsEmpty())
	{
		// Bottom description is drawn under the top description text when there is
		// a title image, no extended image, and uAlign==LVGA_HEADER_CENTER.
		lg.mask |= LVGF_DESCRIPTIONBOTTOM;
		lg.pszDescriptionBottom = bottomDesc;
		lg.cchDescriptionBottom = bottomDesc.GetLength();
	}
#else
	CComBSTR bstrTopDesc = topDesc;
	CComBSTR bstrBottomDesc = bottomDesc;
	if (!topDesc.IsEmpty())
	{
		lg.mask |= LVGF_DESCRIPTIONTOP;
		lg.pszDescriptionTop = bstrTopDesc;
		lg.cchDescriptionTop = bstrTopDesc.Length();
	}
	if (!bottomDesc.IsEmpty())
	{
		lg.mask |= LVGF_DESCRIPTIONBOTTOM;
		lg.pszDescriptionBottom = bstrBottomDesc;
		lg.cchDescriptionBottom = bstrBottomDesc.Length();
	}
#endif

	if (SetGroupInfo(nGroupID, (PLVGROUP)&lg)==-1)
		return FALSE;

	return TRUE;
#else
	return FALSE;
#endif
}

BOOL CGroupListCtrl::OnGroupTaskClick(NMHDR* pNMHDR, LRESULT* pResult)
{
#if _WIN32_WINNT >= 0x0600
	NMLVLINK* pLinkInfo = (NMLVLINK*)pNMHDR;
	int nGroupId = pLinkInfo->iSubItem;
	CheckEntireGroup(nGroupId, true);
#endif
	return FALSE;
}

void CGroupListCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	int nGroupId = GroupHitTest(point);
	if (nGroupId!=-1)
	{
		if (HasGroupState(nGroupId, LVGS_COLLAPSED))
			SetGroupState(nGroupId, LVGS_NORMAL);
		else
			SetGroupState(nGroupId, LVGS_COLLAPSED);
	}
}

BOOL CGroupListCtrl::OnHeaderClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLISTVIEW* pLV = reinterpret_cast<NMLISTVIEW*>(pNMHDR);

	int nCol = pLV->iSubItem;
	if (m_SortCol==nCol)
	{
		m_Ascending = !m_Ascending;
	}
	else
	{
		m_SortCol = nCol;
		m_Ascending = true;
	}

	SortColumn(m_SortCol, m_Ascending);
	return FALSE;	// Let parent-dialog get chance
}

namespace {
	LRESULT EnableWindowTheme(HWND hwnd, LPCWSTR classList, LPCWSTR subApp, LPCWSTR idlist)
	{
		HMODULE hinstDll;
		HRESULT (__stdcall *pSetWindowTheme)(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);
		HANDLE (__stdcall *pOpenThemeData)(HWND hwnd, LPCWSTR pszClassList);
		HRESULT (__stdcall *pCloseThemeData)(HANDLE hTheme);

		hinstDll = ::LoadLibrary("UxTheme.dll");
		if (hinstDll)
		{
			(FARPROC&)pOpenThemeData = ::GetProcAddress(hinstDll, TEXT("OpenThemeData"));
			(FARPROC&)pCloseThemeData = ::GetProcAddress(hinstDll, TEXT("CloseThemeData"));
			(FARPROC&)pSetWindowTheme = ::GetProcAddress(hinstDll, TEXT("SetWindowTheme"));
			::FreeLibrary(hinstDll);
			if (pSetWindowTheme && pOpenThemeData && pCloseThemeData)
			{
				HANDLE theme = pOpenThemeData(hwnd,classList);
				if (theme!=NULL)
				{
					VERIFY(pCloseThemeData(theme)==S_OK);
					return pSetWindowTheme(hwnd, subApp, idlist);
				}
			}
		}
		return S_FALSE;
	}

	bool IsThemeEnabled()
	{
		HMODULE hinstDll;
		bool XPStyle = false;
		bool (__stdcall *pIsAppThemed)();
		bool (__stdcall *pIsThemeActive)();

		// Test if operating system has themes enabled
		hinstDll = ::LoadLibrary("UxTheme.dll");
		if (hinstDll)
		{
			(FARPROC&)pIsAppThemed = ::GetProcAddress(hinstDll, "IsAppThemed");
			(FARPROC&)pIsThemeActive = ::GetProcAddress(hinstDll,"IsThemeActive");
			::FreeLibrary(hinstDll);
			if (pIsAppThemed != NULL && pIsThemeActive != NULL)
			{
				if (pIsAppThemed() && pIsThemeActive())
				{
					// Test if application has themes enabled by loading the proper DLL
					hinstDll = LoadLibrary("comctl32.dll");
					if (hinstDll)
					{
						DLLGETVERSIONPROC pDllGetVersion = (DLLGETVERSIONPROC)::GetProcAddress(hinstDll, "DllGetVersion");
						::FreeLibrary(hinstDll);
						if (pDllGetVersion != NULL)
						{
							DLLVERSIONINFO dvi;
							ZeroMemory(&dvi, sizeof(dvi));
							dvi.cbSize = sizeof(dvi);
							HRESULT hRes = pDllGetVersion ((DLLVERSIONINFO *) &dvi);
							if (SUCCEEDED(hRes))
                                XPStyle = dvi.dwMajorVersion >= 6;
						}
					}
				}
			}
		}
		return XPStyle;
	}
}

void CGroupListCtrl::PreSubclassWindow()
{
	CListCtrl::PreSubclassWindow();

	// Focus retangle is not painted properly without double-buffering
#if (_WIN32_WINNT >= 0x501)
	SetExtendedStyle(LVS_EX_DOUBLEBUFFER | GetExtendedStyle());
#endif
	SetExtendedStyle(GetExtendedStyle() | LVS_EX_FULLROWSELECT);
	SetExtendedStyle(GetExtendedStyle() | LVS_EX_HEADERDRAGDROP);
	SetExtendedStyle(GetExtendedStyle() | LVS_EX_GRIDLINES);
	//SetExtendedStyle(GetExtendedStyle() | LVS_EX_CHECKBOXES);

	// Enable Vista-look if possible
	EnableWindowTheme(GetSafeHwnd(), L"ListView", L"Explorer", NULL);
	EnableGroupView(TRUE);
}
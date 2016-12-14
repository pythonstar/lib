////////////////////////////////////////////////////////////////////////////
// File:	HistoryCombo.cpp
// Version:	3.1
// Created:	20-Apr-2007
//
// Author:	Paul S. Vickery
// E-mail:	paul@vickeryhome.freeserve.co.uk
//
// Implementation of CHistoryCombo which incorporates functionality to help
// with Loading and Saving of history in a combo box
//
// You are free to use or modify this code, with no restrictions, other than
// you continue to acknowledge me as the original author in this source code,
// or any code derived from it.
//
// If you use this code, or use it as a base for your own code, it would be 
// nice to hear from you simply so I know it's not been a waste of time!
//
// Copyright (c) 2001-2007 Paul S. Vickery
//
////////////////////////////////////////////////////////////////////////////
// Version History:
//
// Version 3.1 - 20-Apr-2007
// =========================
// * Updated to compile correctly with VC7 (VS2003) and VC8 (VS2005)
// * Enhanced to use Chris Maunder's auto-completion code 
//   (http://www.codeproject.com/combobox/combocompletion.asp)
//
// Version 3.0 - 22-Jun-2004
// =========================
// Added functionality:
// * added serialization to/from a CArchive object (suggested by EPulse), which 
//   can be used by one of three methods:
//   - calling Serialize() directly with a CArchive object
//   - calling the LoadHistory()/SaveHistory() overloads which take a CArchive 
//     object reference
//   - using the insertion operator overloads >> and <<
//   (the only difference between these methods is that using SaveHistory() 
//   gives you the option of NOT adding the current item to the history)
// * added loading/saving from/to a CString object (suggested by Uwe Keim)
//
// Version 2.1 - 09-Jul-2003
// =========================
// Updated to support Unicode.
//
// Version 2 - 01-May-2002
// =======================
// Produced new version with changes as below:
// * removed CBS_SORT on creation if specified
// * added option to allow the sort style to be set if required
// * fixed SetMaxHistoryItems, so it removes old entries from the list to 
//   ensure that there are no more than the maximum. Also made SaveHistory
//   remove redundant profile entries above the maximum.
// * use WriteProfileString to remove profile entries rather than CRegKey.
//
// Version 1 - 12-Apr-2001
// =======================
// Initial version
// 
////////////////////////////////////////////////////////////////////////////
// PLEASE LEAVE THIS HEADER INTACT
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HistoryCombo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_HISTORY_ITEMS 10

#if defined(_CRT_INSECURE_DEPRECATE) && ! defined(_CRT_SECURE_NO_DEPRECATE)
#define DECLARE_STRTOK_CURRENT  LPTSTR _Current;
#define _tcstok(_String, _Delimiters) _tcstok_s(_String, _Delimiters, &_Current)
#else
#define DECLARE_STRTOK_CURRENT ;
#endif // _CRT_INSECURE_DEPRECATE && ! _CRT_SECURE_NO_DEPRECATE

/////////////////////////////////////////////////////////////////////////////
// CHistoryCombo

// this line defines the default auto-complete behaviour for all CHistoryCombos
// the auto-complete may be set for each instance individually, but this value 
// may be set the the best value for most instances
/*static*/ BOOL CHistoryCombo::ms_bAllowAutoCompleteDefault = TRUE;//FALSE;

IMPLEMENT_SERIAL(CHistoryCombo, CComboBox, 0)

CHistoryCombo::CHistoryCombo(BOOL bAllowSortStyle/*=FALSE*/)
{
  m_nMaxHistoryItems = MAX_HISTORY_ITEMS;
  m_bSaveRestoreLastCurrent = TRUE;
  m_bDoAutoComplete = m_bAllowAutoComplete = ms_bAllowAutoCompleteDefault;
  m_bAllowSortStyle = bAllowSortStyle;
}

CHistoryCombo::~CHistoryCombo()
{
}

BOOL CHistoryCombo::PreCreateWindow(CREATESTRUCT& cs) 
{
  if (! m_bAllowSortStyle)  // turn off CBS_SORT style
    cs.style &= ~CBS_SORT;
  return CComboBox::PreCreateWindow(cs);
}

void CHistoryCombo::PreSubclassWindow() 
{
  // warn if creating with CBS_SORT style
  // (unfortunately we can't turn it off)
  if (! m_bAllowSortStyle && GetStyle() & CBS_SORT)
    TRACE(_T("WARNING: Creating History combo with CBS_SORT style\n"));

  ModifyStyle(0, CBS_AUTOHSCROLL);
  CComboBox::PreSubclassWindow();
}

BEGIN_MESSAGE_MAP(CHistoryCombo, CComboBox)
	//{{AFX_MSG_MAP(CHistoryCombo)
	ON_CONTROL_REFLECT(CBN_EDITUPDATE, OnEditUpdate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHistoryCombo message handlers

// this version of AddString adds a string only if it doesn't already exist
// in the list, and in any case, makes sure that the string is the first
// in the list (ie most recent in history)
// also makes sure number of items in the list doesn't exceed the maximum allowed
int CHistoryCombo::AddString(LPCTSTR lpszString)
{
  int nRet = -1;
  // don't add if already there
  CString sString(lpszString);
  sString.TrimLeft(_T(" "));
  sString.TrimRight(_T(" "));
  nRet = CComboBox::InsertString(0, sString);
  int nIndex = FindStringExact(0, sString);
  if (nIndex != -1 && nIndex != 0)
    DeleteString(nIndex);

  // if we've got too many items then delete the last one
  // truncate list to m_nMaxHistoryItems
  int nNumItems = GetCount();
  for (int n = m_nMaxHistoryItems; n < nNumItems; n++)
    DeleteString(m_nMaxHistoryItems);

  SetCurSel(nRet);
  return nRet;
}

// loads the history from the specified MRU list, and populates the combo list
// if bSelectMostRecent is TRUE, selects the most recent into the edit control
// returns the selected text
// if using this overload of LoadHistory then SaveHistory will not function
// but you should simply save the MRU in the normal way instead
// note that the MRU should have already been read from the profile before
// being passed to this function, as this function will not call MRU->ReadList()
CString CHistoryCombo::LoadHistory(CRecentFileList* pListMRU, BOOL bSelectMostRecent/*=TRUE*/)
{
  if (pListMRU == NULL)
    return "";

  ResetContent(); // clear out any old items
  int nNumItems = pListMRU->GetSize();
  for (int n = 0; n < nNumItems; n++)
  {
    LPCTSTR lpsz = (*pListMRU)[n];
    if (lpsz != NULL && *lpsz != '\0')
     CComboBox::AddString(lpsz);
  }
  if (bSelectMostRecent)
    SetCurSel(0);
  CString sText;
  GetWindowText(sText);
  return sText;
}

// loads the history from the specified profile area, and returns the 
// text selected
// the profile strings are cached so that it doesn't need to specified again
// when saving the history
CString CHistoryCombo::LoadHistory(LPCTSTR lpszSection, LPCTSTR lpszKeyPrefix, 
				   BOOL bSaveRestoreLastCurrent/*=TRUE*/, 
				   LPCTSTR lpszKeyCurItem/*=NULL*/)
{
  if (lpszSection == NULL || lpszKeyPrefix == NULL || *lpszSection == '\0')
    return "";

  ResetContent(); // clear out any old items
  m_sSection = lpszSection;
  m_sKeyPrefix = lpszKeyPrefix;
  m_sKeyCurItem = lpszKeyCurItem == NULL ? _T("") : lpszKeyCurItem;
  m_bSaveRestoreLastCurrent = bSaveRestoreLastCurrent;
  CWinApp* pApp = AfxGetApp();

  int n = 0;
  CString sText;
  do
  {
    CString sKey;
    sKey.Format(_T("%s%d"), m_sKeyPrefix, n++);
    sText = pApp->GetProfileString(m_sSection, sKey);
    if (!sText.IsEmpty())
      CComboBox::AddString(sText);
  }while (!sText.IsEmpty() && n < m_nMaxHistoryItems);
  if (m_bSaveRestoreLastCurrent)
  {
    CString sKey;
    if (!m_sKeyCurItem.IsEmpty())
      sKey = m_sKeyCurItem;
    else if (m_sKeyPrefix.IsEmpty())
      sKey = "Last";
    else
      sKey = m_sKeyPrefix;
    sText = pApp->GetProfileString(m_sSection, sKey);
    if (!sText.IsEmpty())
    {
      int nIndex = FindStringExact(-1, sText);
      if (nIndex != -1)
	SetCurSel(nIndex);
      else if (GetStyle() & CBS_DROPDOWN)
	SetWindowText(sText);
    }
  }
  return sText;
}

// load the history from an archive object in binary format
void CHistoryCombo::LoadHistory(CArchive& ar)
{
  ResetContent(); // clear out any old items
  ASSERT(ar.IsLoading());

  WORD wItems;
  ar >> wItems;
  int nItems = wItems;
  for (int n = 0; n < nItems; n++)
  {
    CString sText;
    ar >> sText;
    CComboBox::AddString(sText);
  }
  // current text
  CString sText;
  ar >> sText;
  // select text
  int nIndex = FindStringExact(-1, sText);
  if (nIndex == -1 && GetStyle() & CBS_DROPDOWN)
    SetWindowText(sText);
  else
    SetCurSel(nIndex);
}

// save the history to an archive object in binary format
void CHistoryCombo::SaveHistory(CArchive& ar, BOOL bAddCurrentItemToHistory/*=TRUE*/)
{
  ASSERT(ar.IsStoring());
  if (bAddCurrentItemToHistory)
    StoreValue();

  // write number of entries
  int nItems = GetCount();
  ar << (WORD)nItems;

  for (int n = 0; n < nItems; n++)
  {
    CString sText;
    GetLBText(n, sText);
    ar << sText;
  }
  // current text
  CString sText;
  GetWindowText(sText);
  ar << sText;
}

// load the history from the string pointed to by lpszHistory
// set the current selection to lpszLastSelected if not NULL
// the lpszHistory should contain a list of items separated by 
// the chars passed in lpszDelims (which defaults to CRLF)
void CHistoryCombo::LoadHistoryFromText(LPCTSTR lpszHistory, 
					LPCTSTR lpszLastSelected/*=NULL*/, 
					LPCTSTR lpszDelims/*=_T("\r\n")*/)
{
  if (lpszHistory == NULL || *lpszHistory == '\0')
    return;
  ResetContent(); // clear out any old items
  LPTSTR lpszList = _tcsdup(lpszHistory);
  if (lpszDelims == NULL || *lpszDelims == '\0')
    lpszDelims = _T("\r\n");
  DECLARE_STRTOK_CURRENT
  LPTSTR lpszTok = _tcstok(lpszList, lpszDelims);
  while (lpszTok != NULL)
  {
    CComboBox::AddString(lpszTok);
    lpszTok = _tcstok(NULL, lpszDelims);
  }
  if (lpszLastSelected != NULL)
  {
    // see if it's in the list of items added
    // if it is then select it, else, if it's an editable
    // combo, set the combo's text to the value
    int nIndex = FindStringExact(-1, lpszLastSelected);
    if (nIndex == -1 && GetStyle() & CBS_DROPDOWN)
      SetWindowText(lpszLastSelected);
    else
      SetCurSel(nIndex);
  }
  if (lpszList != NULL)
    free(lpszList);
}

// write the history to the CString object sHistory
// and return the last selected item's text
CString CHistoryCombo::SaveHistoryToText(CString& sHistory, 
					 BOOL bAddCurrentItemToHistory/*=TRUE*/, 
					 LPCTSTR lpszDelims/*=_T("\r\n")*/)
{
  CString sCurItem;
  GetWindowText(sCurItem);

  if (bAddCurrentItemToHistory)
    StoreValue();

  if (lpszDelims == NULL || *lpszDelims == '\0')
    lpszDelims = _T("\r\n");

  sHistory.Empty();
  int nCount = GetCount();
  for (int n = 0; n < nCount; n++)
  {
    CString sItem;
    GetLBText(n, sItem);
    if (! sHistory.IsEmpty())
      sHistory += lpszDelims;
    sHistory += sItem;
  }
  
  return sCurItem;
}

// saves the history to the profile specified when calling LoadHistory
// if no profile information (ie LoadHistory() wasn't called with it) then
// this function does nothing
void CHistoryCombo::SaveHistory(BOOL bAddCurrentItemToHistory/*=TRUE*/)
{
  if (m_sSection.IsEmpty())
    return;

  CWinApp* pApp = AfxGetApp();
  ASSERT(pApp);

  if (bAddCurrentItemToHistory)
    StoreValue();

  // save history to info cached earlier
  int nMax = min(GetCount(), m_nMaxHistoryItems + 1);
  int n;
  for (n = 0; n < nMax; n++)
  {
    CString sKey;
    sKey.Format(_T("%s%d"), m_sKeyPrefix, n);
    CString sText;
    GetLBText(n, sText);
    pApp->WriteProfileString(m_sSection, sKey, sText);
  }
  // remove redundant items
  for (n = nMax; n < 1000/* prevent runaway*/; n++)
  {
    CString sKey;
    sKey.Format(_T("%s%d"), m_sKeyPrefix, n);
    CString sText = pApp->GetProfileString(m_sSection, sKey);
    if (sText.IsEmpty())
      break;
    pApp->WriteProfileString(m_sSection, sKey, NULL); // remove entry
  }
  if (m_bSaveRestoreLastCurrent)
  {
    CString sText;
    GetWindowText(sText);
    CString sKey;
    if (!m_sKeyCurItem.IsEmpty())
      sKey = m_sKeyCurItem;
    else if (m_sKeyPrefix.IsEmpty())
      sKey = "Last";
    else
      sKey = m_sKeyPrefix;
    pApp->WriteProfileString(m_sSection, sKey, sText);
  }
}

// removes all the items from the history list, and optionally deletes
// the registry items. Note that if the history list is generated from
// a CRecentFileList, then registry entries will not be deleted
void CHistoryCombo::ClearHistory(BOOL bDeleteRegistryEntries/*=TRUE*/)
{
  ResetContent();
  if (! m_sSection.IsEmpty() && bDeleteRegistryEntries)
  {
    // remove profile entries
    CWinApp* pApp = AfxGetApp();
    ASSERT(pApp);
    CString sKey;
    for (int n = 0; n < 1000/* prevent runaway*/; n++)
    {
      sKey.Format(_T("%s%d"), m_sKeyPrefix, n);
      CString sText = pApp->GetProfileString(m_sSection, sKey);
      if (sText.IsEmpty())
	break;
      pApp->WriteProfileString(m_sSection, sKey, NULL); // remove entry
    }
    if (! m_sKeyCurItem.IsEmpty())
      sKey = m_sKeyCurItem;
    else if (m_sKeyPrefix.IsEmpty())
      sKey = "Last";
    else
      sKey = m_sKeyPrefix;
    pApp->WriteProfileString(m_sSection, sKey, NULL);
  }
}

void CHistoryCombo::SetMaxHistoryItems(int nMaxItems)
{
  m_nMaxHistoryItems = nMaxItems;

  // truncate list to nMaxItems
  int nNumItems = GetCount();
  for (int n = m_nMaxHistoryItems; n < nNumItems; n++)
    DeleteString(m_nMaxHistoryItems);
}

void CHistoryCombo::StoreValue(BOOL bIgnoreIfEmpty/*=TRUE*/)
{
  // add the current value to the list
  CString sValue;
  GetWindowText(sValue);
  if (bIgnoreIfEmpty && sValue.IsEmpty())
    return;
  AddString(sValue);
}

BOOL CHistoryCombo::PreTranslateMessage(MSG* pMsg)
{
  if (m_bAllowAutoComplete)
  {
    // Need to check for backspace/delete. These will modify the text in
    // the edit box, causing the auto complete to just add back the text
    // the user has just tried to delete. 

    if (pMsg->message == WM_KEYDOWN)
    {
      m_bDoAutoComplete = TRUE;

      int nVirtKey = (int) pMsg->wParam;
      if (nVirtKey == VK_DELETE || nVirtKey == VK_BACK)
	m_bDoAutoComplete = FALSE;
    }
  }

  return CComboBox::PreTranslateMessage(pMsg);
}

void CHistoryCombo::OnEditUpdate() 
{
  // if we are not to auto update the text, get outta here
  if (! m_bAllowAutoComplete || ! m_bDoAutoComplete) 
      return;

  // Get the text in the edit box
  CString str;
  GetWindowText(str);
  int nLength = str.GetLength();
  
  // Currently selected range
  DWORD dwCurSel = GetEditSel();
  WORD dStart = LOWORD(dwCurSel);
  WORD dEnd   = HIWORD(dwCurSel);

  // Search for, and select in, and string in the combo box that is prefixed
  // by the text in the edit box
  if (SelectString(-1, str) == CB_ERR)
  {
      SetWindowText(str);		// No text selected, so restore what was there before
      if (dwCurSel != CB_ERR)
        SetEditSel(dStart, dEnd);	//restore cursor postion
  }

  // Set the text selection as the additional text that we have added
  if (dEnd < nLength && dwCurSel != CB_ERR)
      SetEditSel(dStart, dEnd);
  else
      SetEditSel(nLength, -1);
}

// serialize the history to and from an archive
void CHistoryCombo::Serialize(CArchive& ar) 
{
  if (ar.IsStoring())
    SaveHistory(ar);
  else
    LoadHistory(ar);
}

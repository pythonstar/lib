// Misc.h: interface for the CMisc class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MISC_H__4B2FDA3E_63C5_4F52_A139_9512105C3AD4__INCLUDED_)
#define AFX_MISC_H__4B2FDA3E_63C5_4F52_A139_9512105C3AD4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

enum { MKS_CTRL = 0x01, MKS_SHIFT = 0x02, MKS_ALT = 0x04 };
enum { MKC_LEFTRIGHT = 0x01, MKC_UPDOWN = 0x02, MKC_ANY = (MKC_LEFTRIGHT | MKC_UPDOWN) };

#ifndef _ttof
#define _ttof(str) Misc::Atof(str)
#endif

//class CString;
class CStringArray;
class CDWordArray;

namespace Misc  
{
	CString FormatGetLastError(DWORD dwLastErr = -1);

	BOOL CopyTexttoClipboard(const CString& sText, HWND hwnd); 
	CString GetClipboardText(HWND hwnd); 
	BOOL ClipboardHasFormat(UINT nFormat, HWND hwnd);

	int GetDropFilePaths(HDROP hDrop, CStringArray& aFiles);
	
	BOOL IsMultibyteString(const CString& sText);
	char* WideToMultiByte(const WCHAR* szFrom, UINT nCodePage = CP_ACP);
	char* WideToMultiByte(const WCHAR* szFrom, int& nLength, UINT nCodePage = CP_ACP);
	WCHAR* MultiByteToWide(const char* szFrom, UINT nCodepage = CP_ACP);
	WCHAR* MultiByteToWide(const char* szFrom, int& nLength, UINT nCodepage = CP_ACP);

	BOOL GuidFromString(LPCTSTR szGuid, GUID& guid);
	BOOL IsGuid(LPCTSTR szGuid);
	BOOL GuidToString(const GUID& guid, CString& sGuid);
	BOOL GuidIsNull(const GUID& guid);
	void NullGuid(GUID& guid);
	BOOL SameGuids(const GUID& guid1, const GUID& guid2);

	template <class T> 
	BOOL ArraysMatch(const T& array1, const T& array2, BOOL bOrderSensitive = FALSE)
	{
		int nSize1 = array1.GetSize();
		int nSize2 = array2.GetSize();
		
		if (nSize1 != nSize2)
			return 0;
		
		if (bOrderSensitive)
		{
			for (int nItem1 = 0; nItem1 < nSize1; nItem1++)
			{
				// check for non-equality
				BOOL bMatch = (array1[nItem1] == array2[nItem1]);

				if (!bMatch)
					return FALSE;
			}
			
			return TRUE;
		}
		
		// else order not important
		for (int nItem1 = 0; nItem1 < nSize1; nItem1++)
		{
			BOOL bMatch = FALSE;
			
			// look for matching item
			for (int nItem2 = 0; nItem2 < nSize2 && !bMatch; nItem2++)
				bMatch = (array1[nItem1] == array2[nItem2]);
			
			// no-match found == not the same
			if (!bMatch)
				return FALSE;
		}
		
		return TRUE;
	}

	CString FormatComputerNameAndUser(char cSeparator = ':');
	CString GetComputerName();
	CString GetUserName();
	CString GetListSeparator();
	CString GetDecimalSeparator();
	CString GetDefCharset();
	CString GetAM();
	CString GetPM();
	CString GetTimeSeparator();
	CString GetTimeFormat(BOOL bIncSeconds = TRUE);
	CString GetShortDateFormat(BOOL bIncDOW = FALSE);
	CString GetDateSeparator();
	BOOL IsMetricMeasurementSystem();

	BOOL ArraysMatch(const CStringArray& array1, const CStringArray& array2, 
					 BOOL bOrderSensitive = FALSE, BOOL bCaseSensitive = FALSE);
	BOOL MatchAny(const CStringArray& array1, const CStringArray& array2, BOOL bCaseSensitive = FALSE);
	CString FormatArray(const CStringArray& array, LPCTSTR szSep = NULL);
	CString FormatArray(const CDWordArray& array, LPCTSTR szSep = NULL);
	int Find(const CStringArray& array, LPCTSTR szItem, BOOL bCaseSensitive = FALSE);
	void Trace(const CStringArray& array);
	int RemoveItems(const CStringArray& aItems, CStringArray& aFrom, BOOL bCaseSensitive = FALSE);
	int AddUniqueItems(const CStringArray& aItems, CStringArray& aTo, BOOL bCaseSensitive = FALSE);
	int AddUniqueItem(const CString& sItem, CStringArray& aTo, BOOL bCaseSensitive = FALSE);

	int Split(const CString& sText, CStringArray& aValues, BOOL bAllowEmpty = FALSE, const CString& sSep = GetListSeparator());
 	int Split(const CString& sText, char cDelim, CStringArray& aValues);

	typedef int (CALLBACK* SORTSTRINGPROC)(const CString&, const CString&);
	void SortArray(CStringArray& array, BOOL bAscending = TRUE, SORTSTRINGPROC pSortProc = NULL);

	int CompareVersions(LPCTSTR szVersion1, LPCTSTR szVersion2);

	double Round(double dValue);
	float Round(float fValue);
	double Atof(const CString& sValue);
	CString Format(double dVal, int nDecPlaces = 2);
	CString Format(int nVal);
	CString FormatCost(double dCost);
	BOOL IsNumber(const CString& sValue);
	BOOL IsSymbol(const CString& sValue);

	BOOL IsWorkStationLocked();
	BOOL IsScreenSaverActive();

	void ProcessMsgLoop();
	int ShowMessageBox(HWND hwndParent, LPCTSTR szCaption, LPCTSTR szInstruction, LPCTSTR szText, UINT nFlags);

	int ParseSearchString(LPCTSTR szLookFor, CStringArray& aWords);
	BOOL FindWord(LPCTSTR szWord, LPCTSTR szText, BOOL bCaseSensitive, BOOL bMatchWholeWord);

	BOOL ModKeysArePressed(DWORD dwKeys); 
	BOOL KeyIsPressed(DWORD dwVirtKey);
	BOOL IsCursorKey(DWORD dwVirtKey, DWORD dwKeys = MKC_ANY);
	BOOL IsCursorKeyPressed(DWORD dwKeys = MKC_ANY);

	BOOL HasFlag(DWORD dwFlags, DWORD dwFlag);
};

#endif // !defined(AFX_MISC_H__4B2FDA3E_63C5_4F52_A139_9512105C3AD4__INCLUDED_)

// Reg.cpp: implementation of the CReg class.
// Author: Bjarke Viksøe
//
// Description:
// Thin wrapper around the Win32 System Registry interface.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Reg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CReg::CReg()
{
   m_hKey = NULL;
}

CReg::CReg(HKEY hKey, LPCTSTR Section)
{
   m_hKey = NULL;
   SetRegistryBranch( hKey, Section );
}

CReg::~CReg()
{
}


//////////////////////////////////////////////////////////////////////
// Initialization

BOOL CReg::SetRegistryBranch(HKEY hKey, LPCTSTR Section)
{
   ASSERT(hKey);
   ASSERT(AfxIsValidString(Section));
   m_hKey = hKey;
   m_strSection = Section;
   ASSERT(!m_strSection.IsEmpty());
   return TRUE;
}

HKEY CReg::GetSectionKey(HKEY Branch, LPCTSTR lpszSection, long lPermission)
// Returns registry key for
//      Branch\lpszSection
// It does not create it if it doesn't exist.
// Responsibility of the caller to call RegCloseKey() on the returned HKEY
{
   ASSERT(AfxIsValidString(lpszSection));
   HKEY hKey = NULL;
   if( RegOpenKeyEx( Branch, lpszSection, 0, lPermission, &hKey ) == ERROR_SUCCESS ) {
      return hKey;
   }
   if (hKey!=NULL) RegCloseKey(hKey);
   return NULL;
}

HKEY CReg::CreateSectionKey(HKEY Branch, LPCTSTR lpszSection)
// Returns registry key for:
//      Branch\lpszSection
// creating it if it doesn't exist.
// responsibility of the caller to call RegCloseKey() on the returned HKEY
{
   ASSERT(AfxIsValidString(lpszSection));

   HKEY hKey = GetSectionKey(Branch,lpszSection);
   if(hKey!=NULL) return hKey;

   DWORD dw;
   RegCreateKeyEx(Branch, lpszSection, 0, REG_NONE,
      REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
      &hKey, &dw);
   return hKey;
}


//////////////////////////////////////////////////////////////////////
// Implementation

UINT CReg::GetInt(HKEY Branch, 
                  LPCTSTR lpszSection, 
                  LPCTSTR lpszEntry, 
                  int nDefault)
{
   ASSERT(AfxIsValidString(lpszSection));
   ASSERT(AfxIsValidString(lpszEntry));
   //
   HKEY hKey = GetSectionKey(Branch,lpszSection,KEY_READ);
   if (hKey == NULL) return nDefault;
   DWORD dwValue;
   DWORD dwType;
   DWORD dwCount = sizeof(DWORD);
   LONG lResult = ::RegQueryValueEx(hKey, (LPTSTR)lpszEntry, NULL, &dwType,
      (LPBYTE)&dwValue, &dwCount);
   ::RegCloseKey(hKey);
   if (lResult == ERROR_SUCCESS)
   {
      ASSERT(dwType == REG_DWORD);
      ASSERT(dwCount == sizeof(dwValue));
      return (UINT)dwValue;
   }
   return nDefault;
}

UINT CReg::GetInt(LPCTSTR lpszEntry, int nDefault)
{
   ASSERT(m_hKey);
   if( m_hKey==NULL ) return 0;
   return GetInt( m_hKey, m_strSection, lpszEntry, nDefault );
}

CString CReg::GetString(HKEY Branch, 
                        LPCTSTR lpszSection, 
                        LPCTSTR lpszEntry,
                        LPCTSTR lpszDefault)
{
   ASSERT(AfxIsValidString(lpszSection));
   //
   HKEY hKey = GetSectionKey(Branch,lpszSection,KEY_READ);
   if(hKey == NULL) return lpszDefault;
   CString strValue;
   DWORD dwType, dwCount;
   LONG lResult = ::RegQueryValueEx(hKey, (LPTSTR)lpszEntry, NULL, &dwType,
      NULL, &dwCount);
   if (lResult == ERROR_SUCCESS)
   {
      ASSERT( (dwType==REG_SZ) || (dwType==REG_EXPAND_SZ) );
      lResult = ::RegQueryValueEx(hKey, (LPTSTR)lpszEntry, NULL, &dwType,
         (LPBYTE)strValue.GetBuffer(dwCount/sizeof(TCHAR)), &dwCount);
      strValue.ReleaseBuffer();
   }
   ::RegCloseKey(hKey);
   if (lResult == ERROR_SUCCESS)
   {
      ASSERT( (dwType==REG_SZ) || (dwType==REG_EXPAND_SZ) );
      return strValue;
   }
   return lpszDefault;
}

CString CReg::GetString(LPCTSTR lpszEntry, LPCTSTR lpszDefault)
{
   ASSERT(m_hKey);
   if( m_hKey==NULL ) return CString();
   return GetString( m_hKey, m_strSection, lpszEntry, lpszDefault );
}

BOOL CReg::GetBinary(HKEY Branch,
                     LPCTSTR lpszSection, 
                     LPCTSTR lpszEntry,
                     BYTE** ppData, 
                     UINT* pBytes)
{
   ASSERT(AfxIsValidString(lpszSection));
   ASSERT(ppData!=NULL);
   ASSERT(pBytes!=NULL);
   //
   *ppData = NULL;
   *pBytes = 0;
   LPBYTE lpByte = NULL;
   HKEY hKey = GetSectionKey(Branch,lpszSection,KEY_READ);
   if (hKey == NULL)   return FALSE;

   DWORD dwType, dwCount;
   LONG lResult = ::RegQueryValueEx(hKey, (LPTSTR)lpszEntry, NULL, &dwType,
      NULL, &dwCount);
   *pBytes = dwCount;
   if (lResult == ERROR_SUCCESS) {
      ASSERT(dwType==REG_BINARY);
      *ppData = new BYTE[*pBytes];
      lResult = ::RegQueryValueEx(hKey, (LPTSTR)lpszEntry, NULL, &dwType,
         *ppData, &dwCount);
   }
   ::RegCloseKey(hKey);
   if (lResult == ERROR_SUCCESS) {
      ASSERT(dwType == REG_BINARY);
      return TRUE;
   }
   else
   {
      delete [] *ppData;
      *ppData = NULL;
   }
   return FALSE;
}

BOOL CReg::WriteInt(HKEY Branch, 
                    LPCTSTR lpszSection, 
                    LPCTSTR lpszEntry,
                    int nValue)
{
   ASSERT(AfxIsValidString(lpszSection));
   ASSERT(AfxIsValidString(lpszEntry));
   HKEY hKey = CreateSectionKey(Branch,lpszSection);
   if( hKey==NULL ) return FALSE;
   LONG lResult = ::RegSetValueEx(hKey, lpszEntry, NULL, REG_DWORD,
      (LPBYTE)&nValue, sizeof(nValue));
   ::RegCloseKey(hKey);
   return lResult == ERROR_SUCCESS;
}

BOOL CReg::WriteString(HKEY Branch, 
                  LPCTSTR lpszSection, 
                  LPCTSTR lpszEntry,
                  LPCTSTR lpszValue)
{
   ASSERT(AfxIsValidString(lpszSection));
   ASSERT(AfxIsValidString(lpszEntry));
   ASSERT(AfxIsValidString(lpszValue));
   LONG lResult;
   HKEY hKey = CreateSectionKey(Branch,lpszSection);
   if (hKey == NULL) return FALSE;
   lResult = ::RegSetValueEx(hKey, lpszEntry, NULL, REG_SZ,
      (LPBYTE)lpszValue, (lstrlen(lpszValue)+1)*sizeof(TCHAR));
   ::RegCloseKey(hKey);
   return lResult == ERROR_SUCCESS;
}

BOOL CReg::WriteExpandString(HKEY Branch, 
                             LPCTSTR lpszSection, 
                             LPCTSTR lpszEntry,
                             LPCTSTR lpszValue)
{
   ASSERT(AfxIsValidString(lpszSection));
   ASSERT(AfxIsValidString(lpszEntry));
   ASSERT(AfxIsValidString(lpszValue));
   LONG lResult;
   HKEY hKey = CreateSectionKey(Branch,lpszSection);
   if( hKey==NULL ) return FALSE;
   lResult = ::RegSetValueEx(hKey, lpszEntry, NULL, REG_EXPAND_SZ,
      (LPBYTE)lpszValue, (lstrlen(lpszValue)+1)*sizeof(TCHAR));
   ::RegCloseKey(hKey);
   return lResult == ERROR_SUCCESS;
}

BOOL CReg::WriteBinary(HKEY Branch,
                       LPCTSTR lpszSection, 
                       LPCTSTR lpszEntry,
                       LPBYTE pData, 
                       UINT nBytes)
{
   ASSERT(AfxIsValidString(lpszSection));
   ASSERT(AfxIsValidString(lpszEntry));
   ASSERT(pData!=NULL);
   LONG lResult;
   HKEY hKey = CreateSectionKey(Branch,lpszSection);
   if( hKey==NULL ) return FALSE;
   lResult = ::RegSetValueEx(hKey, lpszEntry, NULL, REG_BINARY,
      pData, nBytes);
   ::RegCloseKey(hKey);
   return lResult == ERROR_SUCCESS;
}

LONG CReg::DeleteValue(HKEY Branch,  LPCTSTR lpszSection, LPCTSTR lpszValue)
{
   ASSERT(AfxIsValidString(lpszSection));
   ASSERT(AfxIsValidString(lpszValue));
   LONG lResult;
   HKEY hKey = GetSectionKey(Branch,lpszSection,KEY_WRITE|KEY_READ);
   lResult = ::RegDeleteValue(hKey, lpszValue);
   ::RegCloseKey(hKey);
   return lResult; // == ERROR_SUCCESS;
};

BOOL CReg::DeleteKey(HKEY Branch, 
                LPCTSTR lpszSection, 
                LPCTSTR lpszEntry)
{
   ASSERT(AfxIsValidString(lpszSection));
   ASSERT(AfxIsValidString(lpszEntry));
   LONG lResult;
   HKEY hKey = GetSectionKey(Branch,lpszSection,KEY_WRITE|KEY_READ);
   lResult = ::RegDeleteKey(hKey, lpszEntry);
   ::RegCloseKey(hKey);
   return lResult == ERROR_SUCCESS;
};


LONG CReg::DeleteSection(HKEY hParentKey, const CString& strKeyName)
// Under Win32, a reg key may not be deleted unless it is empty.
// Thus, to delete a tree,  one must recursively enumerate and
// delete all of the sub-keys.
{
#ifdef CWinApp__DelRegTree
   return CWinApp::DelRegTree(hParentKey,strKeyName);
#else
   #define MAX_KEY_LENGTH 300
   DWORD   dwIndex = 0L;
   TCHAR   szSubKeyName[MAX_KEY_LENGTH];
   HKEY    hCurrentKey;
   DWORD   dwResult;

   if ((dwResult = ::RegOpenKey(hParentKey, strKeyName, &hCurrentKey)) ==
      ERROR_SUCCESS) {
      // Remove all subkeys of the key to delete
      while ((dwResult = ::RegEnumKey(hCurrentKey, 0, szSubKeyName, MAX_KEY_LENGTH-1)) ==
            ERROR_SUCCESS) {
         if ((dwResult = DeleteSection(hCurrentKey, szSubKeyName)) != ERROR_SUCCESS)
            break;
      };
      // If all went well, we should now be able to delete the requested key
      if ((dwResult == ERROR_NO_MORE_ITEMS) || (dwResult == ERROR_BADKEY)) {
         dwResult = ::RegDeleteKey(hParentKey, strKeyName);
      };
   };
   RegCloseKey(hCurrentKey);
   return dwResult;
#endif
}

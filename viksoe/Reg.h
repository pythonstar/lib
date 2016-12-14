// Reg.h: interface for the CReg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REG_H__08D42A36_E386_11D1_93C1_501808C10000__INCLUDED_)
#define AFX_REG_H__08D42A36_E386_11D1_93C1_501808C10000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
//
// Registry class
//
// Description:
//   Implements helper functions to access
//   the system registry under Win32.
//
//
// Written by Bjarke Viksoe (bjarke@viksoe.dk)
// Copyright (c) 2000.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage to you or your
// computer whatsoever. It's free, so don't hassle me about it.
//
// Beware of bugs.
//
// System Registry wrapper class.
class CReg : public CObject  
{
public:
   CReg();
   CReg(HKEY hKey, LPCTSTR Section);
   virtual ~CReg();

// Operations
public:
   // Sets the current registry branch.
   BOOL SetRegistryBranch(HKEY hKey, LPCTSTR Section);
   //
   // Reads an integer from the system registry.
   UINT GetInt(HKEY Branch, LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault=0);
   // Reads an integer from the system registry.
   UINT GetInt(LPCTSTR lpszEntry, int nDefault=0);
   // Reads a string from the system registry.
   CString GetString(HKEY Branch, LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault=NULL);
   // Reads a string from the system registry.
   CString GetString(LPCTSTR lpszEntry, LPCTSTR lpszDefault=NULL);
   // Reads a binary lump of data from the system registry.
   BOOL GetBinary(HKEY Branch, LPCTSTR lpszSection, LPCTSTR lpszEntry, BYTE** ppData, UINT* pBytes);
   // Writes an integer to the system registry.
   BOOL WriteInt(HKEY Branch, LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue);
   // Writes a string to the system registry.
   BOOL WriteString(HKEY Branch, LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue);
   // Writes binary to the system registry.
   BOOL WriteBinary(HKEY Branch, LPCTSTR lpszSection, LPCTSTR lpszEntry, LPBYTE pData, UINT nBytes);
   // Writes an 'expand string' to the system registry.
   BOOL WriteExpandString(HKEY Branch, LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue);
   // Delete a registry key.
   BOOL DeleteKey(HKEY Branch, LPCTSTR lpszSection, LPCTSTR lpszEntry);
   // Deletes a value
   // ** Added by Jörg Anslik. Thanks.
   LONG DeleteValue(HKEY Branch,  LPCTSTR lpszSection, LPCTSTR lpszValue)
   // Delete an entire registry branch.
   LONG DeleteSection(HKEY hParentKey, const CString& strKeyName);

// Implementation
private:
   HKEY GetSectionKey(HKEY Branch, LPCTSTR lpszSection, long lPermission=KEY_WRITE|KEY_READ);
   HKEY CreateSectionKey(HKEY Branch, LPCTSTR lpszSection);

// Attributes
protected:
   HKEY    m_hKey;             // The current registry root.
   CString m_strSection;       // The current registry branch.
};

#endif // !defined(AFX_REG_H__08D42A36_E386_11D1_93C1_501808C10000__INCLUDED_)

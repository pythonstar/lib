// Ini.h: interface for the CIni class.
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
////////////////////////////////////////////////////////////////////////

#if !defined(AFX_INI_H__2478E9E2_E904_11D1_93C1_241C08C10000__INCLUDED_)
#define AFX_INI_H__2478E9E2_E904_11D1_93C1_241C08C10000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
//
// INI file class
//
// Author:
// Bjarke Viksøe
// Description:
// Implements helper functions to access
// an .INI configuration file using
// conventional CString operations
//

// Ini-file wrapper class
class CIni : public CObject  
{
public:
   CIni();
   CIni( LPCTSTR IniFilename );
   virtual ~CIni();

// Methods
public:
   // Sets the current Ini-file to use.
   RETCODE SetIniFilename(LPCTSTR IniFilename);
   //
   // Reads an integer from the ini-file.
   UINT GetInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault=0);
   // Reads a boolean value from the ini-file.
   BOOL GetBoolean(LPCTSTR lpszSection, LPCTSTR lpszEntry, BOOL bDefault=FALSE);
   // Reads a string from the ini-file.
   CString GetString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault=NULL);
   // Reads a binaryt lump of data from the ini-file.
   BOOL GetBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry, BYTE** ppData, UINT* pBytes);
   //
   // Writes an integer to the ini-file.
   BOOL WriteInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue);
   // Writes a boolean value to the ini-file.
   BOOL WriteBoolean(LPCTSTR lpszSection, LPCTSTR lpszEntry, BOOL bValue);
   // Writes a string to the ini-file.
   BOOL WriteString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue);
   // Writes a binary lump of data to the ini-file.
   BOOL WriteBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPBYTE pData, UINT nBytes);
   // Writes an 'expand string' to the ini-file.
   BOOL WriteExpandString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue);
   //
   // Removes an item from the current ini-file.
   BOOL DeleteKey(LPCTSTR lpszSection, LPCTSTR lpszEntry);
   // Removes a complete section from the ini-file.
   BOOL DeleteSection(LPCTSTR lpszSection);

// Variables
protected:
   CString m_IniFilename; // The current ini-file used.
};

#endif // !defined(AFX_INI_H__2478E9E2_E904_11D1_93C1_241C08C10000__INCLUDED_)

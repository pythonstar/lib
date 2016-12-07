// Ini.cpp: implementation of the CIni class.
// Author: Bjarke Viksøe
//
// Description:
// Thin wrapper around the Win32 Windows Profile (Ini-file configuration)
// interface.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Ini.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIni::CIni()
{
   m_IniFilename.Empty();
}

CIni::CIni(LPCTSTR IniFilename)
{
   SetIniFilename( IniFilename );
}

CIni::~CIni()
{
   // Flush .ini file
   // (This should perhaps not be here. We risk to slow
   //  down the system and this would be done at a more appropriate
   //  time by the OS scheduler anyway)
   ::WritePrivateProfileString( NULL, NULL, NULL, m_IniFilename );
}


//////////////////////////////////////////////////////////////////////
// Methods
//////////////////////////////////////////////////////////////////////

#define MAX_INI_BUFFER 300   // Defines the maximum number of chars we can
                             // read from the ini file 

RETCODE CIni::SetIniFilename(LPCTSTR IniFilename)
{
   ASSERT(AfxIsValidString(IniFilename));
   m_IniFilename = IniFilename;
   if( m_IniFilename.IsEmpty() ) return RET_INVALIDARGS;
   return RET_OK;
};


UINT CIni::GetInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault)
{
   ASSERT(AfxIsValidString(lpszSection));
   ASSERT(AfxIsValidString(lpszEntry));
   if( m_IniFilename.IsEmpty() ) return 0; // error
   CString sDefault;
   sDefault.Format( _T("%d"), nDefault );
   CString s = GetString( lpszSection, lpszEntry, sDefault );
   return _ttol( s );
};

CString CIni::GetString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault)
{
   ASSERT(AfxIsValidString(lpszSection));
   ASSERT(AfxIsValidString(lpszEntry));
   if( m_IniFilename.IsEmpty() ) return CString();
   CString s;
   long ret = ::GetPrivateProfileString( lpszSection, lpszEntry, lpszDefault, s.GetBuffer( MAX_INI_BUFFER ), MAX_INI_BUFFER, m_IniFilename );
   s.ReleaseBuffer();
   if( ret==0 ) return CString(lpszDefault);
   return s;
};

BOOL CIni::GetBoolean(LPCTSTR lpszSection, LPCTSTR lpszEntry, BOOL bDefault)
{
   CString s = GetString(lpszSection,lpszEntry);
   if( s.IsEmpty() ) return bDefault;
   TCHAR c = _totupper( s[0] );
   switch( c ) {
   case _T('Y'): // YES
   case _T('1'): // 1 (binary)
   case _T('O'): // OK
      return TRUE;
   default:
      return FALSE;
   };
};

BOOL CIni::GetBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry, BYTE** ppData, UINT* pBytes)
{
   ASSERT(AfxIsValidString(lpszSection));
   ASSERT(AfxIsValidString(lpszEntry));
   return FALSE;
};

BOOL CIni::WriteInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue)
{
   ASSERT(AfxIsValidString(lpszSection));
   ASSERT(AfxIsValidString(lpszEntry));
   CString s;
   s.Format( _T("%d"), nValue );
   return WriteString( lpszSection, lpszEntry, s );
};

BOOL CIni::WriteBoolean(LPCTSTR lpszSection, LPCTSTR lpszEntry, BOOL bValue)
{
   CString s;
   bValue ? s=_T("Y") : s=_T("N");
   return WriteString( lpszSection, lpszEntry, s );
};

BOOL CIni::WriteString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue)
{
   ASSERT(AfxIsValidString(lpszSection));
   ASSERT(AfxIsValidString(lpszEntry));
   if( m_IniFilename.IsEmpty() ) return RET_NOTINITIALIZED;
   return ::WritePrivateProfileString( lpszSection, lpszEntry, lpszValue, m_IniFilename );
};

BOOL CIni::WriteBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPBYTE pData, UINT nBytes)
{
   ASSERT(AfxIsValidString(lpszSection));
   ASSERT(AfxIsValidString(lpszEntry));
   return FALSE;
};

BOOL CIni::WriteExpandString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue)
{
   ASSERT(AfxIsValidString(lpszSection));
   ASSERT(AfxIsValidString(lpszEntry));
   return FALSE;
};

BOOL CIni::DeleteKey(LPCTSTR lpszSection, LPCTSTR lpszEntry)
{
   ASSERT(AfxIsValidString(lpszSection));
   ASSERT(AfxIsValidString(lpszEntry));
   if( m_IniFilename.IsEmpty() ) return RET_NOTINITIALIZED;
   return ::WritePrivateProfileString( lpszSection, lpszEntry, NULL, m_IniFilename );
};

BOOL CIni::DeleteSection(LPCTSTR lpszSection)
{
   ASSERT(AfxIsValidString(lpszSection));
   if( m_IniFilename.IsEmpty() ) return RET_NOTINITIALIZED;
   return ::WritePrivateProfileString( lpszSection, NULL, NULL, m_IniFilename );
};


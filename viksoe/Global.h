#if !defined(AFX_GLOBAL_H__F3D3F542_98D7_11D1_93C1_444553540000__INCLUDED_)
#define AFX_GLOBAL_H__F3D3F542_98D7_11D1_93C1_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// GLOBAL.h : header file
// Begun: 5 feb 1998 v0.0 Bjarke Viksøe
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

#pragma comment(lib, "mpr.lib")

/////////////////////////////////////////////////////////////////////////////
// GLOBAL MACROS

// Old OLE2 string conversion macros
// Use the ATL string conversion macros instead (look up USES_CONVERSION; in help)
#define TOUNICODE(szX,wszX) if(szX) ::MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szX, strlen(szX)+1, wszX, sizeof(wszX)/sizeof(wszX[0]) )
#define TOANSI(wszX,szX)    if(wszX) ::WideCharToMultiByte( CP_ACP, 0, wszX, -1, szX, sizeof(szX), NULL, NULL)

// Add backslash to string
#define ADDBACKSLASH(s) if(s.Right(1)!=_T('\\')) s+=_T('\\');
#define REMOVEBACKSLASH(s) if(s.Right(1)==_T('\\')) s=s.Left(s.GetLength()-1);

/////////////////////////////////////////////////////////////////////////////
// GLOBAL DEBUG MACROS

#ifdef _DEBUG

#define TRACEFUNCTION(x) TRACE1("--- %s\n", x);
#define BEGIN_TICK                    \
      { DWORD dwBegin = ::GetTickCount();
#define END_TICK(s)                   \
      DWORD dwEnd = ::GetTickCount(); \
      TRACE2("%s: tick count = %d\n", s, dwEnd-dwBegin); }

#else

#define TRACEFUNCTION(x)
#define BEGIN_TICK
#define END_TICK(s)

#endif

/////////////////////////////////////////////////////////////////////////////
// GLOBAL CONSTANTS

#ifdef RETCODE
#undef RETCODE
#endif

// Return codes
typedef enum {
   RET_OK = 0,
   RET_ERROR = -1,
   RET_UNSUPPORTED = -2,
   RET_NOTIMPLEMENTED = -3,
   RET_OUTOFMEMORY = -4,
   RET_OUTOFRESOURCES = -5,
   RET_UNKNOWN = -6,
   RET_ALREADYDEFINED = -7,
   RET_CONNECTERROR = -8,
   RET_WRONGUSERNAME = -9,
   RET_WRONGPASSWORD = -10,
   RET_WRONGNAME = -11,
   RET_CANNOTCREATE = -12,
   RET_CANNOTFIND = -13,
   RET_TIMEOUT = -14,
   RET_NOTFOUND = -15,
   RET_NOTINITIALIZED = -16,
   RET_UNKNOWNTYPE = -17,
   RET_NODATA = -18,
   RET_NOTOPEN = -19,
   RET_FILEERROR = -31,
   RET_EMPTY = -32,
   RET_PATHERROR = -34,
   RET_BUSY = -35,
   RET_NOACCESS = -36,
   RET_NORIGHTS = -37,
   //
   RET_INVALIDARGS = -100,
   RET_BADMEMORY = -101,
   RET_BADTYPE = -102,
   RET_BADFILE = -103,
   RET_BADID = -104,
   RET_BADFILEID = -105,
   RET_BADDOMAINID = -106,
   RET_BADFILENAME = -107,
   RET_BADNAME = -108,
   RET_BADSECTORID = -109,
   RET_BADFORMAT = -110,
   RET_TOOLARGE = -111,
   //
   RET_SQLERROR = -200,
   RET_SQLFAILED = -201,
   RET_SQLDOWN = -202,
   RET_SQLNOCONNECTION = -203,
   RET_SQLTIMEOUT = -204,
   RET_KILLED
} RETCODE;

/////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTIONS

BOOL    BfxFileExists( LPCTSTR sFilename );
BOOL    BfxPathExists( LPCTSTR sPath );
CString BfxGetAppPath(void);
CString BfxGetFilePath( LPCTSTR Filename );
DWORD   BfxGetFileSize( LPCTSTR Filename );

CString BfxRemoveLine( CString &str );
CString BfxRemoveToken( CString &str, LPCTSTR token );
CString BfxRemoveToken( CString &str, TCHAR token );

BOOL    BfxIsSingleInstance( LPCTSTR AppTitle=NULL );
CString BfxGetErrorMessage( long ErrCode=-1 );

void    BfxExitApp(void);


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GLOBAL_H__F3D3F542_98D7_11D1_93C1_444553540000__INCLUDED_)

// Global.cpp: implementation of global support functions.
// All functions must be prefixed "Bfx" !!!
// Begun: 21 feb 1998 v0.0 Bjarke Viksøe
//
// Requires:
//   MPR.LIB
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#pragma comment( exestr, "Library by Bjarke Viksøe. Compiled on " __DATE__ " at " __TIME__ )


CString BfxRemoveLine( CString &str )
// Removes a line from a large multiline string
// 'str' is a string with mulitple lines seperated with \n or \r\n
// characters
{
   return BfxRemoveToken( str, _T("\r\n") );
};

CString BfxRemoveToken( CString &str, LPCTSTR token )
// Function name   : BfxRemoveToken
// Description       : Parses the string and removes a substring from it.
// Return type      : CString 
// Argument         :  CString &str
// Argument         : LPCTSTR token
// 
// Remarks:
// The substring is seperated by the token
// Eg BfxRemoveToken( "ABC::DEF", "::" ) return ABC 
// and leaves DEF in argument 1
{
   int pos = str.Find( token );
   if( pos<0 ) {
      CString temp( str );
      str.Empty();
      return temp;
   }
   else {
      CString temp( str.Left(pos) );
      str = str.Mid( pos + _tcslen(token) );
      return temp;
   };
};

CString BfxRemoveToken( CString &str, TCHAR token )
// Function name     : BfxRemoveToken
// Description        : Parses the string and removes a substring from it.
// Return type        : CString 
// Argument         : CString &str
// Argument         : TCHAR token
//
// Remarks:
// The substring is seperated by the token
// Eg BfxRemoveToken( "ABC::DEF", "::" ) return ABC 
// and leaves DEF in argument 1
{
   int pos = str.Find( token );
   if( pos<0 ) {
      CString temp( str );
      str.Empty();
      return temp;
   }
   else {
      CString temp( str.Left(pos) );
      str = str.Mid( pos + 1 );
      return temp;
   };
};

BOOL BfxFileExists( LPCTSTR sFilename )
// Function name   : BfxFileExists
// Description      : Simple FileExists(...) function that makes a DIR$ 
//                  for the file.
// Return type      : BOOL 
// Argument       : LPCTSTR sFilename
//
// Remarks:
// Can actually accept wildcards, though no filename is returned.
{
   ASSERT( sFilename );
   CFileFind f;
   CString Filename( sFilename );
   BOOL res = f.FindFile( Filename );
   f.Close();
   return res;
};

BOOL BfxPathExists( LPCTSTR sPath )
// Function name   : BfxPathExists
// Description      : PathExists(...) function that makes a DIR$ 
//                  for the specific path.
// Return type      : BOOL 
// Argument       : LPCTSTR sFilename
{
   ASSERT( sPath );
   CFileFind f;
   CString Path( sPath );
   BOOL res = f.FindFile( Path );
   if( res ) { f.FindNextFile(); res = f.IsDirectory(); };
   f.Close();
   return res;
};

CString BfxGetAppPath()
// Function name   : BfxGetAppPath
// Description      : Returns the application path 
// Return type      : CString 
//
// Remarks:
// Only works for EXE and not DLL (returns the path for the calling process)
{
   CString s;
#ifndef _WINDLL
   int len = ::GetModuleFileName( NULL, s.GetBuffer(MAX_PATH), MAX_PATH );
#else
   int len = ::GetModuleFileName( ::GetModuleHandle(AfxGetAppName()), s.GetBuffer(MAX_PATH), MAX_PATH );
#endif
   ASSERT(len>0);
   s.ReleaseBuffer( len );
   int pos = s.ReverseFind(_T('\\'));
   ASSERT(pos>=0);
   if( pos>1 ) s = s.Left( pos+1 );
   ADDBACKSLASH(s);
   return s;
};

CString BfxGetFilePath( LPCTSTR Filename )
// Function name   : BfxGetFilePath
// Description      : Get the path part of a complete filename
// Return type      : CString 
// Argument       : LPCTSTR Filename
//
// Remarks:
// Returns the path component of a complete filename
// This function uses the infamous GetFullPathName() Win32 function
// to calculate the filename part.
// If you just supply a filename, it will add the current drive and path
// and return this!
{
   CString s;
   LPTSTR p;
   DWORD len = ::GetFullPathName( Filename, MAX_PATH, s.GetBuffer(MAX_PATH), &p );
   if( p!=NULL ) len -= _tcslen(p)*sizeof(TCHAR); // substract
   if( len<0 ) len=0;
   s.ReleaseBuffer(len);
   if( s.IsEmpty() ) return CString();
   return s;
};


DWORD BfxGetFileSize( LPCTSTR Filename )
// Function name   : BfxGetFileSize
// Description      : 
// Return type      : DWORD = lower 32-bit size of file
// Argument       : LPCTSTR Filename
{
   HANDLE file; 
   file = ::CreateFile(Filename, 
                       GENERIC_READ, 
                       FILE_SHARE_READ, 
                       NULL, 
                       OPEN_EXISTING, 
                       FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, 
                       NULL); 
    if (file==INVALID_HANDLE_VALUE) return 0;
    DWORD size = ::GetFileSize(file, NULL); 
    ::CloseHandle(file);
    return size;
};

BOOL BfxIsSingleInstance( LPCTSTR AppTitle/*=NULL*/ )
// Function name   : BfxIsSingleInstance
// Description      : Makes sure only one instance of the application is
//                  running.
// Return type      : BOOL 
// Argument       : LPCTSTR AppTitle /*=NULL*/
//
// Remarks:
// This is done in the old traditional Win32 style by creating a mutex
// (or an event in this case) which is persistent in the system.
// Trying to create another, will fail.
// Call thing function only once pr instance (e.g in InitInstance())
{
   HANDLE hEvent;
   hEvent = ::CreateEvent(NULL,FALSE,FALSE, (AppTitle==NULL ? AfxGetAppName() : AppTitle ));
   if( hEvent==NULL ) {
      // Something REALLY went wrong
      return FALSE;
   };
   if( ::GetLastError()==ERROR_ALREADY_EXISTS ) {
      // Some other instance is running!
      ::CloseHandle( hEvent );
      return FALSE;
   };
   // System closes handle automatically when process terminates
   return TRUE;
};

CString BfxGetErrorMessage( long ErrCode/*=-1*/ )
// Function name   : BfxGetErrorMessage
// Description      : Get the system error message
// Return type      : CString 
// Argument       : long ErrCode /*=-1*/ = If ErrCode is -1 the read the last
//                  error returned from the Windows OS
{
   CString s;
   if( ErrCode==-1 ) ErrCode = ::GetLastError();
   LPCTSTR lpBuffer;
   if( ::FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER,
                   NULL,
                   ErrCode,
                   MAKELANGID(LANG_NEUTRAL,SUBLANG_SYS_DEFAULT),
                   (LPTSTR)&lpBuffer,
                   0,
                   NULL) != 0 ) {
      s = lpBuffer;
      ::LocalFree((HLOCAL)lpBuffer);
      return s;
   }
   else {
      // Unknown error
      return CString();
   };
};

void BfxExitApp()
// Function name   : BfxExitApp
// Description      : Attempt to gracefully exit the application
// Return type      : void 
// Description:
// This function can be used to make the application exit
// right away.  The preferred method is still to
// post the Quit-message and let the MFC message handler
// close all the objects properly.
{
   ASSERT(AfxGetMainWnd()!=NULL);
   AfxPostQuitMessage(0);
   ::Sleep(0);
   // If we get here, we couldn't close after all and we
   // do it our-selves
   AfxWinTerm();
   exit(0);
};

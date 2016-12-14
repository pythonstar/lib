// Dir.cpp: implementation of the CDir class.
// By Bjarke Viksøe
// Description:
//   Directory (folder) functions 
// Ver:
//   1.0   10 May.1998 Started
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Dir.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDir::CDir()
{
}

CDir::~CDir()
{
}


//////////////////////////////////////////////////////////////////////
// Methods
//////////////////////////////////////////////////////////////////////

RETCODE CDir::Create(LPCTSTR Path)
// Function name   : CDir::Create
// Description      : Create a directory (even with subfolders)
// Return type      : RETCODE 
// Argument       : LPCTSTR Path
{
   ASSERT(AfxIsValidString(Path));
   CString Components( Path );
   CString NewPath;
   // Validate
   if( Components.GetLength()<=3 ) return RET_INVALIDARGS;
   // Parse all components of the path and create one sub-dir at
   // a time, starting with C:\ ...
   // We slowly build 'NewPath' from C:\ to the complete path
   // creating the directory every time we add a new sub-dir.
   // ::CreateDirectory(...) may fail, but we don't care! We simply
   // check in the end if the path was created and return success!!!
   while( !Components.IsEmpty() ) {
      NewPath += BfxRemoveToken( Components, _T('\\') );
      if( NewPath.GetLength()>2 ) // Actually skip CreateDirectory( "C:" )
                                 // since it is useless
         ::CreateDirectory( NewPath, NULL );
      ADDBACKSLASH( NewPath );
   };   
   // Strip backslash
   NewPath = NewPath.Left( NewPath.GetLength()-1 );
   // and make sure new path is there
   if (::GetFileAttributes( NewPath )==0xFFFFFFFF) return RET_FILEERROR; else return RET_OK;
};

RETCODE CDir::Delete(LPCTSTR Path)
// Function name   : CDir::Delete
// Description      : Removes a dircetory (and all sub-directories)
// Return type      : RETCODE 
// Argument       : LPCTSTR Path
{
   ASSERT(AfxIsValidString(Path));
   CString sPath( Path );
   if( sPath.IsEmpty() ) return RET_OK;
   ADDBACKSLASH( sPath );
   // Find files and dircetories
   CFileFind ff;
   CStringArray SubPaths;
   BOOL bFound;
   bFound = ff.FindFile( sPath + _T("*.*") );
   while( bFound ) {
      bFound = ff.FindNextFile();
      if( ff.IsDots() ) continue;
      if( ff.IsDirectory() ) {
         // Cache all found sub-directories
         SubPaths.Add( ff.GetFilePath() );
      }
      else {
         // Delete actual files right away
         ::DeleteFile( ff.GetFilePath() );
      };
   };
   ff.Close();
   // Now recursively remove all sub-directories
   for( int i=0; i<SubPaths.GetSize(); i++ ) {
      Delete( SubPaths[i] );
   };
   // Finally remove the directory entry itself
   sPath = sPath.Left( sPath.GetLength()-1 ); // remove backslash
   ::RemoveDirectory( sPath );
   return RET_OK;
};


RETCODE CDir::Copy(LPCTSTR From, 
                   LPCTSTR To, 
                   BOOL bIncludeSubs/*=TRUE*/)
// Function name   : CDir::Copy
// Description      : Copies a directory (and optionally subdirectories)
// Return type      : RETCODE 
// Argument       : LPCTSTR From
// Argument       : LPCTSTR To
// Argument       : BOOL bIncludeSubs
{
   return RET_NOTIMPLEMENTED;
};


CString CDir::SearchFile(CStringArray &Paths, LPCTSTR Filename)
// Function name    : CDir::SearchFile
// Description       : Scans the paths supplied in the 'Paths' array for the
//                   file.
// Return type       : CString 
// Argument        : CStringArray &Paths
// Argument        : LPCTSTR Filename
{
   ASSERT(AfxIsValidString(Filename));
   for( int i=0; i<Paths.GetSize(); i++ ) {
      CString sFilename = Paths[i];
      CString sBuffer;
      LPTSTR lpFilePart;
      DWORD len = ::SearchPath(sFilename,Filename,NULL,MAX_PATH,sBuffer.GetBuffer(MAX_PATH),&lpFilePart);
      sBuffer.ReleaseBuffer();
      if( len>0 ) return sBuffer;
   };
   return CString(); // nothing found
};

BOOL CDir::ExpandPath(CString &Path)
// Function name   : CDir::ExpandPath
// Description      : Expands a path to the fully qualified path. E.g.
//                  The path "\DOS\..\WINNT" becomes "C:\WINNT\".
// Return type      : BOOL 
// Argument       : CString &Path
{
   CString sBuffer;
   LPTSTR p;
   ADDBACKSLASH(Path);
   Path += _T("TEST.EXE");
   DWORD len = ::GetFullPathName(Path,MAX_PATH,sBuffer.GetBuffer(MAX_PATH),&p);
   if( len==0 ) return FALSE;
   *p = _T('\0'); // cut off faked filename
   sBuffer.ReleaseBuffer();
   Path = sBuffer;
   return TRUE;
};

BOOL CDir::ExpandFilename(CString &Filename)
// Function name   : CDir::ExpandFilename
// Description      : Expands a filename
// Return type      : BOOL 
// Argument       : CString &Filename
{
   CString sBuffer;
   LPTSTR p;
   DWORD len = ::GetFullPathName(Filename,MAX_PATH,sBuffer.GetBuffer(MAX_PATH),&p);
   if( len==0 ) return FALSE;
   sBuffer.ReleaseBuffer();
   Filename = sBuffer;
   return TRUE;
};

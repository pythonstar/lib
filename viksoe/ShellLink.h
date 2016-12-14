// ShellLink.h: interface for the CShellLink class.
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

#if !defined(AFX_LINK_H__ED3DA203_E759_11D1_93C1_241C08C10000__INCLUDED_)
#define AFX_LINK_H__ED3DA203_E759_11D1_93C1_241C08C10000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <shlobj.h>
#include <shellapi.h>
#pragma comment(lib, "shell32.lib")

class CShellLink : public CObject  
{
public:
   CShellLink();
   virtual ~CShellLink();

// Methods
public:
   RETCODE Init( int DesktopType );
   RETCODE Done(void);
   //
   BOOL    IsSupported(void) const;
   //
   RETCODE CreateFolder( LPCTSTR Title );
   RETCODE DeleteFolder( LPCTSTR Title );
   RETCODE FocusFolder( LPCTSTR Title );
   RETCODE CreateItem( LPCTSTR Title, 
                  LPCTSTR Filename=NULL, 
                  LPCTSTR Args=NULL,
                  LPCTSTR Folder=NULL,
                  LPCTSTR WorkPath=NULL, 
                  LPCTSTR IconPath=NULL,
                  long nCmdShow=3 );
   RETCODE DeleteItem( LPCTSTR Title, LPCTSTR Folder );

// Implementation
protected:
   RETCODE GetShellPath( CString &Path );
private:
   CString STRRETToCString(LPCITEMIDLIST pidl, LPSTRRET lpStr);

// Variables
private:
   int m_DesktopType;
};

#endif // !defined(AFX_LINK_H__ED3DA203_E759_11D1_93C1_241C08C10000__INCLUDED_)

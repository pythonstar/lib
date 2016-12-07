// Dir.h: interface for the CDir class.
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
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILEIO_H__ED3DA204_E759_11D1_93C1_241C08C10000__INCLUDED_)
#define AFX_FILEIO_H__ED3DA204_E759_11D1_93C1_241C08C10000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CDir : public CObject  
{
public:
   CDir();
   virtual ~CDir();

// Methods
public:
   RETCODE Create(LPCTSTR Path);
   RETCODE Delete(LPCTSTR Path);
   RETCODE Copy(LPCTSTR From, LPCTSTR To, BOOL bIncludeSubs=TRUE);
   CString SearchFile(CStringArray &Paths, LPCTSTR Filename);
   static BOOL ExpandPath(CString &Path);
   static BOOL ExpandFilename(CString &Filename);
};

#endif // !defined(AFX_FILEIO_H__ED3DA204_E759_11D1_93C1_241C08C10000__INCLUDED_)

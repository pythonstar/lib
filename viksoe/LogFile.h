// LogFile.h: interface for the CLogFile class.
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

#if !defined(AFX_LOGFILE_H__0EE45201_E8F9_11D1_93C1_241C08C10000__INCLUDED_)
#define AFX_LOGFILE_H__0EE45201_E8F9_11D1_93C1_241C08C10000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// A small class implementing a debug log file.
class CLogFile : public CObject  
{
public:
   CLogFile();
   CLogFile(LPCTSTR Filename);
   virtual ~CLogFile();

// Methods
public:
   // Creates (removes previous) log file.
   RETCODE Create(LPCTSTR Filename, LPCTSTR Text);
   // Set the filename of the log fil to use.
   RETCODE SetFilename(LPCTSTR Filename);
   // Creates or appends to an exisiting log file.
   RETCODE AppendText(LPCTSTR Text, ...);
   // Writes System Information to the log
   RETCODE LogSystemInformation();

// Variables
protected:
   CString m_Filename;     // The log file we're currently writing to.
};

#endif // !defined(AFX_LOGFILE_H__0EE45201_E8F9_11D1_93C1_241C08C10000__INCLUDED_)

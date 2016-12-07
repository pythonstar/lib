// ExecProg.h: interface for the CExecProg class.
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

#if !defined(AFX_EXECPROG_H__C93F5001_F88A_11D1_93C1_A41808C10000__INCLUDED_)
#define AFX_EXECPROG_H__C93F5001_F88A_11D1_93C1_A41808C10000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CExecProg : public CObject  
{
public:
   CExecProg();
   CExecProg( LPCSTR pszProgramName,
              LPCSTR pszParameter=NULL,
              BOOL   bWaitForTermination=FALSE,
              BOOL   bKeepProzessInfo=FALSE,
              LPCSTR pszConsoleTitle=NULL,
              BOOL   bUserInit=FALSE,
              size_t nCheckWaitTime=100 ) ;
   virtual ~CExecProg();

// Implementation
   static BOOL IsExecutable( LPCTSTR pszFilename ) ;
   virtual BOOL StartProgram( LPCSTR pszProgramName,
                        LPCSTR pszParameter=NULL,
                        BOOL   bWaitForTermination=FALSE,
                        BOOL   bKeepProzessInfo=FALSE,
                        LPCSTR pszConsoleTitle=NULL,
                        BOOL   bUserInit=FALSE,       // dont initialize m_Startinfo
                        size_t nCheckWaitTime=100  ); // only used when bWaitForTermination = TRUE
   CString GetLastError( long *pError=NULL ) const;
   BOOL OpenProcess(void);
   BOOL WaitForTermination( size_t nCheckWaitTime=100 );
   BOOL CheckForTermination( BOOL &bRet, size_t nCheckWaitTime=100 );
   BOOL ReleaseProcessInfo(void);
   BOOL ShellProgram( LPCTSTR    pszPath,
                  LPCTSTR    pszFile=NULL,
                  LPCTSTR    pszParameters=NULL,
                  LPCTSTR    pszDefaultDir=NULL,
                  UINT       nShowState=SW_SHOWNORMAL,
                  HINSTANCE *phInst=NULL,
                  CString   *pProgPath=NULL );

protected :
   // for the experts:
   // start a process (similar to the api call), but has defaults
   virtual BOOL CreateProcess ( LPPROCESS_INFORMATION  lpProcessInformation,                              // pointer to PROCESS_INFORMATION
                                LPSTARTUPINFO          lpStartupInfo,                                     // pointer to STARTUPINFO
                                LPCTSTR                lpApplicationName,                                 // pointer to name of executable module
                                LPTSTR                 lpCommandLine       = NULL,                       // pointer to command line string
                                LPSECURITY_ATTRIBUTES  lpProcessAttributes   = NULL,                       // pointer to process security attributes
                                LPSECURITY_ATTRIBUTES  lpThreadAttributes   = NULL,                       // pointer to thread security attributes
                                BOOL                   bInheritHandles       = FALSE,                      // handle inheritance flag
                                DWORD                  dwCreationFlags       = CREATE_DEFAULT_ERROR_MODE|NORMAL_PRIORITY_CLASS, // creation flags
                                LPVOID                 lpEnvironment       = NULL,                       // pointer to new environment block
                                LPCTSTR                lpCurrentDirectory   = NULL );                     // pointer to current directory name
   void SetError(void);
   void Init(void);
   void Clear(void);

// Variables
protected:
   STARTUPINFO          m_Startinfo;
   PROCESS_INFORMATION *m_pProzess;
   BOOL                 m_bCreated;
   CString              m_strProgramName;
   CString              m_strParameter;
   BOOL                 m_bWaiting;
   long                 m_nError;
   CString              m_strError;
   HANDLE               m_procHandle;
};

#endif // !defined(AFX_EXECPROG_H__C93F5001_F88A_11D1_93C1_A41808C10000__INCLUDED_)

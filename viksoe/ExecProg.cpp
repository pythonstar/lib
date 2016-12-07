// ExecProg.cpp: implementation of the CExecProg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ExecProg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CExecProg::CExecProg()
{
   Init();
   m_bCreated = TRUE;
}

CExecProg::~CExecProg()
{
   Clear();
}

CExecProg:: CExecProg( LPCSTR pszProgramName,
                       LPCSTR pszParameter,
                       BOOL   bWaitForTermination,
                       BOOL   bKeepProcessInfo,
                       LPCSTR pszConsoleTitle,
                       BOOL   bUserInit,
                       size_t nCheckWaitTime )
{
   Init();
   m_bCreated = StartProgram( pszProgramName,
                              pszParameter,
                              bWaitForTermination,
                              bKeepProcessInfo,
                              pszConsoleTitle,
                              bUserInit,
                              nCheckWaitTime );
};


//////////////////////////////////////////////////////////////////////
// Initialization

void CExecProg::Init()
{
   ::ZeroMemory( &m_Startinfo, sizeof(STARTUPINFO) );
   m_Startinfo.cb              = sizeof(STARTUPINFO);
   m_Startinfo.lpReserved      = NULL;
   m_Startinfo.lpDesktop       = NULL; // not used (only in Win NT)
   m_Startinfo.lpTitle         = NULL;
   m_Startinfo.dwX             = 0; // win position  ->dwFlags=STARTF_USEPOSITION
   m_Startinfo.dwY             = 0;
   m_Startinfo.dwXSize         = 0; // win size ->dwFlags=STARTF_USESIZE
   m_Startinfo.dwYSize         = 0;
   m_Startinfo.dwXCountChars   = 0; // console width ->dwFlags=STARTF_USECOUNTCHARS
   m_Startinfo.dwYCountChars   = 0; //   "     height
   m_Startinfo.dwFillAttribute = 0; // console text & backgr. color
   m_Startinfo.dwFlags         =  STARTF_FORCEONFEEDBACK;  // show hourglass
                        // STARTF_FORCEOFFFEEDBACK   // dont show hourglass
                        // STARTF_SCREENSAVER        // start as screensaver
                        // STARTF_USESTDHANDLES      // use the hStd.. handles (see help)
   m_Startinfo.wShowWindow     = SW_SHOWDEFAULT;  // show window state ->=STARTF_USESHOWWINDOW
   m_Startinfo.cbReserved2     = 0;
   m_Startinfo.lpReserved2     = NULL;
   m_Startinfo.hStdInput       = NULL; // input/output/error handles
   m_Startinfo.hStdOutput      = NULL; // only used if ->STARTF_USESTDHANDLES
   m_Startinfo.hStdError       = NULL;
   //
   m_pProzess = NULL ;
   m_bCreated = FALSE;
   m_bWaiting = FALSE;
   //
   m_procHandle = NULL;

   m_nError = 0;
};

void CExecProg::Clear()
{
   if ( m_pProzess!=NULL ) {
      delete m_pProzess;
      m_pProzess = NULL;
   }
};


//////////////////////////////////////////////////////////////////////
// Implementation

BOOL CExecProg::IsExecutable( LPCTSTR pszFilename )
{
   return TRUE;
};

BOOL CExecProg:: StartProgram ( LPCSTR pszProgramName,
                                LPCSTR pszParameter,
                                BOOL   bWaitForTermination,
                                BOOL   bKeepProcessInfo,
                                LPCSTR pszConsoleTitle, // = NULL
                                BOOL   bUserInit,       // dont initialize m_Startinfo
                                size_t nCheckWaitTime )
{
   BOOL bRet = ReleaseProcessInfo();
   if ( bRet ) { // this process is not already running
      // reset error
      m_nError = 0; 
      m_strError.Empty();
      //
      m_strProgramName      = pszProgramName;
      m_strParameter        = pszParameter;
      //
      m_Startinfo.cb        = sizeof( STARTUPINFO );
      m_Startinfo.lpTitle   = (LPTSTR)pszConsoleTitle;
      if ( !bUserInit ) {
         m_Startinfo.lpReserved      = NULL;
         m_Startinfo.lpDesktop       = NULL; // not used (only in Win NT)
         m_Startinfo.dwX             = 0; // win position  ->dwFlags=STARTF_USEPOSITION
         m_Startinfo.dwY             = 0;
         m_Startinfo.dwXSize         = 0; // win size ->dwFlags=STARTF_USESIZE
         m_Startinfo.dwYSize         = 0;
         m_Startinfo.dwXCountChars   = 0; // console width ->dwFlags=STARTF_USECOUNTCHARS
         m_Startinfo.dwYCountChars   = 0; //   "     height
         m_Startinfo.dwFillAttribute = 0; // console text & backgr. color
         m_Startinfo.dwFlags         =  STARTF_FORCEONFEEDBACK;  // show hourglass
                               // STARTF_FORCEOFFFEEDBACK  // dont show hourglass
                               // STARTF_SCREENSAVER       // start as screensaver
                               // STARTF_USESTDHANDLES     // use the hStd.. handles (see help)
         m_Startinfo.wShowWindow     = SW_SHOWDEFAULT;  // show window state ->=STARTF_USESHOWWINDOW
         m_Startinfo.cbReserved2     = 0;
         m_Startinfo.lpReserved2     = NULL;
         m_Startinfo.hStdInput       = NULL; // input/output/error handles
         m_Startinfo.hStdOutput      = NULL; // only used if ->STARTF_USESTDHANDLES
         m_Startinfo.hStdError       = NULL;
      };

      m_pProzess = new PROCESS_INFORMATION;
      bRet = CreateProcess( m_pProzess,
                       &m_Startinfo,
                       (LPTSTR)(LPCTSTR)m_strProgramName,
                       (LPTSTR)(LPCTSTR)m_strParameter );
      if ( bRet ) {
         if ( bWaitForTermination || bKeepProcessInfo ) {
            bRet = OpenProcess ();
         }
         if ( bRet ) {
            if ( bWaitForTermination ) {
               bRet = WaitForTermination ( nCheckWaitTime );
               bKeepProcessInfo = FALSE; // nothing to keep anymore
            }
         }
         if ( !bRet || !bKeepProcessInfo ) {
            delete m_pProzess;
            m_pProzess = NULL;
         }
      }
   }
   return bRet;
};

BOOL CExecProg::ReleaseProcessInfo()
{
   BOOL bRet = FALSE;
   if ( !m_bWaiting ) { // not waiting for termination
      if ( m_procHandle ) {
         ::CloseHandle( m_procHandle );
         m_procHandle = NULL;
      }
      delete m_pProzess;
      m_pProzess = NULL;
      bRet = TRUE;
   }
   return bRet;
};


BOOL CExecProg:: CreateProcess( LPPROCESS_INFORMATION  lpProcessInformation,  // pointer to PROCESS_INFORMATION
                                LPSTARTUPINFO          lpStartupInfo,         // pointer to STARTUPINFO
                                LPCTSTR                lpApplicationName,     // pointer to name of executable module
                                LPTSTR                 lpCommandLine,         // pointer to command line string
                                LPSECURITY_ATTRIBUTES  lpProcessAttributes,   // pointer to process security attributes
                                LPSECURITY_ATTRIBUTES  lpThreadAttributes,    // pointer to thread security attributes
                                BOOL                   bInheritHandles,       // handle inheritance flag
                                DWORD                  dwCreationFlags,       // creation flags
                                LPVOID                 lpEnvironment,         // pointer to new environment block
                                LPCTSTR                lpCurrentDirectory )   // pointer to current directory name
{
   ASSERT( lpApplicationName );
   BOOL bRet = FALSE;
   // Build Filename
   CString Commandline  = lpApplicationName;
   Commandline += ' ';
   Commandline += lpCommandLine;
   // Reset error
   m_nError = 0; 
   m_strError.Empty ();
   // Create the process
   bRet = ::CreateProcess( NULL,                             // pointer to name of executable module, not  used!
                       (LPTSTR)(LPCTSTR)Commandline,     // pointer to command line string
                       lpProcessAttributes,              // pointer to process security attributes
                       lpThreadAttributes,               // pointer to thread security attributes
                       bInheritHandles,                  // handle inheritance flag
                       dwCreationFlags,                  // creation flags
                       lpEnvironment,                    // pointer to new environment block
                       lpCurrentDirectory,               // pointer to current directory name
                       lpStartupInfo,                    // pointer to STARTUPINFO
                       lpProcessInformation );           // pointer to PROCESS_INFORMATION
   if ( !bRet ) SetError();
   return bRet;
};

BOOL CExecProg::OpenProcess()
{
   ASSERT( m_pProzess );
   BOOL bRet = FALSE;
   m_procHandle = NULL;

   if ( m_pProzess ) {
      m_nError = 0; // reset error
      m_strError.Empty();
      m_procHandle = ::OpenProcess ( PROCESS_QUERY_INFORMATION,
                                     TRUE,
                                     m_pProzess->dwProcessId );
      bRet = m_procHandle ? TRUE : FALSE;
      if ( !bRet ) SetError();
   };
   return bRet;
}

BOOL CExecProg::WaitForTermination( size_t nCheckWaitTime )
{
   BOOL bRet = TRUE;
   ASSERT( m_pProzess );
   ASSERT( m_procHandle );
   m_bWaiting = TRUE;
   while( !CheckForTermination( bRet, nCheckWaitTime ) && bRet )
      ::Sleep(0); //check for wm_quit
   ::CloseHandle( m_procHandle ); 
   m_procHandle = NULL;
   m_bWaiting = FALSE;
   return bRet;
};

BOOL CExecProg::CheckForTermination ( BOOL &bRet , size_t nCheckWaitTime )
{
   ASSERT( m_pProzess );
   ASSERT( m_procHandle );
   BOOL bTerminated = FALSE;
   // Reset error code
   m_nError = 0; 
   m_strError.Empty();
   DWORD retCode  = ::WaitForSingleObject( m_procHandle , nCheckWaitTime );
   if( retCode == WAIT_ABANDONED ) {     // The specified object is a mutex object that
                                         // was not released by the thread that owned the
      bRet = FALSE;                     // mutex object before the owning thread terminated.
                                  // Ownership of the mutex object is granted to the
      bTerminated = TRUE;               // calling thread, and the mutex is set to nonsignaled.
      SetError();
   }
   else if ( retCode == WAIT_OBJECT_0  ) { // { The state of the specified object is signaled.
      bTerminated = TRUE;
      DWORD exitCode = 0;
      bRet = ::GetExitCodeProcess( m_procHandle , &exitCode );
#ifdef _DEBUG
      BOOL test = exitCode != STILL_ACTIVE;
#endif
   };
   return bTerminated;
}



BOOL CExecProg:: ShellProgram( LPCTSTR pszPath,
                               LPCTSTR pszFile,
                               LPCTSTR pszParameters,
                               LPCTSTR pszDefaultDir,
                               UINT nShowState, // SW_...
                               HINSTANCE *phInst,
                               CString *pProgPath )
{
   ASSERT( AfxIsValidString(pszFile) );
   CString ProgPath;
   BOOL bRet = FALSE; // assume we didn't succeed
   HINSTANCE hInst = NULL;

   // Reset error
   m_nError = 0; 
   m_strError.Empty ();

   // Build filename
   CString Filename;
   if( pszPath!=NULL ) {
      Filename = pszPath;
      ADDBACKSLASH(Filename);
   };
   Filename += pszFile;
   if( Filename.GetLength()<0 ) return FALSE;

   HWND hWnd = AfxGetMainWnd()->m_hWnd;
   hInst = ::ShellExecute( hWnd,
                     _T("open"),
                     Filename,
                     pszParameters,
                     pszDefaultDir,
                     nShowState );
   switch( (long)hInst ) {
   case 0                        : // The operating system is out of memory or resources.
   case SE_ERR_ACCESSDENIED      : // Windows 95 only: The operating system denied access to the specified file.
   case SE_ERR_ASSOCINCOMPLETE   : // The filename association is incomplete or invalid.
   case SE_ERR_DDEBUSY           : // The DDE transaction could not be completed because other DDE transactions were being processed.
   case SE_ERR_DDEFAIL           : // The DDE transaction failed.
   case SE_ERR_DDETIMEOUT        : // The DDE transaction could not be completed because the request timed out.
   case SE_ERR_DLLNOTFOUND       : // Windows 95 only: The specified dynamic-link library was not found.
   case SE_ERR_FNF               : // Windows 95 only: The specified file was not found.
   case SE_ERR_NOASSOC           : // There is no application associated with the given filename extension.
   case SE_ERR_OOM               : // Windows 95 only: There was not enough memory to complete the operation.
   case SE_ERR_PNF               : // Windows 95 only: The specified path was not found.
   case SE_ERR_SHARE             : // A sharing violation occurred.
      break;
   default :
      bRet = TRUE;
      break;
   };
   if( !bRet ) SetError();
   // Done
   if( phInst ) *phInst = hInst;
   if( pProgPath ) *pProgPath = ProgPath;
   return bRet;
};

void CExecProg::SetError()
{
   m_nError = ::GetLastError();
   m_strError = BfxGetErrorMessage( m_nError );
};

CString CExecProg::GetLastError( long *pError /*=NULL*/ ) const
{
   if( pError!=NULL ) *pError = m_nError;
   return m_strError;
};

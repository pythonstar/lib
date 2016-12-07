// LogFile.cpp: implementation of the CLogFile class.
//
// A small class which can be used for debug logs.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LogFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLogFile::CLogFile()
{
}

CLogFile::CLogFile(LPCTSTR Filename)
{
   SetFilename( Filename );
}

CLogFile::~CLogFile()
{
}


//////////////////////////////////////////////////////////////////////
// Methods
//////////////////////////////////////////////////////////////////////

RETCODE CLogFile::Create(LPCTSTR Filename, LPCTSTR Text)
{
   ASSERT( Filename );
   ASSERT( Text );
   m_Filename = Filename;
   ASSERT( !m_Filename.IsEmpty() );
   if( m_Filename.IsEmpty() ) return RET_INVALIDARGS;
   // Write text to (text)file
   CStdioFile f;
   TRY 
   {
      BOOL res = f.Open( Filename, CFile::modeCreate|CFile::modeWrite|CFile::typeText );
      if( res ) {
         f.WriteString( Text );
         f.WriteString( _T("\n") );
         f.Close();
      };
   }
   CATCH_ALL(e) 
   {
      f.Abort();
#ifdef _DEBUG
      e->ReportError();
#endif
      return RET_ERROR;
   }
   END_CATCH_ALL;
   return RET_OK;
};

RETCODE CLogFile::AppendText(LPCTSTR Text, ...)
{
   ASSERT(AfxIsValidString(Text));
   ASSERT(!m_Filename.IsEmpty());
   if( m_Filename.IsEmpty() ) return RET_NOTINITIALIZED;
   // Append text to (text)file
   CStdioFile f;
   CString sText;
   va_list args;   
   va_start(args, Text);   
   sText.FormatV(Text, args);
   TRY 
   {
      BOOL res = f.Open( m_Filename, CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite|CFile::typeText );
      if( res ) {
         f.SeekToEnd();
         f.WriteString( sText );
         f.WriteString( _T("\n") );
         f.Close();
      };
   }
   CATCH_ALL(e) 
   {
      f.Abort();
#ifdef _DEBUG
      e->ReportError();
#endif
      return RET_FILEERROR;
   }
   END_CATCH_ALL;
   return RET_OK;
};

RETCODE CLogFile::SetFilename(LPCTSTR Filename)
// Sets the log filename.  A new log file will
// be created if the file does not exist.
{
   ASSERT(AfxIsValidString(Filename));
   m_Filename = Filename;
   if( m_Filename.IsEmpty() ) return RET_INVALIDARGS;
   return RET_OK;
}

RETCODE CLogFile::LogSystemInformation()
// Write some standard system information to
// the log file.
{
   ASSERT(!m_Filename.IsEmpty());
   if( m_Filename.IsEmpty() ) return RET_NOTINITIALIZED;

   SYSTEMTIME time;
   ::GetLocalTime( &time );
   AppendText(_T("Date: %04d-%02d-%02d Time: %02d:%02d:%02d"),
              time.wYear,time.wMonth,time.wDay,time.wHour,time.wMinute,time.wSecond);

   OSVERSIONINFO verinfo;
   verinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
   ::GetVersionEx(&verinfo);
   AppendText(_T("Win%s Version %d.%.2d (build %d) %s\n"), 
      (verinfo.dwPlatformId==VER_PLATFORM_WIN32_NT ? _T("NT") : _T("32")),
      verinfo.dwMajorVersion,
      verinfo.dwMinorVersion,
      verinfo.dwBuildNumber,
      verinfo.szCSDVersion);
  
   SYSTEM_INFO sysinfo; 
   LPCTSTR pszProcessor; 
   ::GetSystemInfo(&sysinfo); 
   switch( sysinfo.dwProcessorType ) { 
   case PROCESSOR_INTEL_386: 
   case PROCESSOR_INTEL_486: 
   case PROCESSOR_INTEL_PENTIUM: 
      pszProcessor = _T("Intel "); 
      break; 
   case PROCESSOR_MIPS_R4000: 
      pszProcessor = _T("MIPS R"); 
      break; 
   case PROCESSOR_ALPHA_21064: 
      pszProcessor = _T("DEC Alpha "); 
      break; 
   default: 
      pszProcessor = _T("Chipset "); 
      break; 
   } 
   return AppendText(_T("%s%d, %d Processor(s)\n"), 
      (LPCTSTR)pszProcessor, 
      sysinfo.dwProcessorType, 
      sysinfo.dwNumberOfProcessors);
};

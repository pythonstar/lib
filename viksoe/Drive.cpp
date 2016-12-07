// Drive.cpp: implementation of the CDrive class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Drive.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDrive::CDrive()
{
   if (!SetCurrentDrive()) SetDrive( _T('C') );
}

CDrive::CDrive( TCHAR Drive )
{
   if (!SetDrive( Drive )) if( Drive!=_T('C') ) SetDrive( _T('C') );
}

CDrive::CDrive( LPCTSTR Drive )
{
   if (!SetDrive( Drive )) SetDrive( _T('C') );
}

CDrive::CDrive( int Drive )
{
   if (!SetDrive( Drive )) SetDrive( _T('C') );
}

CDrive::~CDrive()
{
}

//////////////////////////////////////////////////////////////////////
// Methods
//////////////////////////////////////////////////////////////////////

BOOL CDrive::SetCurrentDrive()
{
   CString CurrentDir;
   LPTSTR p = CurrentDir.GetBuffer( MAX_PATH );
   int len = ::GetCurrentDirectory( MAX_PATH, p );
   CurrentDir.ReleaseBuffer( len );
   if( len==0 ) return FALSE; // some os error
   return SetDrive( CurrentDir[0] );
};

BOOL CDrive::SetDrive( TCHAR Drive )
// This is the real SetDrive() funciton which all
// other initialize function eventually will call
{
   Drive = _totupper( Drive );
   if( (Drive<_T('A')) || (Drive>_T('Z')) ) return FALSE;
   m_cDrive = Drive;
   m_strRoot = CString(m_cDrive) + _T(":\\");
   return TRUE;
};

BOOL CDrive::SetDrive( LPCTSTR Drive )
{
   ASSERT( Drive );
   if( !AfxIsValidString( Drive, 1 ) ) return FALSE;
   return SetDrive( Drive[0] );
};

BOOL CDrive::SetDrive( int Drive )
{
   return SetDrive( (TCHAR)Drive + _T('C') );
};

BOOL CDrive::SetNextFreeDrive()
{
   for( TCHAR Drive=_T('C'); Drive<_T('Z'); Drive++ ) {
      CString Path;
      Path = CString(Drive) + _T(":\\");
      if( ::GetDriveType(Path)<=1 ) {
         if( SetDrive( Drive ) ) return TRUE;
      };
   };
   return FALSE;
};

BOOL CDrive::SetFirstNetworkDrive()
{
   for( TCHAR Drive=_T('C'); Drive<_T('Z'); Drive++ ) {
      CString Path;
      CString Connection;
      DWORD len = 260;
      LPTSTR p = Connection.GetBuffer( len );
      Path = CString(Drive) + _T(':');
      ::WNetGetConnection( Path, p, &len );
      Connection.ReleaseBuffer(len);
      if( len>0 ) {
         return SetDrive( Drive );
      };
   };
   return FALSE;
};


CString CDrive::GetName() const
{
   CString Name;
   DWORD Flags;
   DWORD MaxComponent;
   ::GetVolumeInformation( m_strRoot,
                      Name.GetBuffer(200), 200,
                      NULL,
                      &MaxComponent,
                      &Flags,
                      NULL, 0 );
   Name.ReleaseBuffer();
   return Name;
};

CString CDrive::GetFileSystemName() const
{
   CString SystemName;
   DWORD Flags;
   DWORD MaxComponent;
   ::GetVolumeInformation( m_strRoot,
                      NULL, 0,
                      NULL,
                      &MaxComponent,
                      &Flags,
                      SystemName.GetBuffer(200), 200 );
   SystemName.ReleaseBuffer();
   return SystemName;
};

DWORD CDrive::GetSerialNumber() const
{
   DWORD Serial;
   DWORD Flags;
   DWORD MaxComponent;
   ::GetVolumeInformation( m_strRoot,
                      NULL, 0,
                      &Serial,
                      &MaxComponent,
                      &Flags,
                      NULL, 0 );
   return Serial;
};

UINT CDrive::GetType() const
{
   // Set error mode to trap errors
   // (DONE BY MFC!)
   //UINT nOldErrorMode = ::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
   // Get Drive type
   UINT type = ::GetDriveType(m_strRoot);
   // Reset error mode
   //::SetErrorMode(nOldErrorMode);   
   return type;
};

DWORD CDrive::GetComponentLength() const
{
   DWORD Flags;
   DWORD MaxComponent;
   ::GetVolumeInformation( m_strRoot,
                      NULL, 0,
                      NULL,
                      &MaxComponent,
                      &Flags,
                      NULL, 0 );
   return MaxComponent;
};

DWORD CDrive::GetAvailableSpace( DWORD *HighResult /*=NULL*/ ) const
// We must check if we are running NT or Win95-OSR2 or greater
// since we can then use the GetDiskFreeSpaceEx(...) function
// which we in addition must link dynamically!!!
{
   ULARGE_INTEGER FreeBytesAvailableToCaller;
   ULARGE_INTEGER TotalNumberOfBytes;
   ULARGE_INTEGER TotalNumberOfFreeBytes;
   OSVERSIONINFO os;
   ULARGE_INTEGER empty = {0,0};
   BOOL ok = FALSE;

   // Initialize if anything fails
   if( *HighResult!=NULL ) *HighResult=0L;
   // Check to see what OS version we are...
   os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
   GetVersionEx( &os );
   if( os.dwPlatformId==VER_PLATFORM_WIN32_NT ) ok = TRUE;
   if( (os.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS) && (LOWORD(os.dwBuildNumber)>1000) ) ok = TRUE;
   if ( ok ) {
      // Running WinNT, Win95-OSR2 or greater
      typedef BOOL (WINAPI *GETDISKFREEPROC)(LPCTSTR, PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER);
      // A pointer-to-function-with-the-return-type-and-parameters-of-GetDiskFreeSpaceExX
#ifndef _UNICODE
      GETDISKFREEPROC pfnGetFreeSpace = 
         (GETDISKFREEPROC)::GetProcAddress( ::GetModuleHandle(_T("KERNEL32.DLL")), 
         _T("GetDiskFreeSpaceExA"));
#else
      GETDISKFREEPROC pfnGetFreeSpace = 
         (GETDISKFREEPROC)::GetProcAddress( ::GetModuleHandle(_T("KERNEL32.DLL")), 
         _T("GetDiskFreeSpaceExW"));
#endif
      if( pfnGetFreeSpace==NULL ) goto OldSystem;
      // Prepare funciton call
      FreeBytesAvailableToCaller = empty;
      TotalNumberOfBytes = empty; 
      TotalNumberOfFreeBytes = empty; 
      // Call the function through the pointer.
      BOOL success = (*pfnGetFreeSpace)( m_strRoot,
                                 &FreeBytesAvailableToCaller, 
                                 &TotalNumberOfBytes, 
                                 &TotalNumberOfFreeBytes );
      if( !success ) return 0L;
      // We got it...
      if( HighResult!=NULL ) *HighResult = FreeBytesAvailableToCaller.HighPart;
      return FreeBytesAvailableToCaller.LowPart;
   }
   else {
OldSystem:
      // Not a GetDiskFreeSpaceEx(...) kind of OS...
      // So we'll just have to do with GetDiskFreeSpace(...)
      DWORD SectorsPerCluster;
      DWORD BytesPerSector;
      DWORD NumberOfFreeClusters;
      DWORD TotalNumberOfClusters;
      if( ::GetDiskFreeSpace( m_strRoot, &SectorsPerCluster, &BytesPerSector, &NumberOfFreeClusters, &TotalNumberOfClusters ) == FALSE ) return 0L; //Error
      if( HighResult!=NULL ) *HighResult=0L;
      return SectorsPerCluster*BytesPerSector*NumberOfFreeClusters;
   };
};

DWORD CDrive::GetSectorsPerCluster() const
{
   DWORD SectorsPerCluster;
   DWORD BytesPerSector;
   DWORD NumberOfFreeClusters;
   DWORD TotalNumberOfClusters;
   if( GetDiskFreeSpace( m_strRoot, &SectorsPerCluster, &BytesPerSector, &NumberOfFreeClusters, &TotalNumberOfClusters ) == FALSE ) return 0; //Error
   return SectorsPerCluster;
};

DWORD CDrive::GetBytesPerSector() const
{
   DWORD SectorsPerCluster;
   DWORD BytesPerSector;
   DWORD NumberOfFreeClusters;
   DWORD TotalNumberOfClusters;
   if( ::GetDiskFreeSpace( m_strRoot, &SectorsPerCluster, &BytesPerSector, &NumberOfFreeClusters, &TotalNumberOfClusters ) == FALSE ) return 0; //Error
   return BytesPerSector;
};

DWORD CDrive::GetNumberOfFreeClusters() const
{
   DWORD SectorsPerCluster;
   DWORD BytesPerSector;
   DWORD NumberOfFreeClusters;
   DWORD TotalNumberOfClusters;
   if( ::GetDiskFreeSpace( m_strRoot, &SectorsPerCluster, &BytesPerSector, &NumberOfFreeClusters, &TotalNumberOfClusters ) == FALSE ) return 0; //Error
   return NumberOfFreeClusters;
};

DWORD CDrive::GetTotalNumberOfClusters() const
{
   DWORD SectorsPerCluster;
   DWORD BytesPerSector;
   DWORD NumberOfFreeClusters;
   DWORD TotalNumberOfClusters;
   if( ::GetDiskFreeSpace( m_strRoot, &SectorsPerCluster, &BytesPerSector, &NumberOfFreeClusters, &TotalNumberOfClusters ) == FALSE ) return 0; //Error
   return TotalNumberOfClusters;
};


BOOL CDrive::HasLongFilenameSupport() const
{
   return( GetComponentLength()>=255 );
};

BOOL CDrive::HasCompressionSupport() const
{
   return ((GetFlags() & FS_FILE_COMPRESSION)>0) ;
};

BOOL CDrive::IsCompressed() const
{
   return ((GetFlags() & FS_VOL_IS_COMPRESSED)>0) ;
};

BOOL CDrive::IsCaseSensitive() const
{
   return ((GetFlags() & FS_CASE_SENSITIVE)>0) ;
};


CString CDrive::GetConnection() const
{
   CString Path;
   CString Connection;
   DWORD len = 260;
   LPTSTR p = Connection.GetBuffer(len);
   Path = CString(m_cDrive) + _T(':');
   ::WNetGetConnection( Path, p, &len );
   Connection.ReleaseBuffer(len);
   return Connection;
};

long CDrive::ConnectTo( LPCTSTR Unc, LPCSTR Username /*=NULL*/, LPCTSTR Password /*=NULL*/ )
{
   return 0;
};

long CDrive::RemoveConnection()
{
   return 0;
};

BOOL CDrive::IsConnected() const
{
   CString s( GetConnection() );
   return( s.IsEmpty() );
};




//////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////

DWORD CDrive::GetFlags() const
{
   DWORD Flags;
   DWORD MaxComponent;
   MaxComponent = Flags = 0;
   ::GetVolumeInformation( m_strRoot,
                      NULL, 0,
                      NULL,
                      &MaxComponent,
                      &Flags,
                      NULL, 0 );
   return Flags;
};

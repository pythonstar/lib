// Drive.h: interface for the CDrive class.
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

#if !defined(AFX_DRIVE_H__6DB881E1_E893_11D1_93C1_241C08C10000__INCLUDED_)
#define AFX_DRIVE_H__6DB881E1_E893_11D1_93C1_241C08C10000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CDrive : public CObject  
{
public:
   CDrive();
   CDrive( TCHAR Drive );
   CDrive( LPCTSTR Drive );
   CDrive( int Drive );
   virtual ~CDrive();

// Methods
public:
   BOOL SetCurrentDrive(void);
   BOOL SetDrive( TCHAR Drive );
   BOOL SetDrive( LPCTSTR Drive );
   BOOL SetDrive( int Drive );
   BOOL SetNextFreeDrive(void);
   BOOL SetFirstNetworkDrive(void);
   //
   TCHAR   GetDrive(void) const { return m_cDrive; };
   UINT    GetType(void) const;
   CString GetName(void) const;
   CString GetFileSystemName(void) const;
   DWORD   GetSerialNumber(void) const;
   DWORD   GetComponentLength(void) const;
   DWORD   GetAvailableSpace( DWORD *HighResult=NULL ) const;
   DWORD   GetSectorsPerCluster(void) const;
   DWORD   GetBytesPerSector(void) const;
   DWORD   GetNumberOfFreeClusters(void) const;
   DWORD   GetTotalNumberOfClusters(void) const;
   //
   CString GetConnection(void) const;
   long    ConnectTo( LPCTSTR Unc, LPCSTR Username=NULL, LPCTSTR Password=NULL );
   long    RemoveConnection(void);
   BOOL    IsConnected(void) const;
   //
   BOOL HasLongFilenameSupport(void) const;
   BOOL HasCompressionSupport(void) const;
   BOOL IsCompressed(void) const;
   BOOL IsCaseSensitive(void) const;

// Implementation
private:
   DWORD GetFlags(void) const;

// Variables
private:
   TCHAR   m_cDrive;
   CString m_strRoot;
};

#endif // !defined(AFX_DRIVE_H__6DB881E1_E893_11D1_93C1_241C08C10000__INCLUDED_)

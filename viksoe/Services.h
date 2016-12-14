// Services.h: interface for the CServices class.
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

#if !defined(AFX_SERVICES_H__ED3DA202_E759_11D1_93C1_241C08C10000__INCLUDED_)
#define AFX_SERVICES_H__ED3DA202_E759_11D1_93C1_241C08C10000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CServices : public CObject  
{
public:
	CServices();
	virtual ~CServices();

// Types
public:
	// Service states (additions to WINSVC.H)
	#define SERVICE_UNKNOWN           0x0000  // The service state is not known
	#define SERVICE_NOTINSTALLED      0xFFFF  // The service is not installed
	// Service create structure
	struct TServiceInfo {
       CString Filename;
       CString ServiceName;
       CString DisplayName;
       BOOL    bInteractWithDesktop;
       CString Login;
       CString Password;
       CString Group;
       CString Dependency;
	};

// Implementation
public:
	RETCODE Create( TServiceInfo &Info );
	RETCODE Delete( LPCTSTR ServiceName );
	RETCODE Start( LPCTSTR ServiceName );
	RETCODE Stop( LPCTSTR ServiceName );
	//
	BOOL IsRunning( LPCTSTR ServiceName );
	BOOL IsInstalled( LPCTSTR ServiceName );
	//
	WORD    GetState( LPCTSTR ServiceName );
	CString GetStateString( WORD State );
	CString GetDisplayName( LPCTSTR ServiceName );
	CString GetServiceName( LPCTSTR DisplayName );
	//
	WORD SendMessage( LPCTSTR ServiceName, WORD Message, WORD &ReturnCode );
	RETCODE GetLastError( long &ErrorCode, CString &ErrorString ) { ErrorCode = m_LastError; ErrorString = m_LastErrorStr; return RET_OK; };
private:
	RETCODE GetServiceManagerHandle( SC_HANDLE &Manager, long Access );
	RETCODE CloseServiceManagerHandle( SC_HANDLE Manager );
	RETCODE GetServiceHandles( LPCTSTR ServiceName, SC_HANDLE &Manager, SC_HANDLE &Service, long Access );
	RETCODE CloseServiceHandles( SC_HANDLE Manager, SC_HANDLE Service );
	RETCODE QueryStatus( SC_HANDLE Service, SERVICE_STATUS &Status );
	RETCODE WaitForState_Loop( SC_HANDLE Service, 
							   WORD Action, 
							   long MaxLoops, long MaxWait );
	void    SetLastError(void);

// Variables
public:
	long    m_LastError;
	CString m_LastErrorStr;
};

#endif // !defined(AFX_SERVICES_H__ED3DA202_E759_11D1_93C1_241C08C10000__INCLUDED_)

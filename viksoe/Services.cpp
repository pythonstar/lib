// Services.cpp: implementation of the CServices class.
// By Bjarke Viksøe
// Description:
//   Create and manipulate NT Services
// Ver:
//   1.0   10 May.1998 Started
//
// Requires: 
//    GLOBAL.CPP/H
//    MPR.LIB
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Services.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CServices::CServices()
{
	m_LastError = 0;
	m_LastErrorStr.Empty();
}

CServices::~CServices()
{

};

RETCODE CServices::Create( TServiceInfo &Info )
{
	if( !IsInstalled( Info.ServiceName ) ) return RET_ALREADYDEFINED;


	// Open a handle to the SC Manager database.
	SC_HANDLE Manager;
	RETCODE ret;
	ret = GetServiceManagerHandle( Manager, SERVICE_INTERROGATE );
	if (ret!=RET_OK) return ret;

	//
	// Init variables
	//
	if( !Info.Login.IsEmpty() ) {
		// If the account name has a backslash, it means that the domain has been
		// specified.  Otherwise, assume the current built-in domain, and pre-pend
		// the domain to the account name.
		long pos = Info.Login.Find( _T("\\") );
		if( pos>=0 ) {
			CString tmp( _T(".\\") + Info.Login );
			Info.Login = tmp;
		};
	};
	// Make sure pointer are NULL if they are empty
	// (only a selected set of the arguments are checked)
	LPCTSTR lpszLogin;
	Info.Login.IsEmpty() ? lpszLogin=NULL : lpszLogin = Info.Login;
	LPCTSTR lpszGroup;
	Info.Group.IsEmpty() ? lpszGroup=NULL : lpszGroup = Info.Group;
	LPCTSTR lpszDependency;
	Info.Dependency.IsEmpty() ? lpszDependency=NULL : lpszDependency = Info.Dependency;
	// Make the Service Type
	DWORD dwServiceType;
	dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	if (Info.bInteractWithDesktop) dwServiceType |= SERVICE_INTERACTIVE_PROCESS;

    //
	// Create Service
	//
	SC_HANDLE Service;
	Service = ::CreateService( Manager,                   // SCManager database
							   Info.ServiceName,          // Name of service
							   Info.DisplayName,	      // Display name
							   SERVICE_ALL_ACCESS,        // Desired access
							   dwServiceType,             // Service type
							   SERVICE_AUTO_START,        // Start type
							   SERVICE_ERROR_NORMAL,      // Error control type
							   Info.Filename,             // Service's binary
							   lpszGroup,                 // Load ordering group
							   NULL,                      // No tag identifier
							   lpszDependency,            // Dependencies
							   lpszLogin,                 // LocalSystem account
							   Info.Password);            // Password

	if (Service == NULL) {
		SetLastError();
		CloseServiceManagerHandle(Manager);
		return RET_ERROR;
	};
	::CloseServiceHandle(Service);
	
	CloseServiceManagerHandle(Manager);
	return RET_OK;
};

RETCODE CServices::Delete( LPCTSTR ServiceName )
{
	if( !IsInstalled(ServiceName) ) return RET_NOTFOUND;
	if( Stop( ServiceName )!=RET_OK ) return RET_BUSY;
	SC_HANDLE Manager;
	SC_HANDLE Service;
	RETCODE ret;
	ret = GetServiceHandles( ServiceName, Manager, Service, SERVICE_INTERROGATE );
	if( ret!=RET_OK ) return ret;
	if ( !DeleteService(Service) ) {
		SetLastError();
		ret = RET_ERROR;
	}
	else
		ret = RET_OK;
	CloseServiceHandles(Manager,Service);
	return ret;
};

RETCODE CServices::Start( LPCTSTR ServiceName )
{
#define MAX_START_LOOPS 6L
#define MAX_START_WAIT 5L
	if( !IsInstalled(ServiceName) ) return RET_NOTFOUND;
	if( IsRunning(ServiceName) ) return RET_OK;    
	SC_HANDLE Manager;
	SC_HANDLE Service;
	RETCODE ret;
	ret = GetServiceHandles( ServiceName, Manager, Service, SERVICE_INTERROGATE );
	if( ret!=RET_OK ) return ret;
	if ( !StartService( Service,           // Handle of service
                        0,                 // Number of arguments
                        NULL) ) {          // No arguments
		SetLastError();
		CloseServiceHandles(Manager,Service);
		return RET_ERROR;
	};
	ret = WaitForState_Loop( Service, SERVICE_RUNNING, MAX_START_LOOPS, MAX_START_WAIT );
	CloseServiceHandles(Manager,Service);
	return ret;
};

RETCODE CServices::Stop( LPCTSTR ServiceName )
{
#define MAX_STOP_LOOPS 6L
#define MAX_STOP_WAIT 5L
	if( !IsInstalled(ServiceName) ) return RET_NOTFOUND;
	SC_HANDLE Manager;
	SC_HANDLE Service;
	RETCODE ret;
	ret = GetServiceHandles( ServiceName, Manager, Service, SERVICE_INTERROGATE );
	if( ret!=RET_OK ) return ret;
	WORD ReturnCode;
	SendMessage( ServiceName, SERVICE_CONTROL_STOP, ReturnCode );
	ret = WaitForState_Loop( Service, SERVICE_STOPPED, MAX_STOP_LOOPS, MAX_STOP_WAIT );
	CloseServiceHandles(Manager,Service);
	return ret;
};

BOOL CServices::IsRunning( LPCTSTR ServiceName )
{
	return GetState( ServiceName )==SERVICE_RUNNING;
};

BOOL CServices::IsInstalled( LPCTSTR ServiceName )
{
	switch( GetState( ServiceName ) ) {
	case SERVICE_UNKNOWN:
	case SERVICE_NOTINSTALLED:
		return FALSE;
	default:
		return TRUE;
	};
};

WORD CServices::GetState( LPCTSTR ServiceName )
{
	ASSERT( ServiceName );
	if( ServiceName==NULL ) return SERVICE_UNKNOWN; // error in argument
	//
	SC_HANDLE Manager;
	SC_HANDLE Service;
	RETCODE ret;
	WORD code;
	ret = GetServiceHandles( ServiceName, Manager, Service, SERVICE_INTERROGATE );
	switch( ret ) {
	case RET_ERROR:
		code =  SERVICE_UNKNOWN; // sanity check
		break;
	case RET_NOTFOUND:
		code = SERVICE_NOTINSTALLED; // not found
		break;
	default:
		SERVICE_STATUS Status;
		if (ControlService( Service, SERVICE_CONTROL_INTERROGATE, &Status )) {
			code = (WORD)Status.dwCurrentState;
		} 
		else {
			if (::GetLastError() == ERROR_SERVICE_NOT_ACTIVE) {
				code = SERVICE_STOPPED; // stopped
			}
			else {
				SetLastError();
				code = SERVICE_UNKNOWN; // don't really know the error
			}
		};
	};
	CloseServiceHandles(Manager,Service);
	return code;
};

CString CServices::GetStateString( WORD State )
{
	switch ( State ) {
	 case SERVICE_UNKNOWN:			return CString("Unknown"); break;
	 case SERVICE_STOPPED:			return CString("Stopped"); break;
	 case SERVICE_START_PENDING:	return CString("Starting"); break;
	 case SERVICE_STOP_PENDING:	    return CString("Stopping"); break;
	 case SERVICE_RUNNING:			return CString("Running"); break;
	 case SERVICE_CONTINUE_PENDING: return CString("Continue pending"); break;
	 case SERVICE_PAUSE_PENDING:	return CString("Pause pending"); break;
	 case SERVICE_PAUSED:			return CString("Paused"); break;
	 case SERVICE_NOTINSTALLED:     return CString("Not installed"); break;
	 default:						return CString("State code error"); break;
	}
};

CString CServices::GetDisplayName( LPCTSTR ServiceName )
{
	SC_HANDLE Manager;
	RETCODE ret;
	ret = GetServiceManagerHandle( Manager, SERVICE_INTERROGATE );
	if( ret!=RET_OK ) return ret;
	CString DisplayName;
	DWORD BufferLen = 256;
	LPTSTR Buffer = DisplayName.GetBuffer(BufferLen);
	if ( !GetServiceDisplayName( Manager, ServiceName, Buffer, &BufferLen ) ) {
		SetLastError();
		DisplayName = CString("");
	}
	else
		DisplayName.ReleaseBuffer(BufferLen);
	CloseServiceManagerHandle(Manager);
	return DisplayName;
};

CString CServices::GetServiceName( LPCTSTR DisplayName )
{
	SC_HANDLE Manager;
	RETCODE ret;
	ret = GetServiceManagerHandle( Manager, SERVICE_INTERROGATE );
	if( ret!=RET_OK ) return ret;
	CString ServiceName;
	DWORD BufferLen = 256;
	LPTSTR Buffer = ServiceName.GetBuffer(BufferLen);
	if ( !GetServiceKeyName( Manager, DisplayName, Buffer, &BufferLen ) ) {
		SetLastError();
		ServiceName = CString("");
	}
	else
		ServiceName.ReleaseBuffer(BufferLen);
	CloseServiceManagerHandle(Manager);
	return ServiceName;
};


WORD CServices::SendMessage( LPCTSTR ServiceName, WORD Message, WORD &ReturnCode )
{
	ASSERT( ServiceName );
	if( ServiceName==NULL ) return SERVICE_UNKNOWN;
	//
	SC_HANDLE Manager;
	SC_HANDLE Service;
	RETCODE ret;
	WORD Access;
	switch( Message ) {
	case SERVICE_CONTROL_CONTINUE:
		Access = SERVICE_START;
		break;
	case SERVICE_CONTROL_INTERROGATE:
		Access = SERVICE_INTERROGATE;
		break;
	case SERVICE_CONTROL_STOP:
		Access = SERVICE_STOP;
		break;
	default:
		if( Access<SERVICE_USER_DEFINED_CONTROL ) 
			Access = SERVICE_INTERROGATE; 
		else 
			Access = SERVICE_USER_DEFINED_CONTROL;
	};
	ret = GetServiceHandles( ServiceName, Manager, Service, Access );
	if( ret!=RET_OK ) return ret;
	//
	SERVICE_STATUS Status;
	ReturnCode = 0;
	if (ControlService( Service, Message, &Status )) {
		ReturnCode = (WORD)Status.dwCurrentState;
		ret = RET_OK;
	} 
	else {
		if (::GetLastError() == ERROR_SERVICE_NOT_ACTIVE) {
			ret = RET_CONNECTERROR;
		}
		else {
			SetLastError();
			ret = RET_ERROR;
		}
	};
	CloseServiceHandles(Manager,Service);
	return ret;
};


//////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTION
//

RETCODE CServices::GetServiceHandles( LPCTSTR ServiceName, 
									  SC_HANDLE &Manager, 
									  SC_HANDLE &Service, long Access )
{
	Manager = NULL;
	Service = NULL;

	// Open the Service Manager database
	RETCODE ret;
	ret = GetServiceManagerHandle( Manager, Access );
	if( ret != RET_OK ) return ret;

	// Get the handle to the Service itself
	Service = ::OpenService(Manager,       // SCManager database
                            ServiceName,   // Name of service
                            Access);       // Service Access needed
	if( Service==NULL ) {
		SetLastError();
		::CloseServiceHandle(Manager);
		return RET_NOTFOUND;
	};
	return RET_OK;
};

RETCODE CServices::GetServiceManagerHandle( SC_HANDLE &Manager, long Access )
{
	Manager = NULL;

	long ManagerAccess;
	switch( Access ) {
	case SERVICE_INTERROGATE:
		ManagerAccess = SC_MANAGER_CREATE_SERVICE;
		break;
	default:
		ManagerAccess = SC_MANAGER_CREATE_SERVICE;
	};
	// Open a handle to the SC Manager database.
	Manager = ::OpenSCManager(NULL,                        // local machine
							  NULL,                        // ServicesActive database
							  ManagerAccess);              // Enough access to create,
														   // delete, or start a service.
	if (Manager==NULL) {
		SetLastError();
		return RET_ERROR;
	}
	return RET_OK;
};

RETCODE CServices::CloseServiceManagerHandle( SC_HANDLE Manager )
{
	::CloseServiceHandle(Manager);
	return RET_OK;
};

RETCODE CServices::CloseServiceHandles( SC_HANDLE Manager, SC_HANDLE Service )
{
	::CloseServiceHandle(Service);
	CloseServiceManagerHandle(Manager);
	return RET_OK;
};

RETCODE CServices::QueryStatus( SC_HANDLE Service, SERVICE_STATUS &Status )
{
	if( Service==NULL ) return RET_ERROR;
	if (!QueryServiceStatus( Service,     // handle of service 
							 &Status) ) {  // address of status info
		SetLastError();
		return RET_ERROR;
	}
	return RET_OK;
};

RETCODE CServices::WaitForState_Loop( SC_HANDLE Service, 
									  WORD Action, 
									  long MaxLoops, long MaxWait )
{
	//
	// Get service status and check that it is running...
	//
	SERVICE_STATUS Status;
	RETCODE ret;
	memset( &Status, 0, sizeof( Status ) );
	ret = QueryStatus( Service, Status );
	if( ret!=RET_OK ) return ret;
	//
	// Service might not start right away...
	// We'll check periodically and give it some time.
	//
	int loops = 0;
	ret = RET_OK;
	while ( Status.dwCurrentState != Action ) {
		DWORD dwOldCheckPoint;
		dwOldCheckPoint = Status.dwCheckPoint;
		// Wait for the specified interval
		// This will give the Service some time to change state...
		Sleep( MaxWait*1000L );
		// Is it there yet?
		ret = QueryStatus( Service, Status );
		if( ret!=RET_OK ) break;
		// Break if the checkpoint has not been incremented
		if( dwOldCheckPoint >= Status.dwCheckPoint ) {
			if ( ++loops >= MaxLoops ) break; // hung!
		}
		else
			loops = 0; // reset progress count, we're still moving
	};
	// Make up return code
	// If QueryStatus() failed, we should be told
	if( ret!=RET_OK ) return ret;
	// If we have exceeded the maximum number of loops
	// we return BUSY
	if ( loops >= MaxLoops ) return RET_BUSY;
	// If the service is still not in correct state we return ERROR
	if ( Status.dwCurrentState != Action ) return RET_ERROR;
	// Everything is fine...
	return RET_OK;
};

void CServices::SetLastError(void)
{
	m_LastError = ::GetLastError();
    DWORD dwLen;
	LPTSTR lpszBuf = m_LastErrorStr.GetBuffer( 256 );
	dwLen = ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
						    NULL,
						    m_LastError,
						    LANG_SYSTEM_DEFAULT,
						    lpszBuf,
						    255,
						    NULL);
	lpszBuf[dwLen-2] = '\0';  // Get rid of CRLF!
	m_LastErrorStr.ReleaseBuffer();
};

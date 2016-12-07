/************************************************************************/
/*
头文件：	TcpSocket.CPP  
创建时间：	2009-03-09
说明：		异步Socket类CTcpAsyncSocket:封装了各类最基本的异步通讯相关的方法 
源码下载自Internet，原作者：lzq（lvzhuqiang@trasin.net）。代码根据
实际需要有改动。
*/
/************************************************************************/  
   
#include "stdafx.h"    
#include "TcpSocket.h"     
   
#ifdef _DEBUG    
#undef THIS_FILE    
static char THIS_FILE[]=__FILE__;   
#define new DEBUG_NEW    
#endif    
   
//默认构造函数，默认为服务器。 
CTcpAsyncSocket::CTcpAsyncSocket()   
{   
    m_hSocket   = INVALID_SOCKET;   
    m_blCanQuit = FALSE;   
	m_bServer=TRUE;
}  

//通过传递参数来设置客户端/服务器标志  
CTcpAsyncSocket::CTcpAsyncSocket(BOOL bServer)
{
	m_bServer=bServer;
}

//析构函数，关闭套接字
CTcpAsyncSocket::~CTcpAsyncSocket()   
{   
    Close();   
}   
   
//关闭套接字
void CTcpAsyncSocket::Close()   
{   
    if (m_hSocket != INVALID_SOCKET)   
    {   
        VERIFY(SOCKET_ERROR != closesocket(m_hSocket));   
        m_hSocket = INVALID_SOCKET;   
    }   
}   

//调用形式如：Create(2009);  
BOOL CTcpAsyncSocket::Create(UINT nSocketPort,int nType,int nIPPROTO,const char* szIPAddress)   
{   
    // Allocate socket handle    
    m_hSocket = socket(AF_INET, nType, 0);   
    if(m_hSocket == INVALID_SOCKET)   
    {   
        int nResult = GetLastError();   
        WSASetLastError(nResult);   
        return FALSE;   
    }  

	//如果是作为客户端，直接返回
	if (!m_bServer)
		return TRUE;

	//如果是作为服务器，则还需要绑定
	BOOL bOptVal = TRUE;   
	/* Set to enable reuse the address,or the bind() will result an error*/   
	int rc = setsockopt(m_hSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&bOptVal, sizeof(int));   
	if (rc == SOCKET_ERROR)   
	{   
		int nResult = GetLastError();   
		Close();   
		WSASetLastError(nResult);   
		return FALSE;   
	}  
	if (Bind(nSocketPort,szIPAddress))   
		return TRUE;   
	else   
	{   
		int nResult = GetLastError();   
		Close();   
		WSASetLastError(nResult);   
		return FALSE;   
	}   
}   
   
BOOL CTcpAsyncSocket::Bind(UINT nSocketPort, LPCTSTR lpszSocketAddress)   
{   
    SOCKADDR_IN sockAddr;   
    memset(&sockAddr,0,sizeof(sockAddr));   
   
    LPSTR lpszAscii = (LPTSTR)lpszSocketAddress;   
    sockAddr.sin_family = AF_INET;   
   
    if (lpszAscii == NULL)   
        sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);   
    else   
    {   
        DWORD lResult = inet_addr(lpszAscii);   
        if (lResult == INADDR_NONE)   
        {   
            WSASetLastError(WSAEINVAL);   
            return FALSE;   
        }   
        sockAddr.sin_addr.s_addr = lResult;   
    }   
   
    sockAddr.sin_port = htons((u_short)nSocketPort);   
    m_iClientPort = nSocketPort;   
    // Bind to socket address    
    if(bind(m_hSocket, (SOCKADDR*)&sockAddr, sizeof(sockAddr)) == SOCKET_ERROR)   
    {   
        int nResult = GetLastError();   
        Close();   
        WSASetLastError(nResult);   
        return FALSE;   
    }   
   
    int addrlen = sizeof(SOCKADDR);   
    // Get full network.number.socket address    
    getsockname(m_hSocket, (PSOCKADDR)&sockAddr, &addrlen) ;   
   
    return TRUE;   
}   
   
int CTcpAsyncSocket::Receive(void* lpBuf, int nBufLen, int nFlags)   
{   
#ifdef MULT_THREAD    
    m_objLock.Lock();   
#endif      
       
    unsigned long length;   
    int ret;   
    ret = ioctlsocket(m_hSocket, FIONREAD, &length);   
    if (ret == SOCKET_ERROR || length == 0)   
    {   
#ifdef MULT_THREAD    
        m_objLock.Unlock();   
#endif    
           
        return 0;   
    }   
    int iRecv = recv(m_hSocket, (LPSTR)lpBuf, nBufLen, nFlags);   
#ifdef MULT_THREAD    
    m_objLock.Unlock();   
#endif    
       
    return iRecv;   
}   
   
int CTcpAsyncSocket::Send(const void* lpBuf, int nBufLen, int nFlags)   
{   
#ifdef MULT_THREAD    
    m_objLock.Lock();   
#endif    
    int iSend = send(m_hSocket, (LPSTR)lpBuf, nBufLen, nFlags);   
   
    char *szBuffer = (char*)calloc(nBufLen+1,sizeof(char));   
    memcpy(szBuffer,lpBuf,nBufLen);   
    CString strMsg;   
    if(iSend > 5)   
        strMsg.Format("Send Succ:%s",szBuffer);   
    else   
        strMsg.Format("Send Faild:%s",szBuffer);   
   // LogMsg(strMsg,DEBUG_CONSOLE_NOMAL,DEBUG_FILE_NOMAL);   
    free(szBuffer);   
#ifdef MULT_THREAD    
    m_objLock.Unlock();   
#endif    
   
    return iSend;   
}   
   
int CTcpAsyncSocket::SendTo( const void* lpBuf, int nBufLen, UINT nHostPort, LPCTSTR lpszHostAddress, int nFlags)   
{   
#ifdef MULT_THREAD    
    m_objLock.Lock();   
#endif    
       
    SOCKADDR_IN sockAddr;   
    memset(&sockAddr,0,sizeof(sockAddr));   
   
    LPSTR lpszAscii = (LPTSTR)lpszHostAddress;   
    sockAddr.sin_family = AF_INET;   
   
    if (lpszAscii == NULL)   
        sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);   
    else   
    {   
        DWORD lResult = inet_addr(lpszAscii);   
        if (lResult == INADDR_NONE)   
        {   
            WSASetLastError(WSAEINVAL);   
#ifdef MULT_THREAD    
            m_objLock.Unlock();   
#endif    
               
            return FALSE;   
        }   
        sockAddr.sin_addr.s_addr = lResult;   
    }   
   
    sockAddr.sin_port = htons((u_short)nHostPort);   
   
    int nSendLen=sendto(m_hSocket, (LPSTR)lpBuf, nBufLen, nFlags, (const struct sockaddr *)&sockAddr, sizeof(sockAddr));   
    if(nSendLen ==-1)   
    {   
        int nResult = GetLastError();   
        WSASetLastError(nResult);   
    }   
#ifdef MULT_THREAD    
    m_objLock.Unlock();   
#endif    
       
    return nSendLen;   
}   
   
int CTcpAsyncSocket::ReceiveFrom(void* lpBuf, int nBufLen, CString& rSocketAddress, UINT& rSocketPort, int nFlags,int nTimeOut)   
{   
#ifdef MULT_THREAD    
    m_objLock.Lock();   
#endif    
       
    int nfds =1;   
    int ret;   
    fd_set readfds;   
    timeval tTimeVal;   
    FD_ZERO(&readfds);   
    FD_SET(m_hSocket,&readfds);   
    tTimeVal.tv_usec = nTimeOut;   
    tTimeVal.tv_sec = nTimeOut;   
   
    ret = select(nfds,&readfds,NULL,NULL,&tTimeVal);   
    if(ret <= 0)   
    {   
#ifdef MULT_THREAD    
        m_objLock.Unlock();   
#endif    
           
        return ret; //time out or socket error    
    }   
    ret = ReceiveFrom(lpBuf,nBufLen,(LPCTSTR)rSocketAddress,&rSocketPort,nFlags);   
#ifdef MULT_THREAD    
    m_objLock.Unlock();   
#endif    
       
    return ret;   
}   
   
int CTcpAsyncSocket::ReceiveFrom(void* lpBuf, int nBufLen, LPCTSTR pSocketAddress, UINT* pSocketPort,int nFlags) 
{   
#ifdef MULT_THREAD    
    m_objLock.Lock();   
#endif    
       
    SOCKADDR_IN sockAddr;   
   
    memset(&sockAddr, 0, sizeof(sockAddr));   
   
    int nSockAddrLen = sizeof(sockAddr);   
    BOOL bResult =SOCKET_ERROR!=recvfrom(m_hSocket, (LPSTR)lpBuf, nBufLen, nFlags, (SOCKADDR*)&sockAddr, &nSockAddrLen);   
    if(bResult)   
    {   
		if(pSocketAddress)
			pSocketAddress = inet_ntoa(sockAddr.sin_addr); 
		if(pSocketPort)
			*pSocketPort = ntohs(sockAddr.sin_port);   
    }   
    else   
    {   
        int nError = GetLastError();   
    }   
#ifdef MULT_THREAD    
    m_objLock.Unlock();   
#endif    
       
    return (int)bResult;   
}   
   
//Connect("192.068.84.209",2009);
BOOL CTcpAsyncSocket::Connect(LPCTSTR lpszHostAddress, UINT nHostPort)   
{   
    ASSERT(lpszHostAddress != NULL);   
#ifdef MULT_THREAD    
    m_objLock.Lock();   
#endif    
       
    SOCKADDR_IN sockAddr;   
    memset(&sockAddr,0,sizeof(sockAddr));   
       
    LPSTR lpszAscii = (LPTSTR)lpszHostAddress;   
    sockAddr.sin_family = AF_INET;   
    sockAddr.sin_addr.s_addr = inet_addr(lpszAscii);   
   
    if (sockAddr.sin_addr.s_addr == INADDR_NONE)   
    {   
        LPHOSTENT lphost;   
        lphost = gethostbyname(lpszAscii);   
        if (lphost != NULL)   
            sockAddr.sin_addr.s_addr = ((LPIN_ADDR)lphost->h_addr)->s_addr;   
        else   
        {   
            WSASetLastError(WSAEINVAL);   
#ifdef MULT_THREAD    
            m_objLock.Unlock();   
#endif    
               
            return FALSE;   
        }   
    }   
    m_iClientPort = nHostPort;   
    sockAddr.sin_port = htons((u_short)nHostPort);   
       
    if(connect(this->m_hSocket,(SOCKADDR*)&sockAddr, sizeof(sockAddr)) == SOCKET_ERROR )   
    {   
        int nResult = GetLastError();   
        Close();   
        WSASetLastError(nResult);   
#ifdef MULT_THREAD    
        m_objLock.Unlock();   
#endif    
           
        return FALSE;   
    }   
#ifdef MULT_THREAD    
    m_objLock.Unlock();   
#endif    
       
    return TRUE;   
}   

   
BOOL CTcpAsyncSocket::IOCtl(long lCommand, DWORD* lpArgument)   
{    
    return (SOCKET_ERROR != ioctlsocket(m_hSocket, lCommand, lpArgument));    
}   
   
BOOL CTcpAsyncSocket::SetNonBlockingMode(BOOL flag)   
{   
    DWORD arg = flag;   
    return IOCtl(FIONBIO, &arg);   
}   
   
BOOL CTcpAsyncSocket::Listen(int nConnectionBacklog)   
{   
    return (SOCKET_ERROR != listen(m_hSocket, nConnectionBacklog));    
}   
   
BOOL CTcpAsyncSocket::Accept(CTcpAsyncSocket& rConnectedSocket)   
{   
#ifdef MULT_THREAD    
    m_objLock.Lock();   
#endif 
	SOCKADDR_IN sockAddr; 
	memset(&sockAddr,0,sizeof(sockAddr));
    int nSockAddrLen = sizeof(SOCKADDR_IN);
	//nSockAddrLen Must Be the size of SOCKADDR_IN
    SOCKET hTemp = accept(m_hSocket,(SOCKADDR*)&sockAddr, &nSockAddrLen);   
    if (hTemp == INVALID_SOCKET)   
    {   
        DWORD dwProblem = GetLastError();   
        rConnectedSocket.m_hSocket = INVALID_SOCKET;   
        SetLastError(dwProblem);   
    }   
    else   
    {   
		rConnectedSocket.m_hSocket = hTemp;   
		rConnectedSocket.m_iClientPort = ntohs(sockAddr.sin_port);   
		rConnectedSocket.m_strClientIp = inet_ntoa(sockAddr.sin_addr);   
    }   
#ifdef MULT_THREAD    
    m_objLock.Unlock();   
#endif    
    return (hTemp != INVALID_SOCKET);   
}   
   
   
BOOL CTcpAsyncSocket::Accept(CTcpAsyncSocket& rConnectedSocket,CString &strHostIp,UINT &iHostPort)   
{   
#ifdef MULT_THREAD    
    m_objLock.Lock();   
#endif    
       
    SOCKADDR_IN sockAddr;   
    memset(&sockAddr, 0, sizeof(sockAddr));   
    int lpSockAddrLen = sizeof(sockAddr);   
    
   
    SOCKET hTemp = accept(m_hSocket, (SOCKADDR*)&sockAddr, &lpSockAddrLen);   
    if (hTemp == INVALID_SOCKET)   
    {   
        DWORD dwProblem = GetLastError();   
        rConnectedSocket.m_hSocket = INVALID_SOCKET;   
        SetLastError(dwProblem);   
    }   
    else   
    {   
        rConnectedSocket.m_hSocket = hTemp;   
        iHostPort = ntohs(sockAddr.sin_port);   
        strHostIp = inet_ntoa(sockAddr.sin_addr);   
        rConnectedSocket.m_iClientPort = iHostPort;   
        rConnectedSocket.m_strClientIp = strHostIp;   
    }   
#ifdef MULT_THREAD    
    m_objLock.Unlock();   
#endif    
       
    return (hTemp != INVALID_SOCKET);   
   
}   

/************************************************************************
模快名:		moxu 公共类库
功能:		对socket的简单封装
完成日期:	2007-10-20
作者:		许 培 Xu Pei(Email/MSN: peimoxu@163.com)

本代码可以自由使用，但因使用本代码造成的后果，本人不承担任何责任
************************************************************************/

#include "stdafx.h"
#include "Socket.h"

namespace  Star
{

bool Socket::Create(unsigned short nPort, int nType, LPCTSTR lpszAddress)
{
	m_hSocket = socket(AF_INET, nType, 0);
	if (INVALID_SOCKET == m_hSocket)
	{
		return false;
	}

	return Bind(nPort, lpszAddress);
}

bool Socket::Bind(unsigned short nPort, LPCTSTR lpszAddress)
{
	sockaddr_in SockAddr;

	memset(&SockAddr, 0, sizeof(SockAddr));
	SockAddr.sin_family	= AF_INET;
	SockAddr.sin_port	= htons(nPort);
	if(NULL == lpszAddress)
		SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	else
	{
		SockAddr.sin_addr.s_addr = inet_addr(lpszAddress);
		//若不是IP地址则通过主机名得到s_addr
		if(INADDR_NONE == SockAddr.sin_addr.s_addr)
		{
			LPHOSTENT lpHost;
			lpHost = gethostbyname(lpszAddress);
			if(NULL == lpHost)
			{
				return false;
			}
			SockAddr.sin_addr.s_addr = ((LPIN_ADDR)lpHost->h_addr)->s_addr;		
		}
	}	

	return Bind((SOCKADDR *)&SockAddr);
}

bool Socket::Bind(const SOCKADDR* lpSockAddr)
{
	int rs;

	rs= bind(m_hSocket, lpSockAddr, sizeof(*lpSockAddr));
	if(SOCKET_ERROR == rs)
	{
		return false;
	}

	return true;
}

bool Socket::Listen(int nConnectionBacklog/* = SOMAXCONN*/)
{
	int rs;

	rs = listen(m_hSocket, nConnectionBacklog);
	if(SOCKET_ERROR == rs)
	{
		return false;
	}

	return true;
}

bool Socket::Connect(LPCTSTR lpszHostAddress, unsigned short nHostPort)
{
	sockaddr_in SockAddr;

	memset(&SockAddr, 0, sizeof(SockAddr));
	SockAddr.sin_family	= AF_INET;
	SockAddr.sin_port	= htons(nHostPort);

	SockAddr.sin_addr.s_addr = inet_addr(lpszHostAddress);

	if(INADDR_NONE == SockAddr.sin_addr.s_addr)
	{
		LPHOSTENT lpHost;
		lpHost = gethostbyname(lpszHostAddress);
		if(NULL == lpHost)
		{
			return false;
		}
		SockAddr.sin_addr.s_addr = ((LPIN_ADDR)lpHost->h_addr)->s_addr;		
	}

	return Connect((SOCKADDR *)&SockAddr);
}

bool Socket::Connect(const SOCKADDR* lpSockAddr)
{
	int rs;

	rs = connect(m_hSocket, lpSockAddr, sizeof(*lpSockAddr));
	if(SOCKET_ERROR == rs)
	{
		return false;
	}

	return true;
}

bool Socket::Accept(Socket& ConnectedSocket, SOCKADDR* lpSockAddr, int* lpSockAddrLen)
{
	ConnectedSocket.m_hSocket = accept(m_hSocket, lpSockAddr, lpSockAddrLen);
	if (INVALID_SOCKET == ConnectedSocket.m_hSocket)
	{
		return false;
	}

	return true;
}

bool Socket::SetOption(int nOptionName, const void* lpOptionValue, int nOptionLen, int nLevel)
{
	int rs;

	rs = setsockopt(m_hSocket, nLevel, nOptionName, (char *)lpOptionValue, nOptionLen);
	if (NO_ERROR != rs) 
	{
		return false;
	}

	return true;
}

int Socket::Send(const void* lpBuf, int nBufLen, int nFlags)
{
	return send(m_hSocket, (char *)lpBuf, nBufLen, nFlags);
}

int Socket::Receive(void* lpBuf, int nBufLen, int nFlags)
{
	return recv(m_hSocket, (char *)lpBuf, nBufLen, nFlags);
}

int Socket::Close()
{
	return closesocket(m_hSocket);
}


}//namespace Star

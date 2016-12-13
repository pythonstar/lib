/************************************************************************
模快名:		moxu 公共类库
功能:		对socket的简单封装
完成日期:	2007-10-20
作者:		许 培 Xu Pei(Email/MSN: peimoxu@163.com)

本代码可以自由使用，但因使用本代码造成的后果，本人不承担任何责任
************************************************************************/

#pragma once

#include <WinSock2.h>

namespace Star
{

	class Socket
	{
	private:
		SOCKET	m_hSocket;

	public:
		Socket(void){}
		~Socket(void){}

		bool Create(unsigned short nPort = 0, int nType = SOCK_STREAM, LPCTSTR lpszAddress = NULL);

		bool Bind(unsigned short nPort, LPCTSTR lpszAddress = NULL);
		bool Bind(const SOCKADDR* lpSockAddr);

		bool Listen(int nConnectionBacklog = SOMAXCONN);

		bool Connect(LPCTSTR lpszHostAddress, unsigned short nHostPort);
		bool Connect(const SOCKADDR* lpSockAddr);

		bool Accept(Socket& ConnectedSocket, SOCKADDR* lpSockAddr = NULL, int* lpSockAddrLen = NULL);

		bool SetOption(int nOptionName, const void* lpOptionValue, int nOptionLen, int nLevel = SOL_SOCKET);

		int Send(const void* lpBuf, int nBufLen, int nFlags = 0);
		int Receive(void* lpBuf, int nBufLen, int nFlags = 0);

		int Close();
	};


}

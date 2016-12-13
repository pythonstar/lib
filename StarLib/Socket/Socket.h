/************************************************************************
ģ����:		moxu �������
����:		��socket�ļ򵥷�װ
�������:	2007-10-20
����:		�� �� Xu Pei(Email/MSN: peimoxu@163.com)

�������������ʹ�ã�����ʹ�ñ�������ɵĺ�������˲��е��κ�����
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

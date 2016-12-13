/************************************************************************/
/*
ͷ�ļ���	TcpSocket.h 
����ʱ�䣺	2009-03-09
˵����		�첽Socket��CTcpAsyncSocket:��װ�˸�����������첽ͨѶ��صķ��� 
			Դ��������Internet��ԭ���ߣ�lzq��lvzhuqiang@trasin.net�����������
			ʵ����Ҫ�иĶ���
*/
/************************************************************************/
 
#ifndef _TCPSOCKET_H 
#define _TCPSOCKET_H 
 
#include  "afxsock.h"
//#define MULT_THREAD 10 
#ifdef MULT_THREAD 
#include "afxmt.h"
#endif 
 
class CTcpAsyncSocket   
{ 
public: 
#ifdef MULT_THREAD 
	CCriticalSection m_objLock; 
#endif 
	BOOL     m_blCanQuit; 
	SOCKET	 m_hSocket; 
	CString  m_strClientIp; 
	UINT     m_iClientPort;

	//���ǵ�server��clientʹ��ͬһ����ʵ�֣�
	//ʹ�ô˱��ֵ�������ǿͻ��˻��Ƿ�����
	BOOL	 m_bServer;
public: 
	CTcpAsyncSocket(); 
	CTcpAsyncSocket(BOOL bServer);
	virtual ~CTcpAsyncSocket(); 
	void	Close(); 
	BOOL	Bind(UINT nSocketPort, LPCTSTR lpszSocketAddress); 
	BOOL	Create(UINT nSocketPort = 0, int nType=SOCK_STREAM,int nIPPROTO=IPPROTO_UDP,const char* szIPAddress=NULL); 
	int		Receive(void* lpBuf, int nBufLen, int nFlags = 0); 
	int		ReceiveFrom(void* lpBuf, int nBufLen, LPCTSTR pSocketAddress=NULL, UINT* pSocketPort=0, int nFlags =0); 
	int		ReceiveFrom(void* lpBuf, int nBufLen, CString& rSocketAddress, UINT& rSocketPort, int nFlags ,int nTimeOut); 
	int		Send(const void* lpBuf, int nBufLen, int nFlags = 0); 
	int		SendTo( const void* lpBuf, int nBufLen, UINT nHostPort, LPCTSTR lpszHostAddress = NULL, int nFlags = 0 ); 
	BOOL	Connect(LPCTSTR lpszHostAddress, UINT nHostPort); 
	BOOL	IOCtl(long lCommand, DWORD* lpArgument); 
	BOOL	SetNonBlockingMode(BOOL flag); 
	BOOL	Listen(int nConnectionBacklog=5); 
	BOOL	Accept(CTcpAsyncSocket& rConnectedSocket); 
	BOOL    Accept(CTcpAsyncSocket& rConnectedSocket,CString &strHostIp,UINT &iHostPort); 
 
}; 

typedef CTcpAsyncSocket *PCTcpAsyncSocket;
 
#endif

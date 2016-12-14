// test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <winsock2.h>
#include "TcpSocket.h"
#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "Ws2_32.lib")

char   m_szIPAddr[32]={0};
SOCKET m_sock;
ULONG  m_ulTime ;

void FormatUpdatedTime(SYSTEMTIME *pstOld, SYSTEMTIME *pstNew)
{
	TCHAR szDateOld [64], szTimeOld [64], szDateNew [64], szTimeNew [64] ;

	::GetDateFormat (LOCALE_USER_DEFAULT, LOCALE_NOUSEROVERRIDE | DATE_SHORTDATE,
		pstOld, NULL, szDateOld, sizeof (szDateOld)) ;

	::GetTimeFormat (LOCALE_USER_DEFAULT, LOCALE_NOUSEROVERRIDE |TIME_NOTIMEMARKER | TIME_FORCE24HOURFORMAT,
		pstOld, NULL, szTimeOld, sizeof (szTimeOld)) ;

	::GetDateFormat (LOCALE_USER_DEFAULT, LOCALE_NOUSEROVERRIDE | DATE_SHORTDATE,
		pstNew, NULL, szDateNew, sizeof (szDateNew)) ;

	::GetTimeFormat (LOCALE_USER_DEFAULT, LOCALE_NOUSEROVERRIDE |TIME_NOTIMEMARKER | TIME_FORCE24HOURFORMAT,
		pstNew, NULL, szTimeNew, sizeof (szTimeNew)) ;

	printf ( "\r\n系统的日期和时间已成功更改："
		"\r\n以前\t%s, %s.%03i \r\n现在\t%s, %s.%03i.", 

		szDateOld, szTimeOld, pstOld->wMilliseconds,
		szDateNew, szTimeNew, pstNew->wMilliseconds) ;
} 

void ChangeSystemTime()
{
	FILETIME      ftNew ;     
	SYSTEMTIME    stOld, stNew ;     
	::GetLocalTime (&stOld) ;   //首先取得目前的本地时间

	stNew.wYear         = 1900 ;
	stNew.wMonth        = 1 ;
	stNew.wDay          = 1 ;
	stNew.wHour         = 0 ;
	stNew.wMinute       = 0 ;
	stNew.wSecond       = 0 ;
	stNew.wMilliseconds = 0 ;
	::SystemTimeToFileTime (&stNew, &ftNew);

	/*  将SYSTEMTIME结构设定为1900年1月1日午夜（0时）。
	并将这个SYSTEMTIME结构传递给SystemTimeToFileTime，将此结构转化为FILETIME结构。
	FILETIME实际上只是由两个32位元的DWORD一起组成64位元的整数，
	用来表示从1601年1月1日至今间隔为100奈秒（nanosecond）的间隔数。 */     

	LARGE_INTEGER li ;			//64位大整数
	li = * (LARGE_INTEGER *) &ftNew;
	li.QuadPart += (LONGLONG) 10000000 * m_ulTime; 
	ftNew = * (FILETIME *) &li;
	::FileTimeToSystemTime (&ftNew, &stNew);

	if (::SetSystemTime (&stNew))	//调用SetSystemTime来设定时间
	{
		::GetLocalTime (&stNew);
		FormatUpdatedTime (&stOld, &stNew);   //最初的本地时间和新的本地时间一起传递给FormatUpdatedTime 这个函数用::GetTimeFormat函数和::GetDateFormat函数将时间转化为ASCII字串。
	}
	else
		printf ("不能设置新的日期和时间!");
}

int WaitData(SOCKET hSocket,DWORD dwTime)
{
	fd_set stFdSet;
	timeval stTimeval;

	stFdSet.fd_count=1;
	stFdSet.fd_array[0]=hSocket;
	stTimeval.tv_usec=dwTime;
	stTimeval.tv_sec=0;
	return select(0,&stFdSet,NULL,NULL,&stTimeval);
}

int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA WSAData;
	::WSAStartup (MAKEWORD(2,0), &WSAData);

	CTcpAsyncSocket *pClientSocket=new CTcpAsyncSocket(FALSE);
	pClientSocket->Create();
	if( pClientSocket->Connect("132.163.4.101",IPPORT_TIMESERVER) )
	{//连接成功
		TRACE("连接成功\n");
	}
	else
	{
		TRACE("连接失败\n");
	}


	while (pClientSocket->m_hSocket)
	{
		int ret= WaitData(pClientSocket->m_hSocket,10*1000);
		if (ret==SOCKET_ERROR)
		{

			break;
		}
		if (ret&&!pClientSocket->Receive(&m_ulTime,4))//与服务器断开链接
		{
			TRACE("服务器断开连接\n");
			break;
		}
		if (ret)//没有退出且ret不为零，说明是正常的发送与接收
		{
			//ProcessServerCmd(theApp.pClientSocket,Buffer);
			m_ulTime=::ntohl (m_ulTime) ;
			printf ( "接受的时间为从1900年1月1日起 %u 秒 \r\n", m_ulTime) ;

			ChangeSystemTime();
		}
	}

	if (pClientSocket){
		delete pClientSocket;
		pClientSocket=NULL;
	}

	system("pause");
	return 0;
}


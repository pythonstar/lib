// test.cpp : �������̨Ӧ�ó������ڵ㡣
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

	printf ( "\r\nϵͳ�����ں�ʱ���ѳɹ����ģ�"
		"\r\n��ǰ\t%s, %s.%03i \r\n����\t%s, %s.%03i.", 

		szDateOld, szTimeOld, pstOld->wMilliseconds,
		szDateNew, szTimeNew, pstNew->wMilliseconds) ;
} 

void ChangeSystemTime()
{
	FILETIME      ftNew ;     
	SYSTEMTIME    stOld, stNew ;     
	::GetLocalTime (&stOld) ;   //����ȡ��Ŀǰ�ı���ʱ��

	stNew.wYear         = 1900 ;
	stNew.wMonth        = 1 ;
	stNew.wDay          = 1 ;
	stNew.wHour         = 0 ;
	stNew.wMinute       = 0 ;
	stNew.wSecond       = 0 ;
	stNew.wMilliseconds = 0 ;
	::SystemTimeToFileTime (&stNew, &ftNew);

	/*  ��SYSTEMTIME�ṹ�趨Ϊ1900��1��1����ҹ��0ʱ����
	�������SYSTEMTIME�ṹ���ݸ�SystemTimeToFileTime�����˽ṹת��ΪFILETIME�ṹ��
	FILETIMEʵ����ֻ��������32λԪ��DWORDһ�����64λԪ��������
	������ʾ��1601��1��1��������Ϊ100���루nanosecond���ļ������ */     

	LARGE_INTEGER li ;			//64λ������
	li = * (LARGE_INTEGER *) &ftNew;
	li.QuadPart += (LONGLONG) 10000000 * m_ulTime; 
	ftNew = * (FILETIME *) &li;
	::FileTimeToSystemTime (&ftNew, &stNew);

	if (::SetSystemTime (&stNew))	//����SetSystemTime���趨ʱ��
	{
		::GetLocalTime (&stNew);
		FormatUpdatedTime (&stOld, &stNew);   //����ı���ʱ����µı���ʱ��һ�𴫵ݸ�FormatUpdatedTime ���������::GetTimeFormat������::GetDateFormat������ʱ��ת��ΪASCII�ִ���
	}
	else
		printf ("���������µ����ں�ʱ��!");
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
	{//���ӳɹ�
		TRACE("���ӳɹ�\n");
	}
	else
	{
		TRACE("����ʧ��\n");
	}


	while (pClientSocket->m_hSocket)
	{
		int ret= WaitData(pClientSocket->m_hSocket,10*1000);
		if (ret==SOCKET_ERROR)
		{

			break;
		}
		if (ret&&!pClientSocket->Receive(&m_ulTime,4))//��������Ͽ�����
		{
			TRACE("�������Ͽ�����\n");
			break;
		}
		if (ret)//û���˳���ret��Ϊ�㣬˵���������ķ��������
		{
			//ProcessServerCmd(theApp.pClientSocket,Buffer);
			m_ulTime=::ntohl (m_ulTime) ;
			printf ( "���ܵ�ʱ��Ϊ��1900��1��1���� %u �� \r\n", m_ulTime) ;

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


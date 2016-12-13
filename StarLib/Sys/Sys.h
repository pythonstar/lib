
#pragma once
#include <afx.h>
#include <vector>
using namespace std;


//������Ϣ�ṹ
typedef struct TRAYINFO
{
	CString strDiscription;	//�����ı�
	CString strFilePath;		//�ļ�·��
}*PTRAYINFO;


namespace Star
{
	namespace Sys
	{
		BOOL IsWow64();
		int GetTrayInfo(HWND hWnd,vector<TRAYINFO>&vtTrays);

#if (WINVER >= 0x0600)

		//�жϲ���ϵͳ�ǲ���win7����vista
		BOOL IsVista();

		//��vista���жϵ�ǰ�û��Ƿ��ǹ���Ա
		BOOL IsAdmin();

#endif

		HWND FindTrayWnd();
		HWND FindNotifyIconOverflowWindow();

		//�����Ƿ�����
		BOOL IsScreensaversOn(void);

		//��ȡ��ʾ�����������Ϊ�������
		int  GetMonitorCount(void);

		//��ʾ���Ƿ�ر�
		BOOL IsMonitorOn(void);

		static BOOL CALLBACK EnumMonitorsProc(HMONITOR hMonitor, HDC /*hdcMonitor*/, LPRECT lprcMonitor, LPARAM dwData);
	}
}

#pragma once
#include <afx.h>
#include <vector>
using namespace std;


//托盘信息结构
typedef struct TRAYINFO
{
	CString strDiscription;	//描述文本
	CString strFilePath;		//文件路径
}*PTRAYINFO;


namespace Star
{
	namespace Sys
	{
		BOOL IsWow64();
		int GetTrayInfo(HWND hWnd,vector<TRAYINFO>&vtTrays);

#if (WINVER >= 0x0600)

		//判断操作系统是不是win7或者vista
		BOOL IsVista();

		//在vista下判断当前用户是否是管理员
		BOOL IsAdmin();

#endif

		HWND FindTrayWnd();
		HWND FindNotifyIconOverflowWindow();

		//屏保是否运行
		BOOL IsScreensaversOn(void);

		//获取显示器个数，如果为零则可疑
		int  GetMonitorCount(void);

		//显示器是否关闭
		BOOL IsMonitorOn(void);

		static BOOL CALLBACK EnumMonitorsProc(HMONITOR hMonitor, HDC /*hdcMonitor*/, LPRECT lprcMonitor, LPARAM dwData);
	}
}
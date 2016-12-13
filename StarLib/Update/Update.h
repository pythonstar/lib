
#pragma once
#include <afx.h>
#include <vector>
using namespace std;


//脚本信息
typedef struct SCRIPTINFO {
	CString strScriptName;		//脚本名称：英文
	CString strScriptDesc;		//脚本描述：中文
	CString strScriptVer;		//脚本版本	
	CString strLocalFileName;	//脚本保存到本地的文件名
	CString strScriptUrl;		//脚本下载地址
	CString strScriptUrl2;		//脚本下载地址2
}*PSCRIPTINFO;

typedef struct UPDATEINFO
{
	CString strName;			//该程序的名称
	CString strVersion;			//最新版本信息
	CString strDownUrl;			//试用版下载地址
	CString strDownUrl2;		//注册版下载地址
	CString strRegUrl;			//购买网址
	CString strHelpUrl;			//帮助网页地址
	CString strHome;			//官方网站
	CString strBBS;				//官方论坛
	CString strNews;			//最新版本信息，可以提示修复的错误，广告，新特征等
	CString strAdsUrl;			//广告配置信息的文件地址
	CString strMailMeUrl;		//邮我
	CString strUrgent;			//应急措施
	CString strScriptPath;		//脚本存放相对路径
	CString strScriptRoot;		//服务器脚本存放目录
	CString strScriptRoot2;		//服务器脚本存放目录2
	BOOL	bUsePlanB;			//备用服务器开关，打开时则使用备用服务器更新数据
	vector<SCRIPTINFO>vtScripts;//脚本列表
}*PUPDATEINFO;


//BOOL CheckUpdateInfo(CString strAppName,CString strUrl,UPDATEINFO&stUpdateInfo);
BOOL CheckUpdateInfoIni(const CString&strUrl,UPDATEINFO&stUpdateInfo);
void StartNewVersionApp(CString strNewApp);

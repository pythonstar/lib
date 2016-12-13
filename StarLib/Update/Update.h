
#pragma once
#include <afx.h>
#include <vector>
using namespace std;


//�ű���Ϣ
typedef struct SCRIPTINFO {
	CString strScriptName;		//�ű����ƣ�Ӣ��
	CString strScriptDesc;		//�ű�����������
	CString strScriptVer;		//�ű��汾	
	CString strLocalFileName;	//�ű����浽���ص��ļ���
	CString strScriptUrl;		//�ű����ص�ַ
	CString strScriptUrl2;		//�ű����ص�ַ2
}*PSCRIPTINFO;

typedef struct UPDATEINFO
{
	CString strName;			//�ó��������
	CString strVersion;			//���°汾��Ϣ
	CString strDownUrl;			//���ð����ص�ַ
	CString strDownUrl2;		//ע������ص�ַ
	CString strRegUrl;			//������ַ
	CString strHelpUrl;			//������ҳ��ַ
	CString strHome;			//�ٷ���վ
	CString strBBS;				//�ٷ���̳
	CString strNews;			//���°汾��Ϣ��������ʾ�޸��Ĵ��󣬹�棬��������
	CString strAdsUrl;			//���������Ϣ���ļ���ַ
	CString strMailMeUrl;		//����
	CString strUrgent;			//Ӧ����ʩ
	CString strScriptPath;		//�ű�������·��
	CString strScriptRoot;		//�������ű����Ŀ¼
	CString strScriptRoot2;		//�������ű����Ŀ¼2
	BOOL	bUsePlanB;			//���÷��������أ���ʱ��ʹ�ñ��÷�������������
	vector<SCRIPTINFO>vtScripts;//�ű��б�
}*PUPDATEINFO;


//BOOL CheckUpdateInfo(CString strAppName,CString strUrl,UPDATEINFO&stUpdateInfo);
BOOL CheckUpdateInfoIni(const CString&strUrl,UPDATEINFO&stUpdateInfo);
void StartNewVersionApp(CString strNewApp);

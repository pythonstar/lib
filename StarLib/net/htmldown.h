
#pragma once
#include <atlstr.h>
#include <WinInet.h>

//�ڲ�ͨ������URLDownloadToFile�������ļ����ú����л��棬��ʱ������Ч����
HRESULT DownloadFileCachely(const CString&strUrl,LPCTSTR lpszSaveAs,int nTimeOutSeconds=10);

//�ڲ�ʵ�ַ���ͬGetHttpFileContentNoCache�����ʺ�https�������https�����ӣ����ش���E_INVALIDARG
//�ú���û��1MB��С�������ƣ��Ҹú���û�л��棬ÿ�����ر��������ļ���
HRESULT DownloadFileNoCache(const CString&strUrl,LPCTSTR lpszSaveAs,int nTimeOutSeconds=10);

/*
[�Ƽ�]
���Э����ָ������utf8��ʽ�ַ�������ú������Զ�ת�롣
�ʺϳ�����json�ַ���ֱ�ӷ��ص���utf8��ʽ�ַ�������ú���������ת����
����html�е�Դ���������Ҫ�ⲿʹ���߸�����ҳ�е������Ƿ����<meta charset="utf-8">������ת��
������ʹ�ã��ú�����Ч������Ҳ�ʺ�https
*/
HRESULT GetHttpFileContent(LPCTSTR lpszUrl,CString&strHtml,int nTimeOutSeconds=10);

/*
�ú���û�л��棬ÿ�����رض����������ݣ��ڼ�ʱ��Ч���Ϻ����á�
���ǲ��ʺ�https�������https�����ӣ����ش���E_INVALIDARG
*/
HRESULT GetHttpFileContentNoCache(const CString&strUrl, CString&strHtml,int nTimeOutSeconds=10);

/*
�ڲ�ʵ�ַ���ͬGetHttpFileContentNoCache�����ʺ�https�������https�����ӣ����ش���E_INVALIDARG
������ڴ�С���ƣ�ֻ�ܻ�ȡ1MB���ڵ����ݣ������Ƽ�ʹ�ã�GetHttpFileContentNoCache
*/
HRESULT GetHttpFileContentEx(const CString&strUrl,CString&strHtml,int nTimeOutSeconds=10);


//�ڲ�ͨ������URLDownloadToCacheFile�Ȱ��ļ����ص���ʱ�ļ����ٶ�ȡ���ݺ�ɾ���ļ�����ʱ������Ч����
BOOL URLDownloadToString(const CString&strUrl,CString&strHtml,int nTimeOutSeconds=10);



/*
�����Zip-Utils��װ����
ref: http://www.codeproject.com/Articles/7530/Zip-Utils-clean-elegant-simple-C-Win
*/
#pragma once
#include <vector>
using namespace std;
#include <zip/zip.h>
#include <zip/unzip.h>

#ifdef _USRDLL

class CZipHelperEx {

public:
	CZipHelperEx();

	BOOL Create(LPCTSTR lpszNewZipFile);
	BOOL AddFile(LPCTSTR lpszFile, LPCTSTR lpszEntryName = NULL);
	BOOL Close();

	BOOL Open(LPCTSTR lpszExistZipFile);

	//��ѹ���ļ��е�lpszEntryName��ѹ��lpszUnzipDir�£����lpszUnzipDirΪnull���ѹ��ͬĿ¼�µ�ͬ���ļ�����
	BOOL UnzipOneItem(LPCTSTR lpszEntryName, LPCTSTR lpszUnzipDir = NULL);

public:
	static BOOL Delete(LPCTSTR lpszExistZipFile, LPCTSTR lpszEntryName);
	static BOOL Add(LPCTSTR lpszExistZipFile, LPCTSTR lpszFilePath, LPCTSTR lpszEntryName = NULL);

public:
	//��lpszFileѹ����lpszExistZipFile�����ΪlpszEntryName�����lpszEntryName==null��ȡ�ļ���������ӵ���Ŀ¼��
	static BOOL PackFile(LPCTSTR lpszNewZipFile, LPCTSTR lpszFile, LPCTSTR lpszEntryName = NULL);

	//��lpszExistZipFile��ѹ��Ŀ¼lpszDir�£����lpszDirΪnull���ѹ��ͬĿ¼�µ�ͬ���ļ�����
	static BOOL UnpackFile(LPCTSTR lpszExistZipFile, LPCTSTR lpszDir = NULL);


private:
	HZIP m_hzip;
	CString m_strZipFilePath;

};
#endif

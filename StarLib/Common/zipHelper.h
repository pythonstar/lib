
#pragma once
#include <vector>
using namespace std;

class CZipHelper {

public:
	static int GetFileCrc(LPCTSTR szFileName, unsigned long* result_crc);

	//ѹ��һ���ļ���Ŀ¼��szDstFilePathΪĿ���ļ���
	static BOOL PackFile(LPCTSTR szSrcFilePath, LPCTSTR szDstFilePath, LPCTSTR szPswd=NULL);

	//ѹ��Ŀ¼�µ��ļ�Ŀ¼���һ���ַ�����б��
	static BOOL PackDir(LPCTSTR szDirPath, LPCTSTR szDstFilePath, LPCTSTR szPswd=NULL);

private:
	static void CZipHelper::RecursiveDir(LPCTSTR szDirPath, vector<CString>&vtFiles);
	static BOOL PackFiles(vector<CString>&vtFiles, LPCTSTR lpszRootPath, LPCTSTR szDstFilePath, LPCTSTR szPswd=NULL);
};

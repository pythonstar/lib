
#pragma once
#include <vector>
using namespace std;

class CZipHelper {

public:
	static int GetFileCrc(LPCTSTR szFileName, unsigned long* result_crc);

	//压缩一个文件或目录，szDstFilePath为目标文件名
	static BOOL PackFile(LPCTSTR szSrcFilePath, LPCTSTR szDstFilePath, LPCTSTR szPswd=NULL);

	//压缩目录下的文件目录最后一个字符须有斜杠
	static BOOL PackDir(LPCTSTR szDirPath, LPCTSTR szDstFilePath, LPCTSTR szPswd=NULL);

private:
	static void CZipHelper::RecursiveDir(LPCTSTR szDirPath, vector<CString>&vtFiles);
	static BOOL PackFiles(vector<CString>&vtFiles, LPCTSTR lpszRootPath, LPCTSTR szDstFilePath, LPCTSTR szPswd=NULL);
};

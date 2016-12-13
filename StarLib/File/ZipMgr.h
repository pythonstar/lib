
#pragma once
#include <vector>
using namespace std;

namespace Star
{
	namespace File
	{
		//计算文件的CRC值，返回0为成功
		int GetFileCRC(LPCTSTR szFileName,unsigned long* result_crc);

		//压缩单个文件
		BOOL PackFile(LPCSTR szSrcFilePath,LPCSTR szDstFilePath,LPCSTR szPswd/*=NULL*/);

		//压缩多个文件
		BOOL PackFiles(vector<CString>&vtFiles,LPCSTR szDstFilePath,LPCSTR szPswd/*=NULL*/);

		//压缩目录
		BOOL PackDir(LPCTSTR szDirPath,LPCSTR szDstFilePath,LPCSTR szPswd/*=NULL*/);
	}
}
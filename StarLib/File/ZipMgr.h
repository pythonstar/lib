
#pragma once
#include <vector>
using namespace std;

namespace Star
{
	namespace File
	{
		//�����ļ���CRCֵ������0Ϊ�ɹ�
		int GetFileCRC(LPCTSTR szFileName,unsigned long* result_crc);

		//ѹ�������ļ�
		BOOL PackFile(LPCSTR szSrcFilePath,LPCSTR szDstFilePath,LPCSTR szPswd/*=NULL*/);

		//ѹ������ļ�
		BOOL PackFiles(vector<CString>&vtFiles,LPCSTR szDstFilePath,LPCSTR szPswd/*=NULL*/);

		//ѹ��Ŀ¼
		BOOL PackDir(LPCTSTR szDirPath,LPCSTR szDstFilePath,LPCSTR szPswd/*=NULL*/);
	}
}
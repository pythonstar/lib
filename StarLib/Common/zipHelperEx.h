

/*
本类从Zip-Utils封装而来
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

	//把压缩文件中的lpszEntryName解压到lpszUnzipDir下，如果lpszUnzipDir为null则解压到同目录下的同名文件夹里
	BOOL UnzipOneItem(LPCTSTR lpszEntryName, LPCTSTR lpszUnzipDir = NULL);

public:
	static BOOL Delete(LPCTSTR lpszExistZipFile, LPCTSTR lpszEntryName);
	static BOOL Add(LPCTSTR lpszExistZipFile, LPCTSTR lpszFilePath, LPCTSTR lpszEntryName = NULL);

public:
	//把lpszFile压缩到lpszExistZipFile里，名字为lpszEntryName，如果lpszEntryName==null则取文件名，且添加到根目录下
	static BOOL PackFile(LPCTSTR lpszNewZipFile, LPCTSTR lpszFile, LPCTSTR lpszEntryName = NULL);

	//把lpszExistZipFile解压到目录lpszDir下，如果lpszDir为null则解压到同目录下的同名文件夹里
	static BOOL UnpackFile(LPCTSTR lpszExistZipFile, LPCTSTR lpszDir = NULL);


private:
	HZIP m_hzip;
	CString m_strZipFilePath;

};
#endif

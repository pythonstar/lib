
#include "stdafx.h"
#include "zipHelper.h"
#include <StarLib/Common/common.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <zlib/zip.h>

#ifdef __cplusplus
}
#endif


//压缩一个文件或目录，szDstFilePath为目标文件名
BOOL CZipHelper::PackFile(LPCTSTR szSrcFilePath, LPCTSTR szDstFilePath, LPCTSTR szPswd/*=NULL*/)
{
	DWORD dwAttr = GetFileAttributes(szSrcFilePath);
	if ( dwAttr==-1 ){
		return FALSE;
	}

	vector<CString>vtFiles;
	if ( dwAttr & FILE_ATTRIBUTE_DIRECTORY ) {
		RecursiveDir(szSrcFilePath,vtFiles);
		return PackFiles(vtFiles, szSrcFilePath, szDstFilePath, szPswd);
	}else{
		vtFiles.push_back(szSrcFilePath);
		return PackFiles(vtFiles, NULL, szDstFilePath,szPswd);
	}
}

BOOL CZipHelper::PackFiles(vector<CString>&vtFiles, LPCTSTR lpszRootPath, LPCTSTR szDstFilePath, LPCTSTR szPswd/*=NULL*/)
{
	//压缩缓冲大小.
	const int BUFLEN = 0x100000;
	BYTE *buff = new BYTE[BUFLEN];
	if ( buff==NULL ) {
		return FALSE;
	}

	CString strRootPath = lpszRootPath;
	CString strDstFilePath = szDstFilePath;
	CString strDstFileName;
	BOOL bStatus = FALSE;
	zipFile hZip = NULL;
	string sMsg;
	LONG64 lTotalSize = 0;
	LONG64 lTotalCompressed = 0;
	DWORD dwBytesRead=0;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	LARGE_INTEGER lFileSize;
	BOOL bGetSize = FALSE;
	unsigned long crcFile = 0;

	Star::Common::PathWithSlash(strRootPath);
	hZip = zipOpen((const char *)strDstFilePath, APPEND_STATUS_CREATE);
	if(hZip==NULL){
		goto cleanup;
	}

	for(vector<CString>::iterator iter=vtFiles.begin(); iter!=vtFiles.end(); ++iter ){ 
		if ( lpszRootPath==NULL ) {
			strDstFileName = Star::Common::GetFileName(*iter);
		}else{
			strDstFileName = (*iter).Mid(strRootPath.GetLength());
		}
		HANDLE hFile = Star::Common::CreateFile(*iter,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,NULL,NULL); 
		if(hFile==INVALID_HANDLE_VALUE){
			continue;
		}

		bGetSize = GetFileSizeEx(hFile, &lFileSize);
		if(bGetSize==FALSE){
			//_T("Couldn't get file size of %s\n")
			CloseHandle(hFile);
			continue;
		}

		lTotalSize += lFileSize.QuadPart;

		BY_HANDLE_FILE_INFORMATION fi;
		GetFileInformationByHandle(hFile, &fi);

		SYSTEMTIME st;
		FileTimeToSystemTime(&fi.ftCreationTime, &st);

		zip_fileinfo info;
		info.dosDate = 0;
		info.tmz_date.tm_year = st.wYear;
		info.tmz_date.tm_mon = st.wMonth;
		info.tmz_date.tm_mday = st.wDay;
		info.tmz_date.tm_hour = st.wHour;
		info.tmz_date.tm_min = st.wMinute;
		info.tmz_date.tm_sec = st.wSecond;
		info.external_fa = FILE_ATTRIBUTE_NORMAL;
		info.internal_fa = FILE_ATTRIBUTE_NORMAL;

		if ( szPswd!=NULL ){
			GetFileCrc(*iter, &crcFile);
		}

		int n = zipOpenNewFileInZip3(hZip, strDstFileName, &info,
			NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION,
			0, MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, szPswd, crcFile);
		if( n!=0 ){
			//Couldn't compress file
			continue;
		}

		for( ; ; ){
			BOOL bRead = ReadFile(hFile, buff, BUFLEN, &dwBytesRead, NULL);
			if( bRead==FALSE || dwBytesRead==0 )
				break;

			int res = zipWriteInFileInZip(hZip, buff, dwBytesRead);
			if(res!=0){
				zipCloseFileInZip(hZip);
				//Couldn't write to compressed file
				break;
			}

			lTotalCompressed += dwBytesRead;
		}//end while

		zipCloseFileInZip(hZip);
		CloseHandle(hFile);
	}

	if(lTotalSize==lTotalCompressed)
		bStatus = TRUE;

cleanup:
	if ( buff!=NULL ) {
		delete[] buff;
	}

	if( hZip!=NULL )
		zipClose(hZip, NULL);

	if(hFile!=INVALID_HANDLE_VALUE)
		CloseHandle(hFile);

	if( bStatus ){
		//TRACE(_T("Finished compressing files...OK"));
		//sMsg.Format(_T("Total file size for compression is %I64d"), lTotalSize);
		//TRACE(sMsg);
	}else{
		//TRACE(_T("File compression failed."));
	}

	return bStatus;
}

void CZipHelper::RecursiveDir(LPCTSTR szDirPath,vector<CString>&vtFiles)
{
	CString strSrcPath = szDirPath;
	CString strFindPath;
	Star::Common::PathWithSlash(strSrcPath);
	strFindPath = strSrcPath;
	strFindPath += _T("*");

	WIN32_FIND_DATA fd;   
	HANDLE hFindFile = FindFirstFile(strFindPath, &fd);   
	if( hFindFile==INVALID_HANDLE_VALUE ){   
		return;
	}   

	CString strFileName;

	BOOL bOK=TRUE;   
	while( bOK ){   

		//如果是.或..  
		if ( (fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)!=0 ){
			if ( _tcscmp(fd.cFileName,_T("."))!=0 && _tcscmp(fd.cFileName,_T(".."))!=0 ){
				CString strSubPath; 
				strSubPath = strSrcPath;
				strSubPath += fd.cFileName;
				RecursiveDir(strSubPath,vtFiles);
			}
		}else{
			strFileName = strSrcPath;
			strFileName.Append(fd.cFileName);
			vtFiles.push_back(strFileName);
		}
		bOK=FindNextFile(hFindFile, &fd);   
	}//end while
	::FindClose(hFindFile); 
}

//压缩目录下的文件
BOOL CZipHelper::PackDir(LPCTSTR szDirPath,LPCTSTR szDstFilePath,LPCTSTR szPswd/*=NULL*/)
{
	if ( GetFileAttributes(szDirPath)==-1 ){
		return FALSE;
	}

	CString strSrcPath = szDirPath;
	Star::Common::PathWithSlash(strSrcPath);
	int nDirLen = strSrcPath.GetLength();

	vector<CString> vtFiles;
	RecursiveDir(szDirPath,vtFiles);
	return PackFiles(vtFiles, szDstFilePath, szPswd);

	//压缩缓冲大小.
	const int BUFLEN = 0x4000;

	CStringA strDstFilePathA;
	CStringA strPasswordA;
	char *szPassword = NULL;
	CString szDstFileName;
	BOOL bStatus = FALSE;
	zipFile hZip = NULL;
	string sMsg;
	LONG64 lTotalSize = 0;
	LONG64 lTotalCompressed = 0;
	BYTE buff[BUFLEN];
	DWORD dwBytesRead=0;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	LARGE_INTEGER lFileSize;
	BOOL bGetSize = FALSE;

	unsigned long crcFile = 0;

	strDstFilePathA = szDstFilePath;
	if ( szPswd!=NULL ){
		strPasswordA = szPswd;
		szPassword = (char *)(LPCSTR)strPasswordA;
	}

	hZip = zipOpen((const char *)strDstFilePathA, APPEND_STATUS_CREATE);
	if(hZip==NULL){
		//TRACE(_T("Failed to create ZIP file."));
		goto cleanup;
	}

	for(vector<CString>::iterator iter=vtFiles.begin(); iter!=vtFiles.end(); ++iter ){ 
		LPCTSTR pFileName = *iter;
		//WideCharToMultiByte(CP_ACP,0,pFileName+nDirLen,-1,(LPSTR)szDstFileName,MAX_PATH,NULL,NULL);
		//strcpy_s(szDstFileName,MAX_PATH,ws2ms(pFileName+nDirLen).c_str());
		szDstFileName = pFileName+nDirLen;

		HANDLE hFile = Star::Common::CreateFile(*iter,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,NULL,NULL); 
		if(hFile==INVALID_HANDLE_VALUE){
			continue;
		}

		bGetSize = GetFileSizeEx(hFile, &lFileSize);
		if(bGetSize==FALSE){
			//_T("Couldn't get file size of %s\n")
			CloseHandle(hFile);
			continue;
		}

		lTotalSize += lFileSize.QuadPart;

		BY_HANDLE_FILE_INFORMATION fi;
		GetFileInformationByHandle(hFile, &fi);

		SYSTEMTIME st;
		FileTimeToSystemTime(&fi.ftCreationTime, &st);

		zip_fileinfo info;
		info.dosDate = 0;
		info.tmz_date.tm_year = st.wYear;
		info.tmz_date.tm_mon = st.wMonth;
		info.tmz_date.tm_mday = st.wDay;
		info.tmz_date.tm_hour = st.wHour;
		info.tmz_date.tm_min = st.wMinute;
		info.tmz_date.tm_sec = st.wSecond;
		info.external_fa = FILE_ATTRIBUTE_NORMAL;
		info.internal_fa = FILE_ATTRIBUTE_NORMAL;

		if ( szPswd!=NULL ){
			GetFileCrc(*iter, &crcFile);
		}

		int n = zipOpenNewFileInZip3( hZip, szDstFileName, &info,
			NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION,
			0, MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, szPassword, crcFile );
		if( n!=0 ){
			//Couldn't compress file
			continue;
		}

		for( ; ; ){
			BOOL bRead = ReadFile(hFile, buff, BUFLEN, &dwBytesRead, NULL);
			if( bRead==FALSE || dwBytesRead==0 )
				break;

			int res = zipWriteInFileInZip(hZip, buff, dwBytesRead);
			if(res!=0){
				zipCloseFileInZip(hZip);
				//Couldn't write to compressed file
				break;
			}

			lTotalCompressed += dwBytesRead;
		}//end while

		zipCloseFileInZip(hZip);
		CloseHandle(hFile);
	}

	if(lTotalSize==lTotalCompressed)
		bStatus = TRUE;

cleanup:

	if( hZip!=NULL )
		zipClose(hZip, NULL);

	if(hFile!=INVALID_HANDLE_VALUE)
		CloseHandle(hFile);

	if( bStatus ){
		//TRACE(_T("Finished compressing files...OK"));
		//sMsg.Format(_T("Total file size for compression is %I64d"), lTotalSize);
		//TRACE(sMsg);
	}else{
		//TRACE(_T("File compression failed."));
	}

	return bStatus;
}


int CZipHelper::GetFileCrc(LPCTSTR szFileName, unsigned long* result_crc)
{
	const int size_buf = 0x4000;
	PBYTE buf = new BYTE[size_buf];
	if ( buf==NULL ){
		return ZIP_ERRNO;
	}

	int err = ZIP_OK;
	unsigned long calculate_crc=0;
	DWORD dwBytesRead = 0;
	HANDLE hFile = Star::Common::CreateFile(szFileName, GENERIC_READ , FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	if ( hFile!=INVALID_HANDLE_VALUE ) {

		while( ReadFile(hFile, (LPVOID)(buf), size_buf, &dwBytesRead, NULL) ) {
			if ( dwBytesRead == 0 )	{
				break;
			}

			calculate_crc = crc32(calculate_crc,(const Bytef *)buf,dwBytesRead);
		}

		CloseHandle(hFile);
	}else{
		err = ZIP_ERRNO;
	}


	if ( buf!=NULL ) {
		delete []buf;
	}

	*result_crc=calculate_crc;
	return err;
}

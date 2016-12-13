
#include "stdafx.h"
#include "../../zlib/zip.h"
#include "ZipMgr.h"



//计算文件的CRC值，返回0为成功
int Star::File::GetFileCRC(LPCTSTR szFileName,unsigned long* result_crc)
{
	const int size_buf=0x4000;
	PBYTE buf=new BYTE[size_buf];
	if ( buf==NULL ){
		return ZIP_ERRNO;
	}

	unsigned long calculate_crc=0;
	int err=ZIP_OK;
	FILE *fin = NULL;
	_tfopen_s(&fin,szFileName, _T("rb"));
	unsigned long size_read = 0;
	unsigned long total_read = 0;
	if (fin==NULL)
	{
		err = ZIP_ERRNO;
	}

	if (err == ZIP_OK)
		do
		{
			err = ZIP_OK;
			size_read = (int)fread(buf,1,size_buf,fin);
			if (size_read < size_buf)
				if (feof(fin)==0)
				{
					//ATLTRACE("error in reading %s\n", filenameinzip);
					err = ZIP_ERRNO;
				}

				if (size_read>0)
					calculate_crc = crc32(calculate_crc,(const Bytef *)buf,size_read);
				total_read += size_read;

		} while ((err == ZIP_OK) && (size_read>0));

		if (fin)
			fclose(fin);

		*result_crc=calculate_crc;
		//ATLTRACE("file %s crc %x\n",filenameinzip,calculate_crc);

		delete []buf;

		return err;
}

//压缩单个文件
BOOL Star::File::PackFile(LPCTSTR szSrcFilePath,LPCTSTR szDstFilePath,LPCTSTR szPswd/*=NULL*/)
{
	vector<CString>vtFiles;
	vtFiles.push_back(szSrcFilePath);
	return PackFiles(vtFiles,szDstFilePath,szPswd);
}

//压缩多个文件
BOOL Star::File::PackFiles(vector<CString>&vtFiles,LPCSTR szDstFilePath,LPCSTR szPswd/*=NULL*/)
{
	// 压缩缓冲大小.
	const int BUFLEN = 0x4000;

	CHAR szDstFileName[MAX_PATH] = {0};
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

	hZip = zipOpen(szDstFilePath, APPEND_STATUS_CREATE);
	if(hZip==NULL){
		//TRACE(_T("Failed to create ZIP file."));
		goto cleanup;
	}

	for(vector<CString>::iterator iter=vtFiles.begin(); iter!=vtFiles.end(); ++iter ){ 
		LPCSTR pFileName=strchr(*iter,'\\');
		if ( pFileName==NULL ){
			continue;
		}
		strcpy_s(szDstFileName,MAX_PATH,pFileName+1);

		HANDLE hFile=CreateFile(*iter,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,NULL,NULL); 
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
			GetFileCRC(*iter, &crcFile);
		}

		int n = zipOpenNewFileInZip3( hZip, (const char *)szDstFileName, &info,
			NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION,
			0, MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, szPswd, crcFile );
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

void RecursiveDir(LPCTSTR szDirPath,vector<CString>&vtFiles)
{
	TCHAR szDir[MAX_PATH]={0};
	_tcscpy_s(szDir, _countof(szDir), szDirPath);
	int nLen = (int)_tcslen(szDir);
	if ( szDir[nLen]!='\\' ){
		szDir[nLen++]='\\';
	}
	_tcscat_s(szDir, _countof(szDir), _T("*"));

	WIN32_FIND_DATA fd;   
	HANDLE hFindFile = FindFirstFile(szDir, &fd);   
	if( hFindFile==INVALID_HANDLE_VALUE ){   
		return;
	}   

	CString strFileName;
	BOOL bOK=TRUE;   
	while( bOK ){   

		//如果是.或..  
		if ( (fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)!=0 ){
			if ( _tcscmp(fd.cFileName,_T("."))!=0 && _tcscmp(fd.cFileName,_T(".."))!=0 ){
				strFileName = szDirPath;
				strFileName += fd.cFileName;
				strFileName += _T("\\");
				RecursiveDir(strFileName,vtFiles);
			}
		}else{
			strFileName = szDirPath;
			strFileName += fd.cFileName;
			vtFiles.push_back(strFileName);
		}
		bOK = FindNextFile(hFindFile, &fd); 

	}//end while

	::FindClose(hFindFile); 
}

//压缩目录
BOOL Star::File::PackDir(LPCTSTR szDirPath,LPCSTR szDstFilePath,LPCSTR szPswd/*=NULL*/)
{
	if ( GetFileAttributes(szDirPath)==-1 ){
		return FALSE;
	}

	TCHAR szDir[MAX_PATH]={0};
	_tcscpy_s(szDir, _countof(szDir), szDirPath);
	int nDirLen = (int)_tcslen(szDir);
	if ( szDir[nDirLen-1]!='\\' ){
		szDir[nDirLen++]='\\';
	}

	vector<CString> vtFiles;
	RecursiveDir(szDirPath,vtFiles);

	// 压缩缓冲大小.
	const int BUFLEN = 0x4000;

	CHAR szDstFileName[MAX_PATH] = {0};
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

	hZip = zipOpen(szDstFilePath, APPEND_STATUS_CREATE);
	if(hZip==NULL){
		//TRACE(_T("Failed to create ZIP file."));
		goto cleanup;
	}

	for(vector<CString>::iterator iter=vtFiles.begin(); iter!=vtFiles.end(); ++iter ){ 
		LPCTSTR pFileName = *iter;
		strcpy_s(szDstFileName,MAX_PATH,pFileName+nDirLen);

		HANDLE hFile=CreateFile(*iter,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,NULL,NULL); 
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
			GetFileCRC(*iter, &crcFile);
		}

		int n = zipOpenNewFileInZip3( hZip, (const char *)szDstFileName, &info,
			NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION,
			0, MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, NULL, crcFile );
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

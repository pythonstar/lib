
#include "stdafx.h"
#include "zipHelperEx.h"
#include <StarLib/Common/common.h>

#ifdef _USRDLL

CZipHelperEx::CZipHelperEx()
{
	m_hzip = NULL;
}

BOOL CZipHelperEx::Create(LPCTSTR lpszNewZipFile)
{
	if ( m_hzip!=NULL ) {
		CloseZip(m_hzip);
	}
	m_hzip = CreateZip(lpszNewZipFile, 0);
	m_strZipFilePath = lpszNewZipFile;
	return m_hzip!=NULL;
}

BOOL CZipHelperEx::AddFile(LPCTSTR lpszFile, LPCTSTR lpszEntryName)
{
	BOOL bOK = FALSE;
	if ( m_hzip!=NULL ) {
		CString strEntryName;
		if ( lpszEntryName==NULL ) {
			strEntryName = Star::Common::GetFileName(lpszFile);
			lpszEntryName = strEntryName;
		}
		ZRESULT ret = ZipAdd(m_hzip, lpszEntryName, lpszFile);
		bOK = (ret==0);
	}

	return bOK;
}

BOOL CZipHelperEx::Open(LPCTSTR lpszExistZipFile)
{
	if ( m_hzip!=NULL ) {
		CloseZip(m_hzip);
	}
	m_hzip = OpenZip(lpszExistZipFile, 0);
	m_strZipFilePath = lpszExistZipFile;
	return m_hzip!=NULL;
}

BOOL CZipHelperEx::Close()
{
	BOOL bOK = TRUE;
	if ( m_hzip!=NULL ) {
		bOK = (CloseZip(m_hzip)==0);
		m_hzip = NULL;
	}

	return bOK;
}

BOOL CZipHelperEx::UnzipOneItem(LPCTSTR lpszEntryName, LPCTSTR lpszUnzipDir)
{
	if ( m_hzip==NULL ) {
		return FALSE;
	}

	if ( lpszUnzipDir==NULL ) {
		SetUnzipBaseDir(m_hzip, Star::Common::GetParentPath(m_strZipFilePath));
	}else{
		SetUnzipBaseDir(m_hzip, lpszUnzipDir);
	}

	ZIPENTRY ze;
	int i = -1; 
	DWORD ret = 0;
	if ( FindZipItem(m_hzip, lpszEntryName, true, &i, &ze)==0 ) {
		ret = UnzipItem(m_hzip, i, ze.name);
	}
	return (ret==0);
}

BOOL CZipHelperEx::PackFile(LPCTSTR lpszNewZipFile, LPCTSTR lpszFile, LPCTSTR lpszEntryName)
{
	HZIP hz = CreateZip(lpszNewZipFile, 0);
	CString strEntryName;
	if ( lpszEntryName==NULL ) {
		strEntryName = Star::Common::GetFileName(lpszFile);
		lpszEntryName = strEntryName;
	}
	ZipAdd(hz, lpszEntryName, lpszFile);
	CloseZip(hz);
	return TRUE;
}

BOOL CZipHelperEx::UnpackFile(LPCTSTR lpszExistZipFile, LPCTSTR lpszDir/* = NULL*/)
{
	HZIP hz = OpenZip(lpszExistZipFile, 0);
	if ( hz==NULL ) {
		return FALSE;
	}

	if ( lpszDir==NULL ) {
		SetUnzipBaseDir(hz, Star::Common::GetParentPath(lpszExistZipFile) + Star::Common::GetFileNameWithoutExt(lpszExistZipFile));
	}else{
		SetUnzipBaseDir(hz, lpszDir);
	}

	ZIPENTRY ze; GetZipItem(hz,-1,&ze); int numitems=ze.index;
	for (int zi=0; zi<numitems; zi++) {	
		GetZipItem(hz,zi,&ze);
		UnzipItem(hz,zi,ze.name);
	}
	CloseZip(hz);
	return TRUE;
}


// AddFileToZip: adds a file to a zip, possibly replacing what was there before
// zipfn ="c:\\archive.zip"             (the fn of the zip file) 
// zefn  ="c:\\my documents\\file.txt"  (the fn of the file to be added)
// zename="file.txt"                    (the name that zefn will take inside the zip)
// If zefn is empty, we just delete zename from the zip archive.
// The way it works is that we create a temporary zipfile, and copy the original
// contents into the new one (with the appropriate addition or substitution)
// and then remove the old one and rename the new one. NB. we are case-insensitive.
//ZRESULT RemoveFileFromZip(const TCHAR *zipfn, const TCHAR *zename)
//{ 
//	return AddFileToZip(zipfn,zename,0);
//}

ZRESULT AddFileToZip(const TCHAR *zipfn, const TCHAR *zename, const TCHAR *zefn)
{ 
	if (GetFileAttributes(zipfn)==0xFFFFFFFF || (zefn!=0 && GetFileAttributes(zefn)==0xFFFFFFFF)) return ZR_NOFILE;
	// Expected size of the new zip will be the size of the old zip plus the size of the new file
	HANDLE hf=CreateFile(zipfn,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,0,0); if (hf==INVALID_HANDLE_VALUE) return ZR_NOFILE; DWORD size=GetFileSize(hf,0); CloseHandle(hf);
	if (zefn!=0) {hf=CreateFile(zefn,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,0,0); if (hf==INVALID_HANDLE_VALUE) return ZR_NOFILE; size+=GetFileSize(hf,0); CloseHandle(hf);}
	size*=2; // just to be on the safe side.
	//
	HZIP hzsrc=OpenZip(zipfn,0); if (hzsrc==0) return ZR_READ;
	HZIP hzdst=CreateZip(0,size,0); if (hzdst==0) {CloseZip(hzsrc); return ZR_WRITE;}
	// hzdst is created in the system pagefile
	// Now go through the old zip, unzipping each item into a memory buffer, and adding it to the new one
	char *buf=0; unsigned int bufsize=0; // we'll unzip each item into this memory buffer
	ZIPENTRY ze; ZRESULT zr=GetZipItem(hzsrc,-1,&ze); int numitems=ze.index; if (zr!=ZR_OK) {CloseZip(hzsrc); CloseZip(hzdst); return zr;}
	for (int i=0; i<numitems; i++)
	{ zr=GetZipItem(hzsrc,i,&ze); if (zr!=ZR_OK) {CloseZip(hzsrc); CloseZip(hzdst); return zr;}
		if (_stricmp(ze.name,zename)==0) continue; // don't copy over the old version of the file we're changing
		if (ze.attr&FILE_ATTRIBUTE_DIRECTORY) {zr=ZipAddFolder(hzdst,ze.name); if (zr!=ZR_OK) {CloseZip(hzsrc); CloseZip(hzdst); return zr;} continue;}
		if (ze.unc_size>(long)bufsize) {if (buf!=0) delete[] buf; bufsize=ze.unc_size*2; buf=new char[bufsize];}
		zr=UnzipItem(hzsrc,i,buf,bufsize); if (zr!=ZR_OK) {CloseZip(hzsrc); CloseZip(hzdst); return zr;}
		zr=ZipAdd(hzdst,ze.name,buf,bufsize); if (zr!=ZR_OK) {CloseZip(hzsrc); CloseZip(hzdst); return zr;}
	}
	delete[] buf;
	// Now add the new file
	if (zefn!=0) {zr=ZipAdd(hzdst,zename,zefn); if (zr!=ZR_OK) {CloseZip(hzsrc); CloseZip(hzdst); return zr;}}
	zr=CloseZip(hzsrc); if (zr!=ZR_OK) {CloseZip(hzdst); return zr;}
	//
	// The new file has been put into pagefile memory. Let's store it to disk, overwriting the original zip
	zr=ZipGetMemory(hzdst,(void**)&buf,&size); if (zr!=ZR_OK) {CloseZip(hzdst); return zr;}
	hf=CreateFile(zipfn,GENERIC_WRITE,0,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0); if (hf==INVALID_HANDLE_VALUE) {CloseZip(hzdst); return ZR_WRITE;}
	DWORD writ; WriteFile(hf,buf,size,&writ,0); CloseHandle(hf);
	zr=CloseZip(hzdst); if (zr!=ZR_OK) return zr;
	return ZR_OK;
}

BOOL CZipHelperEx::Delete(LPCTSTR lpszExistZipFile, LPCTSTR lpszEntryName)
{
	return AddFileToZip(lpszExistZipFile, lpszEntryName, 0)==ZR_OK;
}

BOOL CZipHelperEx::Add(LPCTSTR lpszExistZipFile, LPCTSTR lpszFilePath, LPCTSTR lpszEntryName)
{
	CString strEntryName;
	if ( lpszEntryName==NULL ) {
		strEntryName = Star::Common::GetFileName(lpszFilePath);
		lpszEntryName = strEntryName;
	}
	return AddFileToZip(lpszExistZipFile, lpszEntryName, lpszFilePath);
}
#endif

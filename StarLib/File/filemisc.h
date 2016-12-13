#ifndef _MISCFILE_FUNCTIONS_H_
#define _MISCFILE_FUNCTIONS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\3rdParty\stdiofileex.h"

#include <time.h>

class CFileBackup
{
public:
	CFileBackup(const CString& sFile = _T(""), 
				const CString& sFolder = _T(""), 
				BOOL bTimeStamp = FALSE, 
				const CString& sExt = _T(".bak"));
	~CFileBackup();

	BOOL MakeBackup(const CString& sFile, 
					const CString& sFolder = _T(""), 
					BOOL bTimeStamp = FALSE, 
					const CString& sExt = _T(".bak"));
	BOOL RestoreBackup();

	static CString BuildBackupPath(const CString& sFile, const CString& sFolder = _T(""), BOOL bTimeStamp = FALSE, const CString& sExt = ".bak");

protected:
	CString m_sFile, m_sBackup;
};

class CTempFileBackup : public CFileBackup
{
public:
	CTempFileBackup(const CString& sFile = _T(""), 
					const CString& sFolder = _T(""), 
					BOOL bTimeStamp = FALSE, 
					const CString& sExt = _T(".bak"));	
	~CTempFileBackup(); // auto deletes backup file

};

namespace FileMisc
{
	CString& TerminatePath(CString& sPath);
	CString& UnterminatePath(CString& sPath);
	CString TerminatePath(LPCTSTR szPath);
	CString UnterminatePath(LPCTSTR szPath);

	time_t GetLastModified(LPCTSTR szPath);
	BOOL GetLastModified(LPCTSTR szPath, SYSTEMTIME& sysTime, BOOL bLocalTime = true); // files only
	BOOL ResetLastModified(LPCTSTR szPath); // resets to current time
	double GetFileSize(LPCTSTR szPath);

	BOOL RemoveFolder(LPCTSTR szFolder, 
					  HANDLE hTerminate = NULL, 
					  BOOL bProcessMsgLoop = TRUE);

	BOOL DeleteFolderContents(LPCTSTR szFolder, 
							  BOOL bIncludeSubFolders, 
							  LPCTSTR szFileMask, 
							  HANDLE hTerminate = NULL, 
							  BOOL bProcessMsgLoop = TRUE);

	double GetFolderSize(LPCTSTR szFolder, 
						 BOOL bIncludeSubFolders = TRUE, 
						 LPCTSTR szFileMask = NULL, 
						 HANDLE hTerminate = NULL, 
						 BOOL bProcessMsgLoop = TRUE);

	BOOL CreateFolder(LPCTSTR szFolder);
	BOOL CreateFolderFromFilePath(LPCTSTR szFilePath);
	BOOL FolderExists(LPCTSTR szFolder);
	BOOL FileExists(LPCTSTR szFile);
	BOOL PathExists(LPCTSTR szPath); // file or folder
	BOOL FolderFromFilePathExists(LPCTSTR szFilePath);
	BOOL PathHasWildcard(LPCTSTR szFilePath);

	CString GetCwd();
	void SetCwd(const CString& sCwd);

	int FindFiles(const CString& sFolder, CStringArray& aFiles, BOOL bCheckSubFolders = TRUE, LPCTSTR szPattern = _T("*.*"));
	BOOL FolderContainsFiles(LPCTSTR szFolder, BOOL bCheckSubFolders = TRUE, LPCTSTR szFilter = _T("*.*"));

	CString& ValidateFilename(CString& sFilename, LPCTSTR szReplace = _T(""));
	CString& ValidateFilepath(CString& sFilepath, LPCTSTR szReplace = _T(""));
	void ReplaceExtension(CString& szFilePath, LPCTSTR szExt);
	
	CString GetTempFolder();
	CString GetTempFileName(LPCTSTR szPrefix, UINT uUnique = 0);
	CString GetTempFileName(LPCTSTR szFilename, LPCTSTR szExt);
	BOOL IsTempFile(LPCTSTR szFilename);

	BOOL CanOpenFile(LPCTSTR szPathname, BOOL bDenyWrite = FALSE);
	BOOL LoadFile(LPCTSTR szPathname, CString& sText);
	int LoadFileLines(LPCTSTR szPathname, CStringArray& aLines, int nLineCount = -1);

	BOOL SaveFile(LPCTSTR szPathname, const CString& sText, SFE_SAVEAS nSaveAs = SFE_ASCOMPILED);

	BOOL ExtractResource(UINT nID, LPCTSTR szType, const CString& sTempFilePath, HINSTANCE hInst = NULL);
	BOOL ExtractResource(LPCTSTR szModulePath, UINT nID, LPCTSTR szType, const CString& sTempFilePath);

	CString GetModuleFileName(HMODULE hMod = NULL);
	CString GetModuleFolder(HMODULE hMod = NULL);

	CString GetAppFileName();
	CString GetAppFolder(LPCTSTR szSubFolder = NULL);
	CString GetAppResourceFolder(LPCTSTR szResFolder = _T("Resources"));

	BOOL HasExtension(LPCTSTR szFilePath, LPCTSTR szExt);
	CString GetExtension(LPCTSTR szFilePath);

	CString GetWindowsFolder();
	CString GetWindowsSystemFolder();

	void SplitPath(LPCTSTR szPath, CString* pDrive, CString* pDir = NULL, CString* pFName = NULL, CString* pExt = NULL);
	CString& MakePath(CString& sPath, LPCTSTR szDrive, LPCTSTR szDir = NULL, LPCTSTR szFName = NULL, LPCTSTR szExt = NULL);
	
	CString GetRelativePath(const CString& sFilePath, const CString& sRelativeToFolder, BOOL bFolder);
	CString& MakeRelativePath(CString& sFilePath, const CString& sRelativeToFolder, BOOL bFolder);
	CString GetFullPath(const CString& sFilePath, const CString& sRelativeToFolder = GetCwd());
	CString& MakeFullPath(CString& sFilePath, const CString& sRelativeToFolder = GetCwd());

	CString ResolveShortcut(LPCTSTR szShortcut);

	BOOL IsSameFile(const CString& sFilePath1, const CString& sFilePath2);

	CString GetFolderFromFilePath(LPCTSTR szFilePath);
	CString GetFileNameFromPath(LPCTSTR szFilepath, BOOL bIncExtension = TRUE);

	// will delete the source folder on success
	BOOL MoveFolder(LPCTSTR szSrcFolder, 
					LPCTSTR szDestFolder, 
					HANDLE hTerminate = NULL, 
					BOOL bProcessMsgLoop = TRUE);

	BOOL CopyFolder(LPCTSTR szSrcFolder, 
					LPCTSTR szDestFolder, 
					HANDLE hTerminate = NULL, 
					BOOL bProcessMsgLoop = TRUE);

	// will delete the source folder only if file mask was "*.*"
	BOOL MoveFolder(LPCTSTR szSrcFolder, 
					LPCTSTR szDestFolder, 
					BOOL bIncludeSubFolders, 
					LPCTSTR szFileMask, 
					HANDLE hTerminate = NULL, 
					BOOL bProcessMsgLoop = TRUE);

	BOOL CopyFolder(LPCTSTR szSrcFolder, 
					LPCTSTR szDestFolder, 
					BOOL bIncludeSubFolders, 
					LPCTSTR szFileMask, 
					HANDLE hTerminate = NULL, 
					BOOL bProcessMsgLoop = TRUE);


	void EnableLogging(BOOL bEnable = TRUE, BOOL bResetLogFile = TRUE, BOOL bWantDateTime = TRUE);
	void LogTimeElapsed(DWORD& dwTickStart, LPCTSTR szFormat, ...);
	void LogText(LPCTSTR szText, ...);

	// append a line of text to a text file
	BOOL AppendLineToFile(LPCTSTR szPathname, LPCTSTR szLine, SFE_SAVEAS nSaveAs = SFE_ASIS);

	DWORD Run(HWND hwnd, LPCTSTR lpFile, LPCTSTR lpDirectory = NULL, int nShowCmd = SW_SHOW);

}

#endif // _MISCFILE_FUNCTIONS_H_
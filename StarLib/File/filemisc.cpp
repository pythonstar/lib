#include "stdafx.h"
#include "filemisc.h"
#include "fileregister.h"
#include "misc.h"

#include "..\3rdParty\stdiofileex.h"

#include <sys/utime.h>
#include <sys/stat.h>
#include <shlwapi.h>
#include <io.h>

#pragma comment(lib, "shlwapi.lib")

static BOOL s_bLogging = FALSE;
static BOOL s_bLogDateTime = TRUE;

///////////////////////////////////////////////////////////////////////////////////////////////////

CTempFileBackup::CTempFileBackup(const CString& sFile, const CString& sFolder, BOOL bTimeStamp, const CString& sExt)
	: CFileBackup(sFile, sFolder, bTimeStamp, sExt)
{
}

CTempFileBackup::~CTempFileBackup()
{
	if (FileMisc::FileExists(m_sBackup))
		::DeleteFile(m_sBackup);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

CFileBackup::CFileBackup(const CString& sFile, const CString& sFolder, BOOL bTimeStamp, const CString& sExt)
{
	MakeBackup(sFile, sFolder, bTimeStamp, sExt);
}

CFileBackup::~CFileBackup()
{
}

BOOL CFileBackup::MakeBackup(const CString& sFile, const CString& sFolder, BOOL bTimeStamp, const CString& sExt)
{
	ASSERT (m_sFile.IsEmpty() && m_sBackup.IsEmpty());

	if (!m_sFile.IsEmpty() || !m_sBackup.IsEmpty())
		return FALSE;

	if (!FileMisc::FileExists(sFile))
		return FALSE;

	m_sFile = sFile;
	m_sBackup = BuildBackupPath(sFile, sFolder, bTimeStamp, sExt);
	FileMisc::CreateFolderFromFilePath(m_sBackup);

	BOOL bRes = ::CopyFile(m_sFile, m_sBackup, FALSE);

	if (!bRes)
	{
		TRACE(Misc::FormatGetLastError() + '\n');
	}
	else
		ASSERT (FileMisc::FileExists(m_sBackup));

	return bRes;
}

BOOL CFileBackup::RestoreBackup()
{
	ASSERT (!m_sFile.IsEmpty() && !m_sBackup.IsEmpty());

	if (m_sFile.IsEmpty() || m_sBackup.IsEmpty())
		return FALSE;

	return ::CopyFile(m_sBackup, m_sFile, FALSE);
}

CString CFileBackup::BuildBackupPath(const CString& sFile, const CString& sFolder, BOOL bTimeStamp, const CString& sExt)
{
	CString sBackup(sFile);
	sBackup.TrimRight();

	// handle folder
	if (!sFolder.IsEmpty())
	{
		CString sDrive, sPath, sFName, sExt;
		FileMisc::SplitPath(sFile, &sDrive, &sPath, &sFName, &sExt);

		if (::PathIsRelative(sFolder))
		{
			sPath += sFolder;
			FileMisc::MakePath(sBackup, sDrive, sPath, sFName, sExt);
		}
		else
			FileMisc::MakePath(sBackup, NULL, sFolder, sFName, sExt);
	}

	// add timestamp before existing file extension
	CString sFExt;
	FileMisc::SplitPath(sBackup, NULL, NULL, NULL, &sFExt);

	if (bTimeStamp)
	{
		// use ISO date and 24 hour time so that backups can be sorted 
		// by name in date order
		CString sDate = COleDateTime::GetCurrentTime().Format(_T("%Y-%m-%d_%H-%M-%S"));
		sFExt = "." + sDate + sFExt;
	}

	// add extension before existing file extension
	if (sExt.IsEmpty())
	{
		// only add a default extension if not copying to another folder or not adding timestamp
		// else we'd overwrite the existing file which wouldn't achieve much
		if (sFolder.IsEmpty() && !bTimeStamp)
			sFExt = _T(".bak") + sFExt;
	}
	else
	{
		if (sExt.Find('.') == -1)
			sFExt = '.' + sExt + sFExt;
		else
			sFExt = sExt + sFExt;
	}

	FileMisc::ReplaceExtension(sBackup, sFExt);

	return sBackup;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

CString& FileMisc::TerminatePath(CString& sPath)
{
	sPath.TrimRight();

	if (sPath.ReverseFind('\\') != (sPath.GetLength() - 1))
		sPath += '\\';

	return sPath;
}

CString FileMisc::TerminatePath(LPCTSTR szPath)
{
	CString sPath(szPath);
	return TerminatePath(sPath);
}

CString& FileMisc::UnterminatePath(CString& sPath)
{
	sPath.TrimRight();

	int len = sPath.GetLength();

	if (sPath.ReverseFind('\\') == (len - 1))
		sPath = sPath.Left(len - 1);

	return sPath;
}

CString FileMisc::UnterminatePath(LPCTSTR szPath)
{
	CString sPath(szPath);
	return UnterminatePath(sPath);
}

void FileMisc::ReplaceExtension(CString& sFilePath, LPCTSTR szExt)
{
	CString sDrive, sDir, sFile;

	SplitPath(sFilePath, &sDrive, &sDir, &sFile);
	MakePath(sFilePath, sDrive, sDir, sFile, szExt);
}

CString& FileMisc::ValidateFilepath(CString& sFilepath, LPCTSTR szReplace)
{
	sFilepath.TrimLeft();
	sFilepath.TrimRight();

	sFilepath.Replace(_T("/"), szReplace);
	sFilepath.Replace(_T("*"), szReplace);
	sFilepath.Replace(_T("?"), szReplace);
	sFilepath.Replace(_T("\""), szReplace);
	sFilepath.Replace(_T("<"), szReplace);
	sFilepath.Replace(_T(">"), szReplace);
	sFilepath.Replace(_T("|"), szReplace);

	// make sure if a colon exists it is the 2nd pos
	int nColon = sFilepath.ReverseFind(':');

	while (nColon != -1 && nColon != 1)
	{
		// replace the colon
		sFilepath = sFilepath.Left(nColon) + szReplace + sFilepath.Mid(nColon + 1);
		nColon = sFilepath.ReverseFind(':');
	}

	return sFilepath;
}

CString& FileMisc::ValidateFilename(CString& sFilename, LPCTSTR szReplace)
{
	sFilename.Replace(_T("\\"), szReplace);
	sFilename.Replace(_T(":"), szReplace);
	
	return ValidateFilepath(sFilename, szReplace);
}

CString FileMisc::GetFileNameFromPath(LPCTSTR szFilepath, BOOL bIncExtension)
{
	CString sFName, sExt;
	SplitPath(szFilepath, NULL, NULL, &sFName, &sExt);

	if (bIncExtension)
		sFName += sExt;
	
	return sFName;
}

time_t FileMisc::GetLastModified(LPCTSTR szPath)
{
	struct _stat st;

	if (!szPath || _tstat(szPath, &st) != 0)
		return 0;

	// files only
	if ((st.st_mode & _S_IFDIR) == _S_IFDIR)
		return 0;

	return st.st_mtime;
}

BOOL FileMisc::GetLastModified(LPCTSTR szPath, SYSTEMTIME& sysTime, BOOL bLocalTime)
{
	ZeroMemory(&sysTime, sizeof(SYSTEMTIME));

	DWORD dwAttr = ::GetFileAttributes(szPath);

	// files only
	if (dwAttr == 0xFFFFFFFF)
		return false;

	WIN32_FIND_DATA findFileData;
	HANDLE hFind = FindFirstFile((LPTSTR)szPath, &findFileData);

	if (hFind == INVALID_HANDLE_VALUE)
		return FALSE;

	FindClose(hFind);

	FILETIME ft = findFileData.ftLastWriteTime;

	if (bLocalTime)
		FileTimeToLocalFileTime(&findFileData.ftLastWriteTime, &ft);

	FileTimeToSystemTime(&ft, &sysTime);
	return true;
}

BOOL FileMisc::ResetLastModified(LPCTSTR szPath)
{
	::SetFileAttributes(szPath, FILE_ATTRIBUTE_NORMAL);

	return (_tutime(szPath, NULL) == 0);
}

int FileMisc::FindFiles(const CString& sFolder, CStringArray& aFiles, BOOL bCheckSubFolders, LPCTSTR szPattern)
{
	CFileFind ff;
	CString sSearchSpec;

	MakePath(sSearchSpec, NULL, sFolder, szPattern, NULL);

	BOOL bContinue = ff.FindFile(sSearchSpec);
	
	while (bContinue)
	{
		bContinue = ff.FindNextFile();
		
		if (!ff.IsDots())
		{
			if (ff.IsDirectory())
			{
				if (bCheckSubFolders)
					FindFiles(ff.GetFilePath(), aFiles, TRUE, szPattern);
			}
			else
				aFiles.Add(ff.GetFilePath());
		}
	}

	return aFiles.GetSize();
}

BOOL FileMisc::DeleteFolderContents(LPCTSTR szFolder, BOOL bIncludeSubFolders, LPCTSTR szFileMask, HANDLE hTerminate, BOOL bProcessMsgLoop)
{
	// if the dir does not exists just return
	if (!FolderExists(szFolder))
		return true;

	// if a file mask has been specified with subfolders we need to do 2 passes on each folder, 
	// one for the files and one for the sub folders
	int nPasses = (bIncludeSubFolders && (szFileMask && lstrlen(szFileMask))) ? 2 : 1;
		
	BOOL bResult = true;
	BOOL bStopped = (WaitForSingleObject(hTerminate, 0) == WAIT_OBJECT_0);

	for (int nPass = 0; !bStopped && nPass < nPasses; nPass++)
	{
		CString sSearchSpec(szFolder), sMask(szFileMask);

		if (sMask.IsEmpty() || nPass == 1) // (nPass == 1) == 2nd pass (for folders)
			sMask = _T("*.*");

		TerminatePath(sSearchSpec);
		sSearchSpec += sMask;

		WIN32_FIND_DATA finfo;
		HANDLE hSearch = NULL;

		if ((hSearch = FindFirstFile(sSearchSpec, &finfo)) != INVALID_HANDLE_VALUE) 
		{
			do 
			{
				if (bProcessMsgLoop)
					Misc::ProcessMsgLoop();

				if (finfo.cFileName[0] != '.') 
				{
					CString sItem(szFolder);
					sItem += _T("\\");
					sItem += finfo.cFileName;

					if (finfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						if (bIncludeSubFolders && (nPass == 1 || nPasses == 1))
						{
							if (DeleteFolderContents(sItem, TRUE, szFileMask, hTerminate, bProcessMsgLoop))
							{
								if (!szFileMask || !lstrlen(szFileMask))
									bResult = (RemoveDirectory(sItem) == TRUE);
							}
						}
					}
					else 
						bResult = (DeleteFile(sItem) == TRUE);
				}

				bStopped = (WaitForSingleObject(hTerminate, 0) == WAIT_OBJECT_0);
			} 
			while (!bStopped && bResult && FindNextFile(hSearch, &finfo));
			
			FindClose(hSearch);
		}
	}

	return (!bStopped && bResult);
}

BOOL FileMisc::RemoveFolder(LPCTSTR szFolder, HANDLE hTerminate, BOOL bProcessMsgLoop)
{
	// if the dir does not exists just return
	if (!FolderExists(szFolder))
		return true;

	if (DeleteFolderContents(szFolder, TRUE, NULL, hTerminate, bProcessMsgLoop))
	{
		::SetFileAttributes(szFolder, FILE_ATTRIBUTE_NORMAL);
		return (RemoveDirectory(szFolder) == TRUE);
	}

	return false;
}

double FileMisc::GetFolderSize(LPCTSTR szFolder, BOOL bIncludeSubFolders, LPCTSTR szFileMask, HANDLE hTerminate, BOOL bProcessMsgLoop)
{
	// if the dir does not exists just return
	if (!FolderExists(szFolder))
		return 0;
	
	double dSize = 0;

	WIN32_FIND_DATA finfo;
	CString sSearchSpec(szFolder), sFileMask(szFileMask);

	if (sFileMask.IsEmpty())
		sFileMask = _T("*.*");

	TerminatePath(sSearchSpec);
	sSearchSpec += sFileMask;

	BOOL bStopped = (WaitForSingleObject(hTerminate, 0) == WAIT_OBJECT_0);
	HANDLE h = NULL;
		
	if (!bStopped && (h = FindFirstFile(sSearchSpec, &finfo)) != INVALID_HANDLE_VALUE) 
	{
		do 
		{
			if (bProcessMsgLoop)
				Misc::ProcessMsgLoop();

			if (finfo.cFileName[0] != '.') 
			{
				if (finfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if (bIncludeSubFolders)
					{
						CString sSubFolder(szFolder);
						sSubFolder += "\\";
						sSubFolder += finfo.cFileName;
						
						dSize += GetFolderSize(sSubFolder, TRUE, sFileMask, hTerminate, bProcessMsgLoop);
					}
				}
				else 
					dSize += (finfo.nFileSizeHigh * ((double)MAXDWORD + 1)) + finfo.nFileSizeLow;
			}

			bStopped = (WaitForSingleObject(hTerminate, 0) == WAIT_OBJECT_0);
		}
		while (!bStopped && FindNextFile(h, &finfo));
		
		FindClose(h);
	} 

	return bStopped ? -1 : dSize;
}

BOOL FileMisc::PathExists(LPCTSTR szPath)
{
	// special case
	if (!szPath || !*szPath) // cwd
		return TRUE;

	return (::GetFileAttributes(szPath) != 0xffffffff);
}

BOOL FileMisc::FolderExists(LPCTSTR szFolder)
{
	// special case
	if (!szFolder || !*szFolder) // cwd
		return TRUE;

	DWORD dwAttrib = GetFileAttributes(szFolder);

	return (dwAttrib != 0xffffffff && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

BOOL FileMisc::FileExists(LPCTSTR szFile)
{
	DWORD dwAttrib = GetFileAttributes(szFile);

	return (dwAttrib != 0xffffffff && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) == 0);
}

BOOL FileMisc::FolderFromFilePathExists(LPCTSTR szFilePath)
{
	return FolderExists(GetFolderFromFilePath(szFilePath));
}

CString FileMisc::GetCwd()
{
	CString sFolder;

	::GetCurrentDirectory(MAX_PATH, sFolder.GetBuffer(MAX_PATH + 1));
	sFolder.ReleaseBuffer();

	return sFolder;
}

void FileMisc::SetCwd(const CString& sCwd)
{
	SetCurrentDirectory(sCwd);
}

CString FileMisc::GetFolderFromFilePath(LPCTSTR szFilePath)
{
	CString sFolder;

	// check if its a folder already
	if (FolderExists(szFilePath))
	{
		sFolder = szFilePath;
	}
	else
	{
		// remove file ending
		CString sDrive, sDir;

		SplitPath(szFilePath, &sDrive, &sDir);
		MakePath(sFolder, sDrive, sDir);
	}

	return sFolder;
}

BOOL FileMisc::CreateFolderFromFilePath(LPCTSTR szFilePath)
{
	return CreateFolder(GetFolderFromFilePath(szFilePath));
}

BOOL FileMisc::PathHasWildcard(LPCTSTR szFilePath)
{
	return (_tcschr(szFilePath, '?') || _tcschr(szFilePath, '*'));
}

BOOL FileMisc::CreateFolder(LPCTSTR szFolder)
{
	if (FolderExists(szFolder))
		return true;

	// start from the highest level folder working to the lowest
	CString sFolder, sRemaining(szFolder);
	UnterminatePath(sRemaining);

	BOOL bDone = false;
	BOOL bResult = true;

	// pull off the :\ or \\ start
	int nFind = sRemaining.Find(_T(":\\"));

	if (nFind != -1)
	{
		sFolder += sRemaining.Left(nFind + 2);
		sRemaining = sRemaining.Mid(nFind + 2);
	}
	else
	{
		nFind = sRemaining.Find(_T("\\\\"));
		
		if (nFind != -1)
		{
			sFolder += sRemaining.Left(nFind + 2);
			sRemaining = sRemaining.Mid(nFind + 2);
		}
	}

	while (!bDone && bResult)
	{
		nFind = sRemaining.Find('\\', 1);

		if (nFind == -1)
		{
			bDone = TRUE;
			sFolder += sRemaining;
		}
		else
		{
			sFolder += sRemaining.Left(nFind);
			sRemaining = sRemaining.Mid(nFind);
		}

		// if the folder doesn't exist we try to create it
		if (!FolderExists(sFolder) && (::CreateDirectory(sFolder, NULL) == FALSE))
			bResult = false;
	}

	return bResult;
}

BOOL FileMisc::FolderContainsFiles(LPCTSTR szFolder, BOOL bCheckSubFolders, LPCTSTR szFilter)
{
  CFileFind ff;
  CString sSearchSpec;

  MakePath(sSearchSpec, NULL, szFolder, szFilter, NULL);

  BOOL bContinue = ff.FindFile(sSearchSpec);

  while (bContinue)
  {
    bContinue = ff.FindNextFile();

    if (!ff.IsDots())
    {
      if (ff.IsDirectory())
      {
        if (bCheckSubFolders)
        {
          if (FolderContainsFiles(ff.GetFilePath(), TRUE, szFilter))
            return TRUE;
        }
      }
      else // is file
      {
        return TRUE;
      }
    }
  }

  return FALSE;
}

BOOL FileMisc::MoveFolder(LPCTSTR szSrcFolder, LPCTSTR szDestFolder, BOOL bIncludeSubFolders, LPCTSTR szFileMask, HANDLE hTerminate, BOOL bProcessMsgLoop)
{
	if (CopyFolder(szSrcFolder, szDestFolder, bIncludeSubFolders, szFileMask, hTerminate, bProcessMsgLoop))
	{
		// don't pass on hTerminate to ensure the operation completes
		DeleteFolderContents(szSrcFolder, bIncludeSubFolders, szFileMask, NULL, bProcessMsgLoop);

		return true;
	}

	return false;
}

BOOL FileMisc::CopyFolder(LPCTSTR szSrcFolder, LPCTSTR szDestFolder, BOOL bIncludeSubFolders, LPCTSTR szFileMask, HANDLE hTerminate, BOOL bProcessMsgLoop)
{
	if (!CreateFolder(szDestFolder))
		return false;

	if (!FolderExists(szSrcFolder))
		return false;

	// if a file mask has been specified with subfolders we need to do 2 passes on each folder, 
	// one for the files and one for the sub folders
	int nPasses = (bIncludeSubFolders && (szFileMask && lstrlen(szFileMask))) ? 2 : 1;
		
	BOOL bResult = true;
	BOOL bStopped = (WaitForSingleObject(hTerminate, 0) == WAIT_OBJECT_0);

	for (int nPass = 0; !bStopped && nPass < nPasses; nPass++)
	{
		CString sSearchSpec(szSrcFolder), sMask(szFileMask);

		if (sMask.IsEmpty() || nPass == 1) // (nPass == 1) == 2nd pass (for folders)
			sMask = _T("*.*");

		TerminatePath(sSearchSpec);
		sSearchSpec += sMask;

		WIN32_FIND_DATA finfo;
		HANDLE hSearch = NULL;

		if ((hSearch = FindFirstFile(sSearchSpec, &finfo)) != INVALID_HANDLE_VALUE) 
		{
			do 
			{
				if (bProcessMsgLoop)
					Misc::ProcessMsgLoop();

				if (finfo.cFileName[0] != '.') 
				{
					CString sSource(szSrcFolder);
					sSource += _T("\\");
					sSource += finfo.cFileName;
					
					CString sDest(szDestFolder);
					sDest += _T("\\");
					sDest += finfo.cFileName;
					
					if (finfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						if ((nPass == 1 || nPasses == 1) && bIncludeSubFolders)
							bResult = CopyFolder(sSource, sDest, hTerminate);
					}
					else if (nPass == 0) // files 
					{
						bResult = (TRUE == CopyFile(sSource, sDest, FALSE));
					}
				}

				bStopped = (WaitForSingleObject(hTerminate, 0) == WAIT_OBJECT_0);
			}
			while (!bStopped && bResult && FindNextFile(hSearch, &finfo));
			
			FindClose(hSearch);
		} 
	}

	return (!bStopped && bResult);
}

BOOL FileMisc::MoveFolder(LPCTSTR szSrcFolder, LPCTSTR szDestFolder, HANDLE hTerminate, BOOL bProcessMsgLoop)
{
	return MoveFolder(szSrcFolder, szDestFolder, TRUE, NULL, hTerminate, bProcessMsgLoop);
}

BOOL FileMisc::CopyFolder(LPCTSTR szSrcFolder, LPCTSTR szDestFolder, HANDLE hTerminate, BOOL bProcessMsgLoop)
{
	return CopyFolder(szSrcFolder, szDestFolder, TRUE, NULL, hTerminate, bProcessMsgLoop);
}

double FileMisc::GetFileSize(LPCTSTR szPath)
{
	HANDLE hFile = ::CreateFile(szPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwHighSize = 0;
		DWORD dwLowSize = ::GetFileSize(hFile, &dwHighSize);
		
		::CloseHandle(hFile);
		
		if (dwLowSize != INVALID_FILE_SIZE)
		{
			return (dwHighSize * ((double)MAXDWORD + 1) + dwLowSize);
		}
	}

	// else
	return 0;
}

void FileMisc::EnableLogging(BOOL bEnable, BOOL bResetLogFile, BOOL bWantDateTime)
{
	s_bLogging = bEnable;
	s_bLogDateTime = bWantDateTime;

	if (bEnable && bResetLogFile)
	{
		CString sLogFile = GetAppFileName();
		ReplaceExtension(sLogFile, _T(".log"));

		::DeleteFile(sLogFile);
	}
}

void FileMisc::LogTimeElapsed(DWORD& dwTickStart, LPCTSTR szFormat, ...)
{
	if (!s_bLogging)
		return;

	CString sMessage, sReason(_T("Action"));

	if (szFormat && *szFormat)
	{
		// from CString::Format
		ASSERT(AfxIsValidString(szFormat));

		va_list argList;
		va_start(argList, szFormat);
		sReason.FormatV(szFormat, argList);
		va_end(argList);
	}

	DWORD dwTickEnd = GetTickCount();
	sMessage.Format(_T("%s took %d ms"), sReason, dwTickEnd - dwTickStart);

	FileMisc::LogText(sMessage);
	dwTickStart = dwTickEnd;
}

void FileMisc::LogText(LPCTSTR szText, ...)
{
	if (!s_bLogging)
		return;

	CString sLogLine;

	if (szText && *szText)
	{
		// from CString::Format
		ASSERT(AfxIsValidString(szText));

		va_list argList;
		va_start(argList, szText);
		sLogLine.FormatV(szText, argList);
		va_end(argList);
	}

	if (s_bLogDateTime)
	{
		sLogLine += _T(" (");
		sLogLine += COleDateTime::GetCurrentTime().Format();
		sLogLine += _T(")");
	}

	CString sLogFile = GetAppFileName();
	ReplaceExtension(sLogFile, _T(".log"));

	VERIFY(AppendLineToFile(sLogFile, sLogLine));
}

BOOL FileMisc::AppendLineToFile(LPCTSTR szPathname, LPCTSTR szLine, SFE_SAVEAS nSaveAs)
{
	// make sure parent folder exists
	if (!CreateFolderFromFilePath(szPathname))
		return false;

	CStdioFileEx file;

	if (file.Open(szPathname, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeReadWrite, nSaveAs))
	{
		file.SeekToEnd();
		file.WriteString(szLine);

		if (!_tcsstr(szLine, _T("\n")))
			file.WriteString(_T("\n"));

		return true;
	}

	return false;
}

int FileMisc::LoadFileLines(LPCTSTR szPathname, CStringArray& aLines, int nLineCount)
{
	int nLinesRead = 0;
	aLines.RemoveAll();

	CStdioFileEx file;

	if (file.Open(szPathname, CFile::modeRead | CFile::shareDenyWrite))
	{
		if (file.GetLength())
		{
			if (nLineCount == -1)
				nLineCount = INT_MAX;

			// read lines
			CString sLine;
			while (file.ReadString(sLine) && nLinesRead < nLineCount)
			{
				aLines.Add(sLine);
				nLinesRead++;
			}
		}
	}

	return nLinesRead;
}

BOOL FileMisc::CanOpenFile(LPCTSTR szPathname, BOOL bDenyWrite)
{
	CFile file;
	UINT nFlags = CFile::modeRead | (bDenyWrite ? CFile::shareDenyWrite : 0);

	return (file.Open(szPathname, nFlags) != FALSE);
}

BOOL FileMisc::SaveFile(LPCTSTR szPathname, const CString& sText, SFE_SAVEAS nSaveAs)
{
	CStdioFileEx file;

	if (file.Open(szPathname, CFile::modeCreate | CFile::modeWrite, nSaveAs))
	{
		file.WriteString(sText);
		return true;
	}

	return false;
}

BOOL FileMisc::LoadFile(LPCTSTR szPathname, CString& sText)
{
	CStdioFileEx file;

	if (file.Open(szPathname, CFile::modeRead | CFile::shareDenyWrite))
	{
		return (file.ReadFile(sText) != FALSE);
	}

	return false;
}

BOOL FileMisc::IsTempFile(LPCTSTR szFilename)
{
	CString sFilename(szFilename);
	sFilename.MakeLower();

	CString sTempFolder = GetTempFolder();
	sTempFolder.MakeLower();

	return (sFilename.Find(sTempFolder) == 0);
}

CString FileMisc::GetTempFolder()
{
	TCHAR szTempPath[MAX_PATH];
	
	if (::GetTempPath(MAX_PATH, szTempPath))
		return CString(szTempPath);

	// else
	return _T("C:\\Temp");
}

CString FileMisc::GetTempFileName(LPCTSTR szPrefix, UINT uUnique)
{
	TCHAR szTempFile[MAX_PATH], szTempPath[MAX_PATH];
	
	if (::GetTempPath(MAX_PATH, szTempPath))
	{
		if (::GetTempFileName(szTempPath, szPrefix, uUnique, szTempFile))
			return szTempFile;
	}

	return "";
}

CString FileMisc::GetTempFileName(LPCTSTR szFilename, LPCTSTR szExt)
{
	CString sTempFile;
	TCHAR szTempPath[MAX_PATH];
	
	if (::GetTempPath(MAX_PATH, szTempPath))
		MakePath(sTempFile, NULL, szTempPath, szFilename, szExt);

	return sTempFile;
}

DWORD FileMisc::Run(HWND hwnd, LPCTSTR lpFile, LPCTSTR lpDirectory, int nShowCmd)
{
   CString sFile(lpFile), sParams;
   int nHash = sFile.Find('#');

   if (nHash != -1)
   {
      sParams = sFile.Mid(nHash);
      sFile = sFile.Left(nHash);

      CString sExt;
      SplitPath(sFile, NULL, NULL, NULL, &sExt);

      CString sApp = CFileRegister::GetRegisteredAppPath(sExt);

      if (!sApp.IsEmpty())
      {
         sFile = sApp;
         sParams = lpFile;
      }
      else
      {
         sFile = lpFile;
         sParams.Empty();
      }
   }

	DWORD dwRes = (DWORD)ShellExecute(hwnd, NULL, sFile, sParams, lpDirectory, nShowCmd);

	if (dwRes <= 32) // failure
	{
		// try CreateProcess
		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory( &si, sizeof(si) );
		si.cb = sizeof(si);
		si.wShowWindow = (WORD)nShowCmd;
		si.dwFlags = STARTF_USESHOWWINDOW;

		ZeroMemory( &pi, sizeof(pi) );

		// Start the child process.
		if (CreateProcess( NULL,			// No module name (use command line).
							(LPTSTR)lpFile,	// Command line.
							NULL,			// Process handle not inheritable.
							NULL,			// Thread handle not inheritable.
							FALSE,			// Set handle inheritance to FALSE.
							0,				// No creation flags.
							NULL,			// Use parent's environment block.
							lpDirectory,	// starting directory.
							&si,			// Pointer to STARTUPINFO structure.
							&pi ))			// Pointer to PROCESS_INFORMATION structure.
		{
			dwRes = 32; // success
		}

		// Close process and thread handles.
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );
	}
	return dwRes;
}

BOOL FileMisc::ExtractResource(UINT nID, LPCTSTR szType, const CString& sTempFilePath, HINSTANCE hInst)
{
	// compare time with that of module from which it was loaded
	CString sTempPath;
	CFileStatus fsRes, fsModule;
	CString sModulePath = GetModuleFileName(hInst);

	if (!CFile::GetStatus(sModulePath, fsModule))
		return FALSE;
	
	// see if the file has been created before
	if (!CFile::GetStatus(sTempFilePath, fsRes) || fsRes.m_mtime < fsModule.m_mtime)
	{
		// Load the resource into memory
		HRSRC hRes = FindResource(hInst, (LPCTSTR)nID, szType);
		
		if (!hRes) 
		{
			TRACE(_T("Couldn't find %s resource %d!\n"), szType, nID);
			return FALSE;
		}
		
		DWORD len = SizeofResource(hInst, hRes);
		
		BYTE* lpRes = (BYTE*)LoadResource(hInst, hRes);
		ASSERT(lpRes);

		CFile file;
		
		if (file.Open(sTempFilePath, CFile::modeCreate | CFile::modeWrite))
		{
			file.Write(lpRes, len);
			file.Close();
			FreeResource((HANDLE)lpRes);
		}
		else
		{
			FreeResource((HANDLE)lpRes);
			return FALSE;
		}
	}
	
	return TRUE;
}

CString FileMisc::GetModuleFileName(HMODULE hMod)
{
	static CString sModulePath;

	if (sModulePath.IsEmpty())
	{
		::GetModuleFileName(hMod, sModulePath.GetBuffer(MAX_PATH + 1), MAX_PATH);
		sModulePath.ReleaseBuffer();
	}

	return sModulePath;
}


CString FileMisc::GetModuleFolder(HMODULE hMod) 
{ 
	return GetFolderFromFilePath(GetModuleFileName(hMod)); 
}

CString FileMisc::GetAppFileName()
{
	return GetModuleFileName();
}

CString FileMisc::GetAppFolder(LPCTSTR szSubFolder)
{
	CString sFolder = GetModuleFolder();

	if (szSubFolder && *szSubFolder)
		MakePath(sFolder, NULL, sFolder, szSubFolder, NULL);

	return UnterminatePath(sFolder);
}

CString FileMisc::GetAppResourceFolder(LPCTSTR szResFolder)
{
	return GetAppFolder(szResFolder);
}

CString FileMisc::GetWindowsFolder()
{
	CString sFolder;

	::GetWindowsDirectory(sFolder.GetBuffer(MAX_PATH + 1), MAX_PATH);
	sFolder.ReleaseBuffer();

	return UnterminatePath(sFolder);
}

CString FileMisc::GetWindowsSystemFolder()
{
	CString sFolder;

	::GetSystemDirectory(sFolder.GetBuffer(MAX_PATH + 1), MAX_PATH);
	sFolder.ReleaseBuffer();

	return UnterminatePath(sFolder);
}

BOOL FileMisc::ExtractResource(LPCTSTR szModulePath, UINT nID, LPCTSTR szType, const CString& sTempFilePath)
{
	HMODULE hModule = LoadLibraryEx(szModulePath, NULL, LOAD_LIBRARY_AS_DATAFILE);

	if (!hModule)
		return false;

	// else
	return ExtractResource(nID, szType, sTempFilePath, hModule);
}

BOOL FileMisc::HasExtension(LPCTSTR szFilePath, LPCTSTR szExt)
{
	if (!szExt || !szFilePath)
		return FALSE;

	if (szExt[0] == '.')
		szExt++;

	CString sExt = GetExtension(szFilePath);

	if (sExt.GetLength() && sExt[0] == '.')
		sExt = sExt.Mid(1);

	return (sExt.CompareNoCase(szExt) == 0);
}

CString FileMisc::GetExtension(LPCTSTR szFilePath)
{
	CString sExt;
	SplitPath(szFilePath, NULL, NULL, NULL, &sExt);

	return sExt;
}

void FileMisc::SplitPath(LPCTSTR szPath, CString* pDrive, CString* pDir, CString* pFName, CString* pExt)
{
	TCHAR szDrive[_MAX_DRIVE], szFolder[_MAX_DIR], szFile[_MAX_FNAME], szExt[_MAX_EXT];

#if _MSC_VER >= 1400
	_splitpath_s(szPath, szDrive,_MAX_DRIVE, szFolder,_MAX_DIR, szFile, _MAX_FNAME, szExt, _MAX_EXT);
#else
	_tsplitpath(szPath, szDrive, szFolder, szFile, szExt);
#endif

	if (pDrive)
		*pDrive = szDrive;

	if (pDir)
		*pDir = szFolder;

	if (pFName)
		*pFName = szFile;

	if (pExt)
		*pExt = szExt;
}

CString& FileMisc::MakePath(CString& sPath, LPCTSTR szDrive, LPCTSTR szDir, LPCTSTR szFName, LPCTSTR szExt)
{
	TCHAR szPath[MAX_PATH];

#if _MSC_VER >= 1400
	_makepath_s(szPath, MAX_PATH, szDrive, szDir, szFName, szExt);
#else
	_tmakepath(szPath, szDrive, szDir, szFName, szExt);
#endif

	sPath = szPath;

	return sPath;
}

CString FileMisc::GetFullPath(const CString& sFilePath, const CString& sRelativeToFolder)
{
	if (!::PathIsRelative(sFilePath))
	{
		// special case: filename has no drive letter and is not UNC
		if (sFilePath.Find(_T(":\\")) == -1 && sFilePath.Find(_T("\\\\")) == -1)
		{
			CString sFullPath, sDrive;

			SplitPath(sRelativeToFolder, &sDrive);
			return MakePath(sFullPath, sDrive, NULL, sFilePath);
		}

		// else
		return sFilePath;
	}

	CString sSrcPath(sRelativeToFolder);
	FileMisc::TerminatePath(sSrcPath);
	sSrcPath += sFilePath;

	TCHAR szFullPath[MAX_PATH+1];
	BOOL bRes = ::PathCanonicalize(szFullPath, sSrcPath);

	return bRes ? CString(szFullPath) : sSrcPath;
}

CString& FileMisc::MakeFullPath(CString& sFilePath, const CString& sRelativeToFolder)
{
	sFilePath = GetFullPath(sFilePath, sRelativeToFolder);
	return sFilePath;
}

CString FileMisc::GetRelativePath(const CString& sFilePath, const CString& sRelativeToFolder, BOOL bFolder)
{
	if (::PathIsRelative(sFilePath))
		return sFilePath;

	TCHAR szRelPath[MAX_PATH+1];

	BOOL bRes = ::PathRelativePathTo(szRelPath, 
									UnterminatePath(sRelativeToFolder), FILE_ATTRIBUTE_DIRECTORY, 
									sFilePath, bFolder ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL);

	return bRes ? CString(szRelPath) : sFilePath;
}

CString& FileMisc::MakeRelativePath(CString& sFilePath, const CString& sRelativeToFolder, BOOL bFolder)
{
	sFilePath = GetRelativePath(sFilePath, sRelativeToFolder, bFolder);
	return sFilePath;
}

BOOL FileMisc::IsSameFile(const CString& sFilePath1, const CString& sFilePath2)
{
	CString sFullPath1 = GetFullPath(sFilePath1);
	CString sFullPath2 = GetFullPath(sFilePath2);

	return (sFilePath1.CompareNoCase(sFullPath2) == 0);
}

CString FileMisc::ResolveShortcut(LPCTSTR szShortcut)
{
	// start by checking its a valid file
	if (!FileExists(szShortcut))
		return "";

    CoInitialize(NULL);

	HRESULT hResult;
	IShellLink*	psl;
	CString sTarget(szShortcut);
	
	hResult = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
								IID_IShellLink, (LPVOID*)&psl);
	
	if (SUCCEEDED(hResult))
	{
		LPPERSISTFILE ppf;
		
		hResult = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);

		if (SUCCEEDED(hResult))
		{
			WCHAR wsz[MAX_PATH];	// buffer for Unicode string
#ifndef _UNICODE
			MultiByteToWideChar(CP_ACP, 0, szShortcut, -1, wsz, MAX_PATH);
#else
			lstrcpy(wsz, szShortcut);
#endif
			hResult = ppf->Load(wsz, STGM_READ);
			
			if (SUCCEEDED(hResult))
			{
				hResult = psl->Resolve(NULL, SLR_ANY_MATCH | SLR_NO_UI);

				if (SUCCEEDED(hResult))
				{
					TCHAR szPath[MAX_PATH];
					WIN32_FIND_DATA wfd;
					lstrcpy(szPath, szShortcut);
					hResult = psl->GetPath(szPath, MAX_PATH, &wfd, SLGP_SHORTPATH);

					if (SUCCEEDED(hResult))
						sTarget = CString(szPath);
				}
			}
		
			ppf->Release();
		}
		
		psl->Release();
	}

	CoUninitialize();
	
	return sTarget;
}

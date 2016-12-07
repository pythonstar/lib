/*
Module : HTTPDOWNLOADDLG.CPP
Purpose: Defines the implementation for an MFC dialog which performs HTTP downloads
         similiar to the Internet Explorer download dialog
Created: PJN / 14-11-1999
History: PJN / 25-01-2000 1. Fixed a problem where server authentication was not being detected correctly,
                          while proxy authentication was being handled.
                          2. Updated the way and periodicity certain UI controls are updated during the 
                          HTTP download

Copyright (c) 1999 - 2000 by PJ Naughter.  
All rights reserved.


*/


/////////////////////////////////  Includes  //////////////////////////////////
#include "stdafx.h"
#include "resource.h"
#include "HttpDownloadDlg.h"



///////////////////////////////// Defines /////////////////////////////////////
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const UINT WM_HTTPDOWNLOAD_THREAD_FINISHED = WM_APP + 1;



///////////////////////////////// Implementation //////////////////////////////
IMPLEMENT_DYNAMIC(CHttpDownloadDlg, CDialog);


CHttpDownloadDlg::CHttpDownloadDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHttpDownloadDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CHttpDownloadDlg)
	//}}AFX_DATA_INIT
  m_hInternetSession = NULL;
  m_hHttpConnection = NULL;
  m_hHttpFile = NULL;
  m_bAbort = FALSE;
  m_bSafeToClose = FALSE;
  m_pThread = NULL;
 }

void CHttpDownloadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHttpDownloadDlg)
	DDX_Control(pDX, IDC_STATUS, m_ctrlStatus);
	DDX_Control(pDX, IDC_TRANSFER_RATE, m_ctrlTransferRate);
	DDX_Control(pDX, IDC_TIMELEFT, m_ctrlTimeLeft);
	DDX_Control(pDX, IDC_PROGRESS1, m_ctrlProgress);
	DDX_Control(pDX, IDC_FILESTATUS, m_ctrlFileStatus);
	DDX_Control(pDX, IDC_ANIMATE1, m_ctrlAnimate);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CHttpDownloadDlg, CDialog)
	//{{AFX_MSG_MAP(CHttpDownloadDlg)
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
  ON_MESSAGE(WM_HTTPDOWNLOAD_THREAD_FINISHED, OnThreadFinished)
END_MESSAGE_MAP()


LRESULT CHttpDownloadDlg::OnThreadFinished(WPARAM wParam, LPARAM /*lParam*/)
{
  //It's now safe to close since the thread has signaled us
  m_bSafeToClose = TRUE;

  //Stop the animation
  m_ctrlAnimate.Stop();

  //If an error occured display the message box
  if (m_bAbort)
    EndDialog(IDCANCEL);
  else if (wParam)
  {
    AfxMessageBox(m_sError);
    EndDialog(IDCANCEL);
  }
  else
    EndDialog(IDOK);

  return 0L;
}

BOOL CHttpDownloadDlg::OnInitDialog() 
{
  //Let the parent class do its thing
	CDialog::OnInitDialog();

  //Setup the animation control
  m_ctrlAnimate.Open(IDR_HTTPDOWNLOAD_ANIMATION);

  //Validate the URL
  ASSERT(m_sURLToDownload.GetLength()); //Did you forget to specify the file to download
  if (!AfxParseURL(m_sURLToDownload, m_dwServiceType, m_sServer, m_sObject, m_nPort))
  {
    //Try sticking "http://" before it
    m_sURLToDownload = _T("http://") + m_sURLToDownload;
    if (!AfxParseURL(m_sURLToDownload, m_dwServiceType, m_sServer, m_sObject, m_nPort))
    {
      TRACE(_T("Failed to parse the URL: %s\n"), m_sURLToDownload);
      EndDialog(IDCANCEL);
      return TRUE;
    }
  }

  //Check to see if the file we are downloading to exists and if
  //it does, then ask the user if they were it overwritten
  //CFileStatus fs;
  //ASSERT(m_sFileToDownloadInto.GetLength());
  //if (CFile::GetStatus(m_sFileToDownloadInto, fs))
  //{
  //  CString sMsg;
  //  AfxFormatString1(sMsg, IDS_HTTPDOWNLOAD_OK_TO_OVERWRITE, m_sFileToDownloadInto);
  //  if (AfxMessageBox(sMsg, MB_YESNO) != IDYES)
  //  {
  //    TRACE(_T("Failed to confirm file overwrite, download aborted\n"));
  //    EndDialog(IDCANCEL);
  //    return TRUE;
  //  }
  //}

  //Try and open the file we will download into
  if (!m_FileToWrite.Open(m_sFileToDownloadInto, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite))
  {
    TRACE(_T("Failed to open the file to download into, Error:%d\n"), GetLastError());
    CString sError;
    sError.Format(_T("%d"), ::GetLastError());
    CString sMsg;
    AfxFormatString1(sMsg, IDS_HTTPDOWNLOAD_FAIL_FILE_OPEN, sError);
    AfxMessageBox(sMsg);
    EndDialog(IDCANCEL);
    return TRUE;
  }

  //Pull out just the filename component
  int nSlash = m_sObject.ReverseFind(_T('/'));
  if (nSlash == -1)
    nSlash = m_sObject.ReverseFind(_T('\\'));
  if (nSlash != -1 && m_sObject.GetLength() > 1)
    m_sFilename = m_sObject.Right(m_sObject.GetLength() - nSlash - 1);
  else
    m_sFilename = m_sObject;

	//Set the file status text
  CString sFileStatus;
  ASSERT(m_sObject.GetLength());
  ASSERT(m_sServer.GetLength());
  AfxFormatString2(sFileStatus, IDS_HTTPDOWNLOAD_FILESTATUS, m_sFilename, m_sServer);
  m_ctrlFileStatus.SetWindowText(sFileStatus);

  //Spin off the background thread which will do the actual downloading
  m_pThread = AfxBeginThread(_DownloadThread, this, THREAD_PRIORITY_NORMAL, CREATE_SUSPENDED);
  if (m_pThread == NULL)
  {
    TRACE(_T("Failed to create download thread, dialog is aborting\n"));
    EndDialog(IDCANCEL);
    return TRUE;
  }
  m_pThread->m_bAutoDelete = FALSE;
  m_pThread->ResumeThread();
	
	return TRUE;
}

UINT CHttpDownloadDlg::_DownloadThread(LPVOID pParam)
{
  //Convert from the SDK world to the C++ world
  CHttpDownloadDlg* pDlg = (CHttpDownloadDlg*) pParam;
  ASSERT(pDlg);
  ASSERT(pDlg->IsKindOf(RUNTIME_CLASS(CHttpDownloadDlg)));
  pDlg->DownloadThread();
  return 0;
}

void CHttpDownloadDlg::SetPercentage(int nPercentage)
{
  //Change the progress control
  m_ctrlProgress.SetPos(nPercentage);

  //Change the caption text
  CString sPercentage;
  sPercentage.Format(_T("%d"), nPercentage);
  CString sCaption;
  AfxFormatString2(sCaption, IDS_HTTPDOWNLOAD_PERCENTAGE, sPercentage, m_sFilename);
  SetWindowText(sCaption);
}

void CHttpDownloadDlg::SetProgressRange(DWORD dwFileSize)
{
  m_ctrlProgress.SetRange(0, (short)((dwFileSize+512)/1024));
}

void CHttpDownloadDlg::SetProgress(DWORD dwBytesRead)
{
  m_ctrlProgress.SetPos(dwBytesRead/1024);
}

void CHttpDownloadDlg::SetTimeLeft(DWORD dwSecondsLeft, DWORD dwBytesRead, DWORD dwFileSize)
{
  CString sCopied;
  if (dwBytesRead < 1024)
  {
    CString sBytes;
    sBytes.Format(_T("%d"), dwBytesRead);
    AfxFormatString1(sCopied, IDS_HTTPDOWNLOAD_BYTES, sBytes);
  }
  else if (dwBytesRead < 1048576)
  {
    CString sKiloBytes;
    sKiloBytes.Format(_T("%0.1f"), dwBytesRead/1024.0);
    AfxFormatString1(sCopied, IDS_HTTPDOWNLOAD_KILOBYTES, sKiloBytes);
  }
  else
  {
    CString sMegaBytes;
    sMegaBytes.Format(_T("%0.2f"), dwBytesRead/1048576.0);
    AfxFormatString1(sCopied, IDS_HTTPDOWNLOAD_MEGABYTES, sMegaBytes);
  }

  CString sTotal;
  if (dwFileSize < 1024)
  {
    CString sBytes;
    sBytes.Format(_T("%d"), dwFileSize);
    AfxFormatString1(sTotal, IDS_HTTPDOWNLOAD_BYTES, sBytes);
  }
  else if (dwFileSize < 1048576)
  {
    CString sKiloBytes;
    sKiloBytes.Format(_T("%0.1f"), dwFileSize/1024.0);
    AfxFormatString1(sTotal, IDS_HTTPDOWNLOAD_KILOBYTES, sKiloBytes);
  }
  else
  {
    CString sMegaBytes;
    sMegaBytes.Format(_T("%0.2f"), dwFileSize/1048576.0);
    AfxFormatString1(sTotal, IDS_HTTPDOWNLOAD_MEGABYTES, sMegaBytes);
  }

  CString sOf;
  AfxFormatString2(sOf, IDS_HTTPDOWNLOAD_OF, sCopied, sTotal);

  CString sTime;
  if (dwSecondsLeft < 60)
  {
    CString sSeconds;
    sSeconds.Format(_T("%d"), dwSecondsLeft);
    AfxFormatString1(sTime, IDS_HTTPDOWNLOAD_SECONDS, sSeconds);
  }
  else
  {
    DWORD dwMinutes = dwSecondsLeft / 60;
    DWORD dwSeconds = dwSecondsLeft % 60;
    CString sSeconds;
    sSeconds.Format(_T("%d"), dwSeconds);
    CString sMinutes;
    sMinutes.Format(_T("%d"), dwMinutes);
    if (dwSeconds == 0)
      AfxFormatString1(sTime, IDS_HTTPDOWNLOAD_MINUTES, sMinutes);
    else
      AfxFormatString2(sTime, IDS_HTTPDOWNLOAD_MINUTES_AND_SECONDS, sMinutes, sSeconds);
  }

  CString sTimeLeft;
  AfxFormatString2(sTimeLeft, IDS_HTTPDOWNLOAD_TIMELEFT, sTime, sOf);
  m_ctrlTimeLeft.SetWindowText(sTimeLeft);
}

void CHttpDownloadDlg::SetStatus(const CString& sCaption)
{
  m_ctrlStatus.SetWindowText(sCaption);
}

void CHttpDownloadDlg::SetStatus(UINT nID)
{
  CString sCaption;
  sCaption.LoadString(nID);
  SetStatus(sCaption);
}

void CHttpDownloadDlg::SetStatus(UINT nID, const CString& lpsz1)
{
  CString sStatus;
  AfxFormatString1(sStatus, nID, lpsz1);
  SetStatus(sStatus);
}

void CHttpDownloadDlg::SetTransferRate(double KbPerSecond)
{
  CString sRate;
  if (KbPerSecond < 1)
  {
    CString sBytesPerSecond;
    sBytesPerSecond.Format(_T("%0.0f"), KbPerSecond*1024);
    AfxFormatString1(sRate, IDS_HTTPDOWNLOAD_BYTESPERSECOND, sBytesPerSecond);
  }
  else if (KbPerSecond < 10)
  {
    CString sKiloBytesPerSecond;
    sKiloBytesPerSecond.Format(_T("%0.2f"), KbPerSecond);
    AfxFormatString1(sRate, IDS_HTTPDOWNLOAD_KILOBYTESPERSECOND, sKiloBytesPerSecond);
  }
  else
  {
    CString sKiloBytesPerSecond;
    sKiloBytesPerSecond.Format(_T("%0.0f"), KbPerSecond);
    AfxFormatString1(sRate, IDS_HTTPDOWNLOAD_KILOBYTESPERSECOND, sKiloBytesPerSecond);
  }
  m_ctrlTransferRate.SetWindowText(sRate);
}

void CHttpDownloadDlg::PlayAnimation()
{
  m_ctrlAnimate.Play(0, (UINT)-1, (UINT)-1);
}

void CHttpDownloadDlg::HandleThreadErrorWithLastError(UINT nIDError, DWORD dwLastError)
{
  //Form the error string to report
  CString sError;
  if (dwLastError)
    sError.Format(_T("%d"), dwLastError);
  else
    sError.Format(_T("%d"), ::GetLastError());
  AfxFormatString1(m_sError, nIDError, sError);

  //Delete the file being downloaded to if it is present
  m_FileToWrite.Close();
  ::DeleteFile(m_sFileToDownloadInto);

  PostMessage(WM_HTTPDOWNLOAD_THREAD_FINISHED, 1);
}

void CHttpDownloadDlg::HandleThreadError(UINT nIDError)
{
  m_sError.LoadString(nIDError);
  PostMessage(WM_HTTPDOWNLOAD_THREAD_FINISHED, 1);
}

void CHttpDownloadDlg::DownloadThread()
{
  //Create the Internet session handle
  ASSERT(m_hInternetSession == NULL);
  m_hInternetSession = ::InternetOpen(AfxGetAppName(), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
  if (m_hInternetSession == NULL)
  {
    TRACE(_T("Failed in call to InternetOpen, Error:%d\n"), ::GetLastError());
    HandleThreadErrorWithLastError(IDS_HTTPDOWNLOAD_GENERIC_ERROR);
    return;
  }

  //Should we exit the thread
  if (m_bAbort)
  {
    PostMessage(WM_HTTPDOWNLOAD_THREAD_FINISHED);
    return;
  }  

  //Setup the status callback function
  if (::InternetSetStatusCallback(m_hInternetSession, _OnStatusCallBack) == INTERNET_INVALID_STATUS_CALLBACK)
  {
    TRACE(_T("Failed in call to InternetSetStatusCallback, Error:%d\n"), ::GetLastError());
    HandleThreadErrorWithLastError(IDS_HTTPDOWNLOAD_GENERIC_ERROR);
    return;
  }

  //Should we exit the thread
  if (m_bAbort)
  {
    PostMessage(WM_HTTPDOWNLOAD_THREAD_FINISHED);
    return;
  }  

  //Make the connection to the HTTP server          
  ASSERT(m_hHttpConnection == NULL);
  if (m_sUserName.GetLength())
    m_hHttpConnection = ::InternetConnect(m_hInternetSession, m_sServer, m_nPort, m_sUserName, 
                                          m_sPassword, m_dwServiceType, 0, (DWORD) this);
  else
    m_hHttpConnection = ::InternetConnect(m_hInternetSession, m_sServer, m_nPort, NULL, 
                                          NULL, m_dwServiceType, 0, (DWORD) this);
  if (m_hHttpConnection == NULL)
  {
    TRACE(_T("Failed in call to InternetConnect, Error:%d\n"), ::GetLastError());
    HandleThreadErrorWithLastError(IDS_HTTPDOWNLOAD_FAIL_CONNECT_SERVER);
    return;
  }

  //Should we exit the thread
  if (m_bAbort)
  {
    PostMessage(WM_HTTPDOWNLOAD_THREAD_FINISHED);
    return;
  }  

  //Start the animation to signify that the download is taking place
  PlayAnimation();

  //Issue the request to read the file
  LPCTSTR ppszAcceptTypes[2];
  ppszAcceptTypes[0] = _T("*/*");  //We support accepting any mime file type since this is a simple download of a file
  ppszAcceptTypes[1] = NULL;
  ASSERT(m_hHttpFile == NULL);
  m_hHttpFile = HttpOpenRequest(m_hHttpConnection, NULL, m_sObject, NULL, NULL, ppszAcceptTypes, INTERNET_FLAG_RELOAD | 
                                INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_KEEP_CONNECTION, (DWORD) this);
  if (m_hHttpFile == NULL)
  {
    TRACE(_T("Failed in call to HttpOpenRequest, Error:%d\n"), ::GetLastError());
    HandleThreadErrorWithLastError(IDS_HTTPDOWNLOAD_FAIL_CONNECT_SERVER);
    return;
  }

  //Should we exit the thread
  if (m_bAbort)
  {
    PostMessage(WM_HTTPDOWNLOAD_THREAD_FINISHED);
    return;
  }  

//label used to jump to if we need to resend the request
resend:

  //Issue the request
  BOOL bSend = ::HttpSendRequest(m_hHttpFile, NULL, 0, NULL, 0);
  if (!bSend)
  {
    TRACE(_T("Failed in call to HttpSendRequest, Error:%d\n"), ::GetLastError());
    HandleThreadErrorWithLastError(IDS_HTTPDOWNLOAD_FAIL_CONNECT_SERVER);
    return;
  }

  //Check the HTTP status code
	TCHAR szStatusCode[32];
	DWORD dwInfoSize = 32;
	if (!HttpQueryInfo(m_hHttpFile, HTTP_QUERY_STATUS_CODE, szStatusCode, &dwInfoSize, NULL))
  {
    TRACE(_T("Failed in call to HttpQueryInfo for HTTP query status code, Error:%d\n"), ::GetLastError());
    HandleThreadError(IDS_HTTPDOWNLOAD_INVALID_SERVER_RESPONSE);
    return;
  }
  else
  {
    long nStatusCode = _ttol(szStatusCode);

    //Handle any authentication errors
    if (nStatusCode == HTTP_STATUS_PROXY_AUTH_REQ || nStatusCode == HTTP_STATUS_DENIED)
    {
      // We have to read all outstanding data on the Internet handle
      // before we can resubmit request. Just discard the data.
      char szData[51];
      DWORD dwSize;
      do
	    {
		    ::InternetReadFile(m_hHttpFile, (LPVOID)szData, 50, &dwSize);
	    }
	    while (dwSize != 0);

      //Bring up the standard authentication dialog
      if (::InternetErrorDlg(GetSafeHwnd(), m_hHttpFile, ERROR_INTERNET_INCORRECT_PASSWORD, FLAGS_ERROR_UI_FILTER_FOR_ERRORS |
                             FLAGS_ERROR_UI_FLAGS_GENERATE_DATA | FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS, NULL) == ERROR_INTERNET_FORCE_RETRY)
        goto resend;
    }
  	else if (nStatusCode != HTTP_STATUS_OK)
    {
      TRACE(_T("Failed to retrieve a HTTP 200 status, Status Code:%d\n"), nStatusCode);
      HandleThreadErrorWithLastError(IDS_HTTPDOWNLOAD_INVALID_HTTP_RESPONSE, nStatusCode);
      return;
    }
  }

  //Update the status control to reflect that we are getting the file information
  SetStatus(IDS_HTTPDOWNLOAD_GETTING_FILE_INFORMATION);

  // Get the length of the file.            
  TCHAR szContentLength[32];
  dwInfoSize = 32;
  DWORD dwFileSize = 0;
  BOOL bGotFileSize = FALSE;
  if (::HttpQueryInfo(m_hHttpFile, HTTP_QUERY_CONTENT_LENGTH, szContentLength, &dwInfoSize, NULL))
  {
    //Set the progress control range
    bGotFileSize = TRUE;
    dwFileSize = (DWORD) _ttol(szContentLength);
    SetProgressRange(dwFileSize);
  }

  //Update the status to say that we are now downloading the file
  SetStatus(IDS_HTTPDOWNLOAD_RETREIVEING_FILE);

  //Now do the actual read of the file
  DWORD dwStartTicks = ::GetTickCount();
  DWORD dwCurrentTicks = dwStartTicks;
  DWORD dwBytesRead = 0;
  char szReadBuf[1024];
  DWORD dwBytesToRead = 1024;
  DWORD dwTotalBytesRead = 0;
  DWORD dwLastTotalBytes = 0;
  DWORD dwLastPercentage = 0;
  do
  {
    if (!::InternetReadFile(m_hHttpFile, szReadBuf, dwBytesToRead, &dwBytesRead))
    {
      TRACE(_T("Failed in call to InternetReadFile, Error:%d\n"), ::GetLastError());
      HandleThreadErrorWithLastError(IDS_HTTPDOWNLOAD_ERROR_READFILE);
      return;
    }
    else if (dwBytesRead && !m_bAbort)
    {
      //Write the data to file
      TRY
      {
        m_FileToWrite.Write(szReadBuf, dwBytesRead);
      }
      CATCH(CFileException, e);                                          
      {
        TRACE(_T("An exception occured while writing to the download file\n"));
        HandleThreadErrorWithLastError(IDS_HTTPDOWNLOAD_ERROR_READFILE, e->m_lOsError);
        e->Delete();
        return;
      }
      END_CATCH

      //Increment the total number of bytes read
      dwTotalBytesRead += dwBytesRead;  

      UpdateControlsDuringTransfer(dwStartTicks, dwCurrentTicks, dwTotalBytesRead, dwLastTotalBytes, 
                                     dwLastPercentage, bGotFileSize, dwFileSize);
    }
  } 
  while (dwBytesRead && !m_bAbort);

  //Delete the file being downloaded to if it is present and the download was aborted
  m_FileToWrite.Close();
  if (m_bAbort)
    ::DeleteFile(m_sFileToDownloadInto);

  //We're finished
  PostMessage(WM_HTTPDOWNLOAD_THREAD_FINISHED);
}

void CHttpDownloadDlg::UpdateControlsDuringTransfer(DWORD dwStartTicks, DWORD& dwCurrentTicks, DWORD dwTotalBytesRead, DWORD& dwLastTotalBytes, 
                                                    DWORD& dwLastPercentage, BOOL bGotFileSize, DWORD dwFileSize)
{
  if (bGotFileSize)
  {
    //Update the percentage downloaded in the caption
    DWORD dwPercentage = (DWORD) (dwTotalBytesRead * 100.0 / dwFileSize);
    if (dwPercentage != dwLastPercentage)
    {
      SetPercentage(dwPercentage);
      dwLastPercentage = dwPercentage;

      //Update the progress control bar
      SetProgress(dwTotalBytesRead);
    }
  }

  //Update the transfer rate amd estimated time left every second
  DWORD dwNowTicks = GetTickCount();
  DWORD dwTimeTaken = dwNowTicks - dwCurrentTicks;
  if (dwTimeTaken > 1000)
  {
    double KbPerSecond = ((double)(dwTotalBytesRead) - (double)(dwLastTotalBytes)) / ((double)(dwTimeTaken));
    SetTransferRate(KbPerSecond);

    //Setup for the next time around the loop
    dwCurrentTicks = dwNowTicks;
    dwLastTotalBytes = dwTotalBytesRead;

    if (bGotFileSize)
    {
      //Update the estimated time left
      if (dwTotalBytesRead)
      {
        DWORD dwSecondsLeft = (DWORD) (((double)dwNowTicks - dwStartTicks) / dwTotalBytesRead * 
                                       (dwFileSize - dwTotalBytesRead) / 1000);
        SetTimeLeft(dwSecondsLeft, dwTotalBytesRead, dwFileSize);
      }
    }
  }
}

void CALLBACK CHttpDownloadDlg::_OnStatusCallBack(HINTERNET hInternet, DWORD dwContext, DWORD dwInternetStatus, 
                                                  LPVOID lpvStatusInformation, DWORD dwStatusInformationLength)
{
  //Convert from the SDK C world to the C++ world
  CHttpDownloadDlg* pDlg = (CHttpDownloadDlg*) dwContext;
  ASSERT(pDlg);
  ASSERT(pDlg->IsKindOf(RUNTIME_CLASS(CHttpDownloadDlg)));
  pDlg->OnStatusCallBack(hInternet, dwInternetStatus, lpvStatusInformation, dwStatusInformationLength);
}

void CHttpDownloadDlg::OnStatusCallBack(HINTERNET /*hInternet*/, DWORD dwInternetStatus, 
                                         LPVOID lpvStatusInformation, DWORD /*dwStatusInformationLength*/)
{
  switch (dwInternetStatus)
  {
    case INTERNET_STATUS_RESOLVING_NAME:
    {
      SetStatus(IDS_HTTPDOWNLOAD_RESOLVING_NAME, (LPCTSTR) lpvStatusInformation);
      break;
    }
    case INTERNET_STATUS_NAME_RESOLVED:
    {
      SetStatus(IDS_HTTPDOWNLOAD_RESOLVED_NAME, (LPCTSTR) lpvStatusInformation);
      break;
    }
    case INTERNET_STATUS_CONNECTING_TO_SERVER:
    {
      SetStatus(IDS_HTTPDOWNLOAD_CONNECTING, (LPCTSTR) lpvStatusInformation);
      break;
    }
    case INTERNET_STATUS_CONNECTED_TO_SERVER:
    {
      SetStatus(IDS_HTTPDOWNLOAD_CONNECTED, (LPCTSTR) lpvStatusInformation);
      break;
    }
    case INTERNET_STATUS_REDIRECT:
    {
      SetStatus(IDS_HTTPDOWNLOAD_REDIRECTING, (LPCTSTR) lpvStatusInformation);
      break;
    }
    default:
    {
      break;
    }
  }
}

void CHttpDownloadDlg::OnDestroy() 
{
  //Wait for the worker thread to exit
  if (m_pThread)
  {
    WaitForSingleObject(m_pThread->m_hThread, INFINITE);
    delete m_pThread;
    m_pThread = NULL;
  }

  //Free up the internet handles we may be using
  if (m_hHttpFile)
  {
    ::InternetCloseHandle(m_hHttpFile);
    m_hHttpFile = NULL;
  }
  if (m_hHttpConnection)
  {
    ::InternetCloseHandle(m_hHttpConnection);
    m_hHttpConnection = NULL;
  }
  if (m_hInternetSession)
  {
    ::InternetCloseHandle(m_hInternetSession);
    m_hInternetSession = NULL;
  }

  //Let the parent class do its thing
	CDialog::OnDestroy();
}

void CHttpDownloadDlg::OnCancel() 
{
  //Just set the abort flag to TRUE and
  //disable the cancel button
  m_bAbort = TRUE;	
  GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
  SetStatus(IDS_HTTPDOWNLOAD_ABORTING_TRANSFER);
}

void CHttpDownloadDlg::OnClose() 
{
  if (m_bSafeToClose)	
	  CDialog::OnClose();
  else
  {
    //Just set the abort flag to TRUE and
    //disable the cancel button
    m_bAbort = TRUE;	
    GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
    SetStatus(IDS_HTTPDOWNLOAD_ABORTING_TRANSFER);
  }
}

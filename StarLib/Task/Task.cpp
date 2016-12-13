
#include "stdafx.h"
#include "Task.h"
#include   <initguid.h>
#include   <ole2.h>
#include   <mstask.h>
#include <WinUser.h>

BOOL CTask::CreateAutoRunTask(LPCTSTR lpszTaskName, LPCTSTR lpszAppName,
					   LPCTSTR lpszParameters, LPCTSTR lpszWorkingDirectory)
{
	BOOL bSuccess=FALSE;

	HRESULT hr=ERROR_SUCCESS; 
	ITaskScheduler *pITS=NULL; 

	///////////////////////////////////////////////////////////////// 
	//   Call   CoInitialize   to   initialize   the   COM   library   and   then   
	//   CoCreateInstance   to   get   the   Task   Scheduler   object.   
	///////////////////////////////////////////////////////////////// 
	hr=CoInitialize(NULL); 
	if ( SUCCEEDED(hr) ){ 
		hr=CoCreateInstance( CLSID_CTaskScheduler, NULL, CLSCTX_INPROC_SERVER, 
			IID_ITaskScheduler, (void**)&pITS ); 
		if ( FAILED(hr) ){ 
			CoUninitialize(); 
			return bSuccess; 
		} 
	}else{ 
		return bSuccess; 
	} 

	///////////////////////////////////////////////////////////////// 
	//   Call   ITaskScheduler::NewWorkItem   to   create   new   task. 
	///////////////////////////////////////////////////////////////// 
	CStringW strTaskNameW;
	strTaskNameW=lpszTaskName;
	ITask *pITask; 
	IPersistFile *pIPersistFile; 

	//先删除后创建
	pITS->Delete((LPCWSTR)strTaskNameW);

	hr=pITS->NewWorkItem( (LPCWSTR)strTaskNameW, 
		CLSID_CTask, IID_ITask, (IUnknown**)&pITask );

	pITS->Release(); //   Release   object 
	if ( FAILED(hr) ){ 
		CoUninitialize(); 
		TRACE( "Failed   calling   NewWorkItem,   error   =   0x%x\n", hr ); 
		return bSuccess; 
	} 

	////////////////////////////////////////////////////////////////// 
	//   set   task   application   name 
	// 
	///////////////////////////////////////////////////////////////// 
	CStringW strAppNameW;
	strAppNameW=lpszAppName;
	hr=pITask->SetApplicationName((LPCWSTR)strAppNameW); 
	if ( FAILED(hr) ){ 
		CoUninitialize(); 
		TRACE( "Failed   calling   SetApplicationName,   error   =   0x%x\n", hr ); 
		return bSuccess; 
	} 

	////////////////////////////////////////////////////////////////// 
	//Sets   the   command-line   parameters   for   the   task 
	///////////////////////////////////////////////////////////////// 
	CStringW strParameters;
	strParameters=lpszParameters;
	hr=pITask->SetParameters((LPCWSTR)strParameters); 
	if ( FAILED(hr) ){ 
		CoUninitialize(); 
		TRACE( "Failed   calling   SetParameters,   error   =   0x%x\n", hr ); 
		return bSuccess; 
	} 

	////////////////////////////////////////////////////////////////// 
	//Sets   the   working   directory   for   the   task 
	// 
	///////////////////////////////////////////////////////////////// 
	CStringW strWorkingDirectoryW;
	strWorkingDirectoryW=lpszWorkingDirectory;
	hr=pITask->SetWorkingDirectory((LPCWSTR)strWorkingDirectoryW); 
	if ( FAILED(hr) ){ 
		CoUninitialize(); 
		TRACE( "Failed   calling   SetWorkingDirectory,   error   =   0x%x\n", hr ); 
		return bSuccess; 
	} 

	///////////////////////////////////////////////////////////////// 
	//   Call   IUnknown::QueryInterface   to   get   a   pointer   to   
	//   IPersistFile   and   IPersistFile::Save   to   save   
	//   the   new   task   to   disk. 
	///////////////////////////////////////////////////////////////// 

	hr=pITask->QueryInterface(IID_IPersistFile,(void**)&pIPersistFile); 
	if ( FAILED(hr) ){ 
		CoUninitialize(); 
		TRACE( "Failed   calling   QueryInterface,   error   =   0x%x\n", hr ); 
		return bSuccess; 
	} 


	/////////////////////////////////////////////////////////////// 
	//下面这段代码需要屏蔽掉，否则会以“NT AUTHORITY\SYSTEM”身份
	//创建任务，导致启动的程序没有界面。
	///////////////////////////////////////////////////////////////
	WCHAR szUser[MAX_PATH];
	DWORD nSize=MAX_PATH;
	GetUserNameW(szUser,&nSize);
	hr=pITask->SetAccountInformation(szUser,   NULL); 
	if ( FAILED(hr) ){ 
		CoUninitialize(); 
		TRACE( "Failed   calling   SetAccountInformation,   error   =   0x%x\n",hr); 
		return bSuccess; 
	} 

	/////////////////////////////////////////////////////////////////// 
	//   Set   accountinformation 
	//////////////////////////////////////////////////////////////////// 
	pITask->SetFlags(TASK_FLAG_INTERACTIVE|TASK_FLAG_RUN_ONLY_IF_LOGGED_ON);

	/////////////////////////////////////////////////////////////// 
	//   Save 
	////////////////////////////////////////////////////////////// 
	hr=pIPersistFile->Save(NULL,   TRUE); 
	// pIPersistFile->Release(); 
	if ( FAILED(hr) ){ 
		if ( hr==SCHED_E_SERVICE_NOT_RUNNING ){//任务计划程序服务没有运行
			::MessageBox(NULL,
				_T("任务计划程序服务没有运行，请按以下方法解决后重试(任选一种)：\n\n")
				_T("1.我的电脑->右键->管理->服务和应用程序->服务->Task Scheduler->启用该服务\n")
				_T("2.在“运行”对话框中敲入 net start Schedule ")
				,NULL,MB_OK);
		}
		CoUninitialize();
		TRACE( "Failed   calling   Save,   error   =   0x%x\n",hr); 
		return bSuccess; 
	} 

	////////////////////////////////////////////////////////////////// 
	//Create   task   trigger   object 
	// 
	///////////////////////////////////////////////////////////////// 
	ITaskTrigger *pITaskTrigger; 
	WORD piNewTrigger=0; 
	hr=pITask->CreateTrigger(&piNewTrigger,   &pITaskTrigger); 
	if ( FAILED(hr) ){ 
		CoUninitialize(); 
		TRACE( "Failed   create   tasktrigger   object,   error   =   0x%x\n",hr); 
		return bSuccess; 
	} 

	/////////////////////////////////////////////////////////////// 
	//   Save 
	////////////////////////////////////////////////////////////// 
	hr=pIPersistFile->Save(NULL,   TRUE); 
	// pIPersistFile->Release(); 
	if ( FAILED(hr) ){ 
		CoUninitialize(); 
		TRACE( "Failed   calling   Save,   error   =   0x%x\n",hr); 
		return bSuccess; 
	} 


	////////////////////////////////////////////////////////////////// 
	//Set   task   trigger 
	// 
	///////////////////////////////////////////////////////////////// 
	TASK_TRIGGER   pTrigger; 
	TRIGGER_TYPE_UNION   triggertype; 
	triggertype.Daily.DaysInterval   =   1; 
	ZeroMemory(&pTrigger,   sizeof   (TASK_TRIGGER)); 
	pTrigger.wStartHour   =   14; 
	pTrigger.wStartMinute   =   40; 
	pTrigger.wBeginDay   =12; 
	pTrigger.wBeginMonth   =11; 
	pTrigger.wBeginYear   =2004; 
	pTrigger.cbTriggerSize=sizeof(TASK_TRIGGER); 
	pTrigger.TriggerType=TASK_EVENT_TRIGGER_AT_SYSTEMSTART; 
	pTrigger.Type=triggertype; 
	hr=pITaskTrigger->SetTrigger(&pTrigger); 
	pITaskTrigger->Release(); 
	if ( FAILED(hr) ){ 
		CoUninitialize(); 
		TRACE( "Failed   create   tasktrigger   object,   error   =   0x%x\n",hr); 
		return bSuccess; 
	} 

	/////////////////////////////////////////////////////////////// 
	//   Save 
	////////////////////////////////////////////////////////////// 
	hr=pIPersistFile->Save(NULL,TRUE); 
	pIPersistFile->Release(); 
	if ( FAILED(hr) ){ 
		CoUninitialize(); 
		TRACE( "Failed   calling   Save,   error   =   0x%x\n",hr); 
		return bSuccess; 
	} 

	///////////////////////////////////////////////////////////////// 
	//   Release   all   resources. 
	///////////////////////////////////////////////////////////////// 

	hr   =   pITask->Release(); 
	if ( FAILED(hr) ){ 
		CoUninitialize(); 
		TRACE( "Failed   calling   Release,   error   =   0x%x\n",hr); 
		return bSuccess; 
	} 

	TRACE("Created task.\n"); 
	return bSuccess;
}

BOOL CTask::DeleteTask(LPCTSTR lpszTaskName)
{
	BOOL bSuccess=FALSE;

	HRESULT hr=ERROR_SUCCESS; 
	ITaskScheduler *pITS=NULL; 

	///////////////////////////////////////////////////////////////// 
	//   Call   CoInitialize   to   initialize   the   COM   library   and   then   
	//   CoCreateInstance   to   get   the   Task   Scheduler   object.   
	///////////////////////////////////////////////////////////////// 
	hr=CoInitialize(NULL); 
	if ( SUCCEEDED(hr) ){ 
		hr=CoCreateInstance( CLSID_CTaskScheduler, NULL, CLSCTX_INPROC_SERVER, 
			IID_ITaskScheduler, (void**)&pITS ); 
		if ( FAILED(hr) ){ 
			CoUninitialize(); 
			return bSuccess; 
		} 
	}else{ 
		return bSuccess; 
	} 

	///////////////////////////////////////////////////////////////// 
	//   Call   ITaskScheduler::NewWorkItem   to   create   new   task. 
	///////////////////////////////////////////////////////////////// 
	CStringW strTaskNameW;
	strTaskNameW=lpszTaskName;

	//先删除后创建
	pITS->Delete((LPCWSTR)strTaskNameW);
	pITS->Release(); //   Release   object 
	bSuccess=TRUE;

	return bSuccess;
}
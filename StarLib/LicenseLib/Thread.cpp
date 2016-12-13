#include <windows.h>
#include "thread.h"
#include <process.h>
#include <assert.h>
#include <tchar.h>

#ifndef _DEBUG 
	#define MY_TRY_BEGIN	try {
	#define MY_CATCH_ALL	} catch (...) {
	#define MY_CATCH_END	}
#else 
	#define MY_TRY_BEGIN	{{
	#define MY_CATCH_ALL	} if (0) {
	#define MY_CATCH_END	}}
#endif

CThread::CThread(bool self_delete) 
	: thread_handle_(INVALID_HANDLE_VALUE)
	, thread_id_(0)
	, suspended_(false)
	, terminated_(false)
	, self_delete_(self_delete)
{
	memset(thread_info_, 0, sizeof(thread_info_));
}

CThread::~CThread()
{
	/* ����ʱҪֹͣ�߳� */
	HANDLE h = (HANDLE)::InterlockedExchange((PLONG)&thread_handle_, (LONG)INVALID_HANDLE_VALUE);
	if (h!=INVALID_HANDLE_VALUE) {
		if (WAIT_OBJECT_0!=::WaitForSingleObject(h, 0)) {
			::TerminateThread(h, -8008);
		}
	}
	thread_id_ = 0;
}

DWORD CThread::wait_for(DWORD ms)
{
	return ::WaitForSingleObject(thread_handle_, ms);
}

bool CThread::start_thread(bool suspended)
{
	on_init();

	/* �����߳� */
	assert(thread_handle_==INVALID_HANDLE_VALUE);
	suspended = suspended;
	thread_handle_ = (HANDLE)_beginthreadex(
		NULL,
		0,
		ThreadProc,
		(void*)this,
		suspended ? CREATE_SUSPENDED : 0,
		&thread_id_
	);

	if (thread_handle_==INVALID_HANDLE_VALUE)
		return FALSE;

	on_start();

	return TRUE;
}

bool CThread::stop_thread(DWORD ms)
{
	/* ������ҧβ�� */
	assert(::GetCurrentThreadId()!=thread_id_);

	/* ����Ҫ����ֹͣ����������resume֮���̵߳õ�ʱ�䣬���ֱ�suspend�ˣ� */
	terminated_ = true;

	/* ֹ֮ͣǰ�Ȼָ�һ�� */
	resume_thread();

	/* ÿ��stopһ�μ��� */
	HANDLE h = (HANDLE)::InterlockedExchange((PLONG)&thread_handle_, (LONG)INVALID_HANDLE_VALUE);
	if (h==INVALID_HANDLE_VALUE) {
//		LOG__("[message] close thread but handle invalid.");
		return false;
	}

	/* ע�����������stop�Ļ����Զ�ȡ��self_deleteģʽ�������ڴ��ͻ */
	self_delete_ = false;		

	if (WAIT_OBJECT_0!=::WaitForSingleObject(h, ms)) {
		/* �߳�û�н�������ô�͵ȴ� */
		on_stop(true);
		::TerminateThread(h, -8888);
	} else {
		on_stop(false);
	}

	/* ���������� */
	thread_id_ = 0;
	memset(thread_info_, 0, sizeof(thread_info_));

	CloseHandle(h);
	return true;
}

DWORD CThread::suspend_thread()
{
	return SuspendThread(thread_handle_);
}

DWORD CThread::resume_thread(bool all)
{
	/* ע���̻߳�����Щ����Ҫ�������� */
	DWORD ticks = -1;
	for (;;) {
		ticks = ResumeThread(thread_handle_);
		if (ticks==-1)
			break;

		if (!all)
			break;

		if (ticks>0)
			continue;
		else 
			break;
	}
	return ticks;
}

bool CThread::is_valid()
{
	return (thread_handle_==INVALID_HANDLE_VALUE) ? FALSE : TRUE;
}


/* �õ��߳̾�� */
HANDLE CThread::get_handle()
{
	return thread_handle_;
}

/* �õ��̱߳�� */
DWORD CThread::get_threadid()
{
	return thread_id_;
}


void CThread::set_threadinfo(LPCTSTR info)
{
	_tcsncpy(thread_info_, info, sizeof(thread_info_)-1);
}

LPCTSTR CThread::get_threadinfo()
{
	return thread_info_;
}

void CThread::set_autodelete(bool del)
{
	self_delete_ = del;
}

bool CThread::get_autodelete()
{
	return self_delete_;
}

void CThread::set_terminate()
{
	terminated_ = true;
}

UINT CThread::execute()
{
	// һ���̵߳����ⲻӦ��Ӱ���������̣���������tryס��
	int retcode = 0;

	/////////////////////////////////////////////////////
	MY_TRY_BEGIN

		retcode = on_execute();

	MY_CATCH_ALL

		retcode = 8005;
		on_exception();

	MY_CATCH_END
	/////////////////////////////////////////////////////

	/* �߳�������� */
	on_runover();

	/* ����Լ����ڴ� */
	HANDLE h = (HANDLE)::InterlockedExchange((PLONG)&thread_handle_, (LONG)INVALID_HANDLE_VALUE);
	if (h!=INVALID_HANDLE_VALUE) {
		memset(thread_info_, 0, sizeof(thread_info_));
		::CloseHandle(h);

		/* ����߳�����ɾ���ģ�ɾ���Լ� */
		if (self_delete_) {
			delete this;
		}
	}

	_endthreadex(0);
	return retcode;
}

UINT CThread::on_execute()
{
	return -8806;
}

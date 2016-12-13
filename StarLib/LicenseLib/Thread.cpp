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
	/* 结束时要停止线程 */
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

	/* 开启线程 */
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
	/* 不能蛇咬尾巴 */
	assert(::GetCurrentThreadId()!=thread_id_);

	/* 首先要进行停止操作，否则resume之后，线程得到时间，就又被suspend了！ */
	terminated_ = true;

	/* 停止之前先恢复一下 */
	resume_thread();

	/* 每次stop一次即可 */
	HANDLE h = (HANDLE)::InterlockedExchange((PLONG)&thread_handle_, (LONG)INVALID_HANDLE_VALUE);
	if (h==INVALID_HANDLE_VALUE) {
//		LOG__("[message] close thread but handle invalid.");
		return false;
	}

	/* 注意如果外界调用stop的话，自动取消self_delete模式，否则内存冲突 */
	self_delete_ = false;		

	if (WAIT_OBJECT_0!=::WaitForSingleObject(h, ms)) {
		/* 线程没有结束，那么就等待 */
		on_stop(true);
		::TerminateThread(h, -8888);
	} else {
		on_stop(false);
	}

	/* 清除相关数据 */
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
	/* 注意线程唤醒这些是需要计数器的 */
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


/* 得到线程句柄 */
HANDLE CThread::get_handle()
{
	return thread_handle_;
}

/* 得到线程编号 */
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
	// 一个线程的意外不应该影响整个进程，所以这里try住了
	int retcode = 0;

	/////////////////////////////////////////////////////
	MY_TRY_BEGIN

		retcode = on_execute();

	MY_CATCH_ALL

		retcode = 8005;
		on_exception();

	MY_CATCH_END
	/////////////////////////////////////////////////////

	/* 线程运行完毕 */
	on_runover();

	/* 清除自己的内存 */
	HANDLE h = (HANDLE)::InterlockedExchange((PLONG)&thread_handle_, (LONG)INVALID_HANDLE_VALUE);
	if (h!=INVALID_HANDLE_VALUE) {
		memset(thread_info_, 0, sizeof(thread_info_));
		::CloseHandle(h);

		/* 如果线程是自删除的，删除自己 */
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

#pragma once

//
// 封装了windows线程，使用的时候请派生，并不同于MFC的CWinThread
// 注意delete一个线程指针的时候，最慢需要1秒钟。所以需要注意用户等待问题
// 
// liusiyang
//

class CThread
{
protected:
	HANDLE thread_handle_;			// 线程句柄
	unsigned int thread_id_;		// 线程号
	bool suspended_;				// 标志是否已挂起
	bool terminated_;				// 标志是否已终止
	bool self_delete_;				// 是否自己删除
	TCHAR thread_info_[128];		// 一个线程保存信息

public:
	// 线程是否自删除，默认关闭，否则不容易管理
	CThread(bool self_delete = false);
	virtual ~CThread();

public:
	/* 启动线程 */
	bool start_thread(bool suspended = false);

	/* 终止线程 */
	bool stop_thread(DWORD ms = 1000);

	/* 等待线程 */
	DWORD wait_for(DWORD ms = -1);

	/* 挂起线程 */
	DWORD suspend_thread();

	/* 唤醒线程 */
	DWORD resume_thread(bool all = true);

public:
	/* 判断线程是否有效 */
	bool is_valid();

	/* 得到线程句柄 */
	HANDLE get_handle();

	/* 得到线程编号 */
	DWORD get_threadid();

	/* 设置得到线程信息，最大为128字节 */
	void set_threadinfo(LPCTSTR info);
	LPCTSTR get_threadinfo();

	/* 设置为自动删除 */
	void set_autodelete(bool del);

	/* 返回是否自动删除 */
	bool get_autodelete();

	/* 设置线程退出标志 */
	void set_terminate();
	
protected:
	static UINT WINAPI ThreadProc(LPVOID param)
	{
		CThread* thread = reinterpret_cast<CThread*>(param);
		return thread->execute();
	}

	/* 线程人口 */
	UINT execute();

	/* 线程的主运行体 */
	virtual UINT on_execute() = 0;

	/* 线程开始事件，还没有完全开始 */
	virtual void on_init() {}

	/* 线程已经创建，但是不保证启动 */
	virtual void on_start() {}

	/* 线程停止事件，term为是否强行中止！ */
	virtual void on_stop(bool term) {}

	/* 线程正常结束，即线程体执行完毕 */
	virtual void on_runover() {}

	/* 异常事件，发生异常之后调用 */
	virtual void on_exception() {}

};




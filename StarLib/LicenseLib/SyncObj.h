#pragma	once

//
// 封装各种多线锁操作
// 为了防止和MFC或者其他库的名称重复
// 在所有名称前都加上了Sync前缀
// 同时提供了一个定界加锁的模式CLockGuard
// 使用他会使Lock更稳定
//
// liusiyang
//

class CSyncLock
{
private:
	CRITICAL_SECTION m_cs;
	CSyncLock(CSyncLock CONST& rSrc);
	CSyncLock& operator = (CSyncLock CONST& rSrc);

public:
	CSyncLock()	{
		InitializeCriticalSection(&m_cs);
	}

	~CSyncLock() {
		DeleteCriticalSection(&m_cs);
	}

	BOOL Lock()	{
		EnterCriticalSection(&m_cs);
		return TRUE;
	}

	VOID UnLock() {
		LeaveCriticalSection(&m_cs);
	}
};

// 定界加锁模式
class CLockGuard
{
private:
	CSyncLock& m_cLock;
	CLockGuard& operator = (CLockGuard& cLock);

public:
	CLockGuard(CSyncLock& cLock) : m_cLock(cLock) {
		m_cLock.Lock();
	}

	~CLockGuard() {
		m_cLock.UnLock();
	}
};


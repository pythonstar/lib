#pragma	once

//
// ��װ���ֶ���������
// Ϊ�˷�ֹ��MFC����������������ظ�
// ����������ǰ��������Syncǰ׺
// ͬʱ�ṩ��һ�����������ģʽCLockGuard
// ʹ������ʹLock���ȶ�
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

// �������ģʽ
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


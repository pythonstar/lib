#pragma once

//
// ��װ��windows�̣߳�ʹ�õ�ʱ��������������ͬ��MFC��CWinThread
// ע��deleteһ���߳�ָ���ʱ��������Ҫ1���ӡ�������Ҫע���û��ȴ�����
// 
// liusiyang
//

class CThread
{
protected:
	HANDLE thread_handle_;			// �߳̾��
	unsigned int thread_id_;		// �̺߳�
	bool suspended_;				// ��־�Ƿ��ѹ���
	bool terminated_;				// ��־�Ƿ�����ֹ
	bool self_delete_;				// �Ƿ��Լ�ɾ��
	TCHAR thread_info_[128];		// һ���̱߳�����Ϣ

public:
	// �߳��Ƿ���ɾ����Ĭ�Ϲرգ��������׹���
	CThread(bool self_delete = false);
	virtual ~CThread();

public:
	/* �����߳� */
	bool start_thread(bool suspended = false);

	/* ��ֹ�߳� */
	bool stop_thread(DWORD ms = 1000);

	/* �ȴ��߳� */
	DWORD wait_for(DWORD ms = -1);

	/* �����߳� */
	DWORD suspend_thread();

	/* �����߳� */
	DWORD resume_thread(bool all = true);

public:
	/* �ж��߳��Ƿ���Ч */
	bool is_valid();

	/* �õ��߳̾�� */
	HANDLE get_handle();

	/* �õ��̱߳�� */
	DWORD get_threadid();

	/* ���õõ��߳���Ϣ�����Ϊ128�ֽ� */
	void set_threadinfo(LPCTSTR info);
	LPCTSTR get_threadinfo();

	/* ����Ϊ�Զ�ɾ�� */
	void set_autodelete(bool del);

	/* �����Ƿ��Զ�ɾ�� */
	bool get_autodelete();

	/* �����߳��˳���־ */
	void set_terminate();
	
protected:
	static UINT WINAPI ThreadProc(LPVOID param)
	{
		CThread* thread = reinterpret_cast<CThread*>(param);
		return thread->execute();
	}

	/* �߳��˿� */
	UINT execute();

	/* �̵߳��������� */
	virtual UINT on_execute() = 0;

	/* �߳̿�ʼ�¼�����û����ȫ��ʼ */
	virtual void on_init() {}

	/* �߳��Ѿ����������ǲ���֤���� */
	virtual void on_start() {}

	/* �߳�ֹͣ�¼���termΪ�Ƿ�ǿ����ֹ�� */
	virtual void on_stop(bool term) {}

	/* �߳��������������߳���ִ����� */
	virtual void on_runover() {}

	/* �쳣�¼��������쳣֮����� */
	virtual void on_exception() {}

};




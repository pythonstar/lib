#ifndef _WIN32_WINNT  // 允许使用特定于 Windows XP 或更高版本的功能。
#define _WIN32_WINNT 0x0501 // 将此值更改为相应的值，以适用于 Windows 的其他版本。
#endif      



// TODO: 在此处引用程序需要的其他头文件
#ifndef WINVER    // 允许使用特定于 Windows XP 或更高版本的功能。
#define WINVER 0x0501  // 将此值更改为相应的值，以适用于 Windows 的其他版本。
#endif

#ifndef _WIN32_WINNT  // 允许使用特定于 Windows XP 或更高版本的功能。
#define _WIN32_WINNT 0x0501 // 将此值更改为相应的值，以适用于 Windows 的其他版本。
#endif      

#ifndef _WIN32_WINDOWS  // 允许使用特定于 Windows 98 或更高版本的功能。
#define _WIN32_WINDOWS 0x0410 // 将它更改为适合 Windows Me 或更高版本的相应值。
#endif

#ifndef _WIN32_IE   // 允许使用特定于 IE 6.0 或更高版本的功能。
#define _WIN32_IE 0x0600 // 将此值更改为相应的值，以适用于 IE 的其他版本。值。
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS // 某些 CString 构造函数将是显式的

// 关闭 MFC 对某些常见但经常可放心忽略的警告消息的隐藏
#define _AFX_ALL_WARNINGS

#include "stdafx.h"
#include "AntiDebug.h"
#include <windows.h>
#include <WinBase.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <stdio.h>
#include "thread.h"
#include <time.h>

#if 0
Windows allow you to call the CreateProcess function with CREATE_SUSPENDED flag, that tells the API to keep the process suspended until the ResumeThread function is called.

This gives us time to grab the suspended thread's context using GetThreadContext function, then the EBX register will hold a pointer to the PBE(Process Enviroment Block) structure, which we need to determine the base address. 

From the layout of the PBE structure we can see that the ImageBaseAddress is stored at the 8th byte, therefore [EBX+8] will give us actual base address of the process being suspended.

Now we need the in-memory EXE and do appropiate alignment if the alignment of memory and in-memory EXE differs.

If the base address of suspended process and in-memory exe matches, plus if the imageSize of the in-memory exe is lesser or equal to the suspended process' we can simply use WriteProcessMemory to write in-memory exe into the memory space of the suspended process.

But if the aforementioned conditions weren't met, we need a little more magic. First, we need to unmap the original image using ZwUnmapViewOfSection, and then allocate enough memory using VirtualAllocEx within the memory space of the suspended process. Now we need to write the in-memory exe into the memory space of the suspended process using the WriteProcessMemory function.

Next, patch the BaseAddress of the in-memory exe into the PEB->ImageBaseAddress of the suspended process.

EAX register of the thread context holds EntryPoint address, which we need to rewrite with the EntryPoint address of the in-memory exe. Now we need to save the altered thread context using the SetThreadContext function.

Voila! We're ready to call the ResumeThread function on the suspended process to execute it!
#endif

//#pragma warning(push)
#pragma warning(disable: 4733)
#pragma warning(disable: 4731)
#pragma warning(disable: 4355)

#define test_out printf

class CCheckThread : public CThread
{
protected:
	BOOL under_debug_;
public:
	BOOL under_debug() {
		return under_debug_;
	}
	CCheckThread() : under_debug_(FALSE) {}
};

// 调试检测方法1
BOOL CheckDebugger_Method1(HANDLE process)
{
	typedef BOOL (WINAPI *CHECK_REMOTE_DEBUGGER_PRESENT)(HANDLE, PBOOL);
	HMODULE module = ::GetModuleHandle(_T("Kernel32"));
	if (module)
	{
		CHECK_REMOTE_DEBUGGER_PRESENT CheckRemoteDebuggerPresent_;
		CheckRemoteDebuggerPresent_ = (CHECK_REMOTE_DEBUGGER_PRESENT)GetProcAddress(module, "CheckRemoteDebuggerPresent");
		if (CheckRemoteDebuggerPresent_) 
		{
			BOOL bDebuggerPresent = FALSE;
			if ((*CheckRemoteDebuggerPresent_)(process, &bDebuggerPresent)) 
			{
				if (bDebuggerPresent) {
					test_out("cdm 1 check debug!\n");
				}
				return bDebuggerPresent;
			}
		}
		else
		{
			// 如果没有获取到CheckRemoteDebugger，则调用原始的检测函数
			return IsDebuggerPresent();
		}
	}
	return FALSE;
}

class CDM1 : public CCheckThread
{
	virtual UINT on_execute()
	{
		__try {
			if (CheckDebugger_Method1(::GetCurrentProcess())) {
				under_debug_ = TRUE;
				return -10000;
			}
		} __except(1) {
		}
		under_debug_ = FALSE;
		return 100;
	}
};

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

BOOL CheckDebugger_Method2()
{
	// 只能检测自己的进程是否在调试中
	__asm {
		mov eax, fs:[30h]
		mov eax, [eax+0x68]
		and	eax, 0x70
	}
}
class CDM2 : public CCheckThread
{
	virtual UINT on_execute()
	{
		__try {
			if (CheckDebugger_Method2()) {
				test_out("cdm 2 check debug!\n");
				under_debug_ = TRUE;
				return -20000;
			}
		} __except(1) {
		}
		under_debug_ = FALSE;
		return 200;
	}
};


/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

BOOL CheckDebugger_Method3(HANDLE hProcess)
{
	typedef long NTSTATUS; 
	#define STATUS_SUCCESS    ((NTSTATUS)0L) 

	typedef struct _SYSTEM_KERNEL_DEBUGGER_INFORMATION { 
		BOOLEAN DebuggerEnabled; 
		BOOLEAN DebuggerNotPresent; 
	} SYSTEM_KERNEL_DEBUGGER_INFORMATION, *PSYSTEM_KERNEL_DEBUGGER_INFORMATION; 

	typedef struct _PROCESS_DEBUG_PORT_INFO { 
		HANDLE DebugPort; 
	} PROCESS_DEBUG_PORT_INFO; 

	enum SYSTEM_INFORMATION_CLASS { SystemKernelDebuggerInformation = 35 }; 
	enum THREAD_INFO_CLASS        { ThreadHideFromDebugger          = 17 }; 
	enum PROCESS_INFO_CLASS       { ProcessDebugPort                = 7  }; 

	typedef NTSTATUS  (__stdcall *ZW_QUERY_SYSTEM_INFORMATION)(IN SYSTEM_INFORMATION_CLASS SystemInformationClass, IN OUT PVOID SystemInformation, IN ULONG SystemInformationLength, OUT PULONG ReturnLength); 
	typedef NTSTATUS  (__stdcall *ZW_SET_INFORMATION_THREAD)(IN HANDLE ThreadHandle, IN THREAD_INFO_CLASS ThreadInformationClass, IN PVOID ThreadInformation, IN ULONG ThreadInformationLength); 
	typedef NTSTATUS  (__stdcall *ZW_QUERY_INFORMATION_PROCESS)(IN HANDLE ProcessHandle, IN PROCESS_INFO_CLASS ProcessInformationClass, OUT PVOID ProcessInformation, IN ULONG ProcessInformationLength, OUT PULONG ReturnLength); 

	// 开始检测
	ZW_QUERY_SYSTEM_INFORMATION ZwQuerySystemInformation;
	ZW_QUERY_INFORMATION_PROCESS ZwQueryInformationProcess;
	SYSTEM_KERNEL_DEBUGGER_INFORMATION Info;
	PROCESS_DEBUG_PORT_INFO ProcessInfo;

	HMODULE hModule = GetModuleHandle(_T("ntdll.dll"));
	if (!hModule) {
		return FALSE;
	}
	ZwQuerySystemInformation = (ZW_QUERY_SYSTEM_INFORMATION)GetProcAddress(hModule, "ZwQuerySystemInformation");
	ZwQueryInformationProcess = (ZW_QUERY_INFORMATION_PROCESS)GetProcAddress(hModule, "ZwQueryInformationProcess");

	// 检测全局调试器
	if (ZwQuerySystemInformation) {
		if (STATUS_SUCCESS == ZwQuerySystemInformation(SystemKernelDebuggerInformation, &Info, sizeof(Info), NULL)) {
			if (Info.DebuggerEnabled&&!Info.DebuggerNotPresent) {
				return TRUE;
			}
		}
	}

	// 检测局部调试器
	if (ZwQueryInformationProcess) {
		if (STATUS_SUCCESS == ZwQueryInformationProcess(hProcess, ProcessDebugPort, &ProcessInfo, sizeof(ProcessInfo), NULL)) {
			if (ProcessInfo.DebugPort) {
				return TRUE;
			}
		}
	}

	return FALSE;
}

class CDM3 : public CCheckThread
{
	virtual UINT on_execute()
	{
		__try {
			if (CheckDebugger_Method3(::GetCurrentProcess())) {
				test_out("cdm 3 check debug!\n");
				under_debug_ = TRUE;
				return -30000;
			}
		} __except(1) {
		}
		under_debug_ = FALSE;
		return 300;
	}
};


/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

BOOL GetPIDPath(DWORD dwPID, LPTSTR exe_path, int max_path)
{
	BOOL success = FALSE;
	HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
	MODULEENTRY32 me32;

	// Take a snapshot of all modules in the specified process.
	hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
	if( hModuleSnap == INVALID_HANDLE_VALUE )
	{
		return( FALSE );
	}

	// Set the size of the structure before using it.
	me32.dwSize = sizeof( MODULEENTRY32 );

	// Retrieve information about the first module, and exit if unsuccessful
	if( !Module32First( hModuleSnap, &me32 ) )
	{
		CloseHandle( hModuleSnap );    // Must clean up the
		return( FALSE );
	}

	do   
	{
		if (me32.th32ProcessID == dwPID)   
		{   
			_tcsncpy(exe_path, me32.szExePath, max_path-1);
			success = TRUE;
			break;
		}   
	} while(Module32Next(hModuleSnap, &me32));   

	CloseHandle(hModuleSnap);     
	return success;
}   

BOOL CheckDebugger_Method4(DWORD processid)
{
	BOOL has_debugger = FALSE;
	HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
	PROCESSENTRY32 me32;

	// Take a snapshot of all modules in the specified process.
	hModuleSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if( hModuleSnap == INVALID_HANDLE_VALUE )
	{
		return( FALSE );
	}

	// Set the size of the structure before using it.
	me32.dwSize = sizeof( PROCESSENTRY32 );

	// Retrieve information about the first module, and exit if unsuccessful
	if( !Process32First( hModuleSnap, &me32 ) )
	{
		CloseHandle( hModuleSnap );    // Must clean up the
		return( FALSE );
	}

	// Now walk the module list of the process, and display information about each module
	do
	{
		if (me32.th32ProcessID == processid) 
		{
			TCHAR full_path[MAX_PATH] = {0};
			if (GetPIDPath(me32.th32ParentProcessID, full_path, MAX_PATH))
			{
				// 如果是系统目录的话，
				TCHAR system_path[MAX_PATH] = {0};
				GetSystemWindowsDirectory(system_path, MAX_PATH);
				if (!_tcsstr(full_path, system_path))
				{
					TCHAR cur_path[MAX_PATH] = {0};
					::GetModuleFileName(NULL, cur_path, MAX_PATH);
					TCHAR* p2 = _tcsrchr(cur_path, _T('\\'));
					if (p2)
					{
						*p2 = 0;
						if (!_tcsstr(full_path, cur_path))
						{
							has_debugger = TRUE;
						}
					}
				}
			}
			break;
		}
	} while( Process32Next ( hModuleSnap, &me32 ) );

	CloseHandle( hModuleSnap );
	return has_debugger;
}



class CDM4 : public CCheckThread
{
	virtual UINT on_execute()
	{
		__try {
			if (CheckDebugger_Method4(::GetCurrentProcessId())) {
				test_out("cdm 4 check debug!\n");
				under_debug_ = TRUE;
				return -40000;	
			}
		} __except(1) {
		}
		under_debug_ = FALSE;
		return 400;
	}
};

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

BOOL CheckDebugger_Method5()
{
	//	APIs making user of the ZwClose syscall (such as CloseHandle, indirectly) 
	//can be used to detect a debugger. When a process is debugged, calling ZwClose 
	//with an invalid handle will generate a STATUS_INVALID_HANDLE (0xC0000008) exception.
	//	As with all anti-debugs that rely on information made directly available 
	//from the kernel (therefore involving a syscall), the only proper way to bypass 
	//the "CloseHandle" anti-debug is to either modify the syscall data from ring3, 
	//before it is called, or set up a kernel hook.
	//	This anti-debug, though extremely powerful, does not seem to be widely used 
	//by malicious programs.

	__try {
		CloseHandle((HANDLE)0x3333);
	} __except(1) {
		return TRUE;
	}

	return FALSE;
}

class CDM5 : public CCheckThread
{
	virtual UINT on_execute()
	{
		__try {
			if (CheckDebugger_Method5()) {
				test_out("cdm 5 check debug!\n");
				under_debug_ = TRUE;
				return -50000;
			}
		} __except(1) {

		}
		under_debug_ = FALSE;
		return 500;
	}
};

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
BOOL CheckDebugger_Method6()
{
	DWORD csrss_id = -1;
	HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
	PROCESSENTRY32 me32;

	// Take a snapshot of all modules in the specified process.
	hModuleSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if( hModuleSnap == INVALID_HANDLE_VALUE )
	{
		return( FALSE );
	}

	// Set the size of the structure before using it.
	me32.dwSize = sizeof( PROCESSENTRY32 );

	// Retrieve information about the first module, and exit if unsuccessful
	if( !Process32First( hModuleSnap, &me32 ) )
	{
		CloseHandle( hModuleSnap );    // Must clean up the
		return( FALSE );
	}

	// Now walk the module list of the process, and display information about each module
	do
	{
		if (!_tcsicmp(me32.szExeFile, _T("csrss.exe")))
		{
			csrss_id = me32.th32ProcessID;
			break;
		}
	
	} while( Process32Next ( hModuleSnap, &me32 ) );

	CloseHandle( hModuleSnap );
	
	// 如果没有找到，直接返回
	if (csrss_id==-1) 
	{
		return FALSE;
	}

	// 只能检测本进程
	HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, csrss_id);  
	if (handle)
	{
		// 如果能打开，表示就有问题
		CloseHandle(handle);
		return TRUE;
	}
	return FALSE;
}

class CDM6 : public CCheckThread
{
	virtual UINT on_execute()
	{
		__try {
			if (CheckDebugger_Method6()) {
				test_out("cdm 6 check debug!\n");
				under_debug_ = TRUE;
				return -60000;
			}
		} __except(1) {

		}
		under_debug_ = FALSE;
		return 600;
	}
};


/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

BOOL CheckDebugger_Method7()
{
	HWND hwnd = ::GetForegroundWindow();
	if (hwnd)
	{
		TCHAR title[260];
		::GetWindowText(hwnd, title, 260);
		_tcslwr(title);
		if (_tcsstr(title, _T("lly"))) 
		{
			return TRUE;
		}
		else if (_tcsstr(title, _T("debug")))
		{
			return TRUE;
		}
		else if (_tcsstr(title, _T("dbg")))
		{
			return TRUE;
		}
		else if (_tcsstr(title, _T("ida")))
		{
			return TRUE;
		}
		else if (_tcsstr(title, _T("asm")))
		{
			return TRUE;
		}
	}

	if (FindWindow(NULL, _T("ollydbg"))) {
		return TRUE;
	}

	if (FindWindow(NULL, _T("TIdaWindow"))) {
		return TRUE;
	}

	return FALSE;
}

class CDM7 : public CCheckThread
{
	virtual UINT on_execute()
	{
		__try {
			if (CheckDebugger_Method7()) {
				test_out("cdm 7 check debug!\n");
				under_debug_ = TRUE;
				return -70000;
			}
		} __except(1) {

		}
		under_debug_ = FALSE;
		return 700;
	}
};



/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

BOOL CheckDeleteFib()
{
	char fib[1024] = {0};	
	__try { 
		DeleteFiber(fib); 
	}
	__except(1) {
		return FALSE;
	}

	if(GetLastError() == 0x00000057)
		return FALSE;

	return TRUE;
}

class CDM8 : public CCheckThread
{
	virtual UINT on_execute()
	{
		try {
			if (CheckDeleteFib()) {
				test_out("cdm 8 check debug!\n");
				under_debug_ = TRUE;
				return -80000;
			}
		} catch (...) {

		}
		under_debug_ = FALSE;
		return 800;
	}
};


/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

BOOL TimeChecker()
{
	DWORD time_begin;
	DWORD time_end;
	int time_low, time_high;

	JUNK_CODE_FOUR
	JUNK_CODE_THREE
	time_begin = ::GetTickCount();
	JUNK_CODE_TWO

	JUNK_CODE_THREE
	time_low = ::GetTickCount();
	JUNK_CODE_ONE
	JUNK_CODE_ONE

	JUNK_CODE_ONE
	time_high = ::GetTickCount();
	JUNK_CODE_FIVE

	for (int i=0;i<200;++i) {
		time_low += time_high;
		time_low -= i;
	}

	JUNK_CODE_ONE
	__asm
	{
		rdtsc
		mov time_low,eax
		mov time_high,edx
	}

	for (int i=0;i<200;++i) {
		time_low -= time_high;
		JUNK_CODE_ONE
		time_low += i;
	}
	JUNK_CODE_ONE
	JUNK_CODE_ONE
	time_end = GetTickCount();

	__asm
	{
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
	}

	__try
	{
		__asm { 
			int 2dh
			inc eax				;any opcode of singlebyte.
			JUNK_CODE_FIVE
		}
	}
	__except(1)
	{
		JUNK_CODE_THREE
		JUNK_CODE_ONE

		// 阈值设为13秒吧，防止机器突然卡死等问题
		if (time_end>=time_begin&&(time_end-time_begin < 13000)) {
			return FALSE;
		}
	}
	
	return TRUE;
}

class CDM9 : public CCheckThread
{
	virtual UINT on_execute()
	{
		try {
			if (TimeChecker()) {
				test_out("cdm 9 check debug!\n");
				under_debug_ = TRUE;
				return -90000;
			}
		} catch (...) {

		}
		under_debug_ = FALSE;
		return 900;
	}
};


/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

BOOL FTPushSSPopSS()
{
	__asm
	{
		push ebp
		mov ebp, esp

		push ss 
		pop ss 

		pushf
		pop eax 
		and eax, 0x100 
		or eax, eax 
		jnz _debugged

		xor eax, eax
		jmp _end

_debugged:
		mov eax, 1
_end:
		mov esp,ebp
		pop ebp
	}
}

class CDM10 : public CCheckThread
{
	virtual UINT on_execute()
	{
		try {
			if (FTPushSSPopSS()) {
				test_out("cdm 10 check debug!\n");
				under_debug_ = TRUE;
				return -100000;
			}
		} catch (...) {

		}
		under_debug_ = FALSE;
		return 1000;
	}
};


/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
// The IsDbgPresentPrefixCheck works in at least two debuggers
// OllyDBG and VS 2008, by utilizing the way the debuggers handle
// prefixes we can determine their presence. Specifically if this code
// is ran under a debugger it will simply be stepped over;
// however, if there is no debugger SEH will fire :D
BOOL IsDbgPresentPrefixCheck()
{
	__try
	{
		__asm __emit 0xF3 // 0xF3 0x64 disassembles as PREFIX REP:
		__asm __emit 0x64
		__asm __emit 0xF1 // One byte INT 1
	}
	__except(1)
	{
		return FALSE;
	}

	return TRUE;
}

class CDM11 : public CCheckThread
{
	virtual UINT on_execute()
	{
		try {
			if (IsDbgPresentPrefixCheck()) {
				test_out("cdm 11 check debug!\n");
				under_debug_ = TRUE;
				return -110000;
			}
		} catch (...) {

		}
		under_debug_ = FALSE;
		return 1100;
	}
};

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
// 直接停止调试
BOOL DisableDebugEvent()
{
	// 关闭调试端口
	typedef DWORD (WINAPI *ZW_SET_INFORMATION_THREAD)(HANDLE, DWORD, PVOID, ULONG);
	static const DWORD ThreadHideFromDebugger = 17;
	
	HMODULE module = ::GetModuleHandle(_T("NtDll"));
	if (module)
	{
		ZW_SET_INFORMATION_THREAD ZwSetInformationThread_;
		ZwSetInformationThread_ = (ZW_SET_INFORMATION_THREAD)GetProcAddress(module, "ZwSetInformationThread");
		if (ZwSetInformationThread_) 
		{
			(*ZwSetInformationThread_)(GetCurrentThread(), ThreadHideFromDebugger, 0, 0);
		}
	}
	return TRUE;
}

class CDM12 : public CCheckThread
{
	virtual UINT on_execute()
	{
		try {
			if (DisableDebugEvent()) {
				test_out("cdm 12 check debug!\n");
				under_debug_ = TRUE;
				return -120000;
			}
		} catch (...) {

		}
		under_debug_ = FALSE;
		return 1200;
	}
};


/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

#ifdef _ANTIDEBUG

VOID NTAPI test_PIMAGE_TLS_CALLBACK(PVOID DllHandle, DWORD Reason, PVOID Reserved)
{
#if 0
	IMAGE_DOS_HEADER *dos_head=(IMAGE_DOS_HEADER *)GetModuleHandle(NULL);
	PIMAGE_NT_HEADERS32 nt_head=(PIMAGE_NT_HEADERS32)((DWORD)dos_head+(DWORD)dos_head->e_lfanew);
	BYTE*OEP=(BYTE*)(nt_head->OptionalHeader.AddressOfEntryPoint+(DWORD)dos_head);

	//下面的代码则通过扫描程序入口点的20字节，判断其中有无调试断点，如有，则退出进程。
	for(unsigned long index=0;index<20;index++)
	{
		if (OEP[index]==0xcc) {
			ExitProcess(0);
		}
	}
#endif

	DisableDebugEvent();
	return;
}

#pragma comment(linker, "/INCLUDE:__tls_used")
#pragma data_seg(".CRT$XLB")
PIMAGE_TLS_CALLBACK TlsCallBackArray[] = {
	test_PIMAGE_TLS_CALLBACK,
	NULL
};
#pragma data_seg()

#endif

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

// case 宏
#define CREATE_THREAD(x) case x: thread_ = new CDM##x; break

class AntiThread : public CThread
{
	// 当前检测线程
	CCheckThread* thread_;

	DWORD update_tick_;
	DWORD thread_count_;
	BOOL under_debug_;
	HANDLE last_handle_;
	int last_thread_idx_;

public:
	AntiThread() : 
		thread_(NULL), 
		update_tick_(0), 
		thread_count_(0), 
		last_thread_idx_(0), 
		last_handle_(INVALID_HANDLE_VALUE) 
	{}

	int get_last_thread_idx() {
		return last_thread_idx_;
	}
	DWORD get_update_tick() {
		return update_tick_;
	}
	BOOL is_under_debug() 
	{
		DWORD now = ::GetTickCount();
		if (now>update_tick_&&(now-update_tick_>1200000)) {
			printf("---time update!\n");
			return TRUE;
		}

		//if (under_debug_) {
		//	printf("---in debug(%d)!\n", last_thread_idx_);
		//} else {
		//	printf("ok!\n");
		//}
		return under_debug_;
	}

	virtual UINT on_execute()
	{
		while (!terminated_) 
		{
			update_tick_ = ::GetTickCount();
			JUNK_CODE_ONE

			// 创建一个线程
			last_thread_idx_ = rand() % 11 + 1;
			//printf("thread = %d!\n", last_thread_idx_);

			JUNK_CODE_THREE
			switch (last_thread_idx_) {
			default:
				CREATE_THREAD(1);
				CREATE_THREAD(2);
				CREATE_THREAD(3);
				CREATE_THREAD(4);
				CREATE_THREAD(5);
				CREATE_THREAD(6);
				CREATE_THREAD(7);
				CREATE_THREAD(8);
				CREATE_THREAD(9);
				CREATE_THREAD(10);
				CREATE_THREAD(11);
			}
			if (!thread_) {
				continue;
			}

			//char temp[30];
			//sprintf(temp, "%d", last_thread_idx_);
			//MessageBox(NULL, temp, "asdfdsafa", MB_OK);

			test_out("select  = %d .", last_thread_idx_);


			thread_->start_thread();
			++thread_count_;

			// 等待检测线程是否结束
			while (!terminated_) {
				if (thread_->is_valid()&&WAIT_TIMEOUT == thread_->wait_for(50)) {
					continue;
				} else {
					break;
				}
			}

			JUNK_CODE_ONE
			JUNK_CODE_ONE
			// 在调试中
			under_debug_ |= thread_->under_debug();

			JUNK_CODE_ONE
			JUNK_CODE_TWO
			JUNK_CODE_ONE

			// 记录最后一次句柄
			last_handle_ = thread_->get_handle();


			JUNK_CODE_THREE
			JUNK_CODE_ONE

			// 得到线程退出值
			DWORD last_code = 0;
			if (::GetExitCodeThread(last_handle_, &last_code)) 
			{
				if ((int)last_code<-10000) 
				{
					JUNK_CODE_THREE
					under_debug_ = TRUE;
				}
			}

			JUNK_CODE_FIVE

			// 删除
			delete thread_;
			thread_ = NULL;

			// 休息
//#ifdef _DEBUG
//			int s_time = rand() % 20 + 100;
//#else
//			int s_time = rand() % 60000 + 30000;
//#endif
			int s_time = 200;

			// 休息一会，然后进行下一次检测
			JUNK_CODE_TWO
			JUNK_CODE_ONE
			for (int i=0;i<s_time/100;++i) {
				Sleep(100);
				if (terminated_) {
					break;
				}
			}
		}
		return 0;
	}
};

AntiThread* global_thread = NULL;

// 开始反调试检测
bool start_anti_debug()
{
#ifdef _ANTIDEBUG
	srand((unsigned int)time(NULL));

	// 开启一个后台程序，不时创建反调试线程，没创建一个增加一个计数
	global_thread = new AntiThread;
	global_thread->start_thread();
#endif

	return true;
}

// 停止反调试检测
bool stop_anti_debug()
{
#ifdef _ANTIDEBUG
	if (global_thread) {
		global_thread->stop_thread();
		delete global_thread;
		global_thread = NULL;
	}
#endif

	return false;
}

// 是否在调试中
bool is_under_debug()
{
#ifdef _ANTIDEBUG
	if (global_thread) {
		return global_thread->is_under_debug() ? true : false;
	}
#endif

	return false;
}

bool test_under_debug()
{
	return false;

#ifdef _ANTIDEBUG
	CCheckThread* thread_ = NULL;
	int last_thread_idx_ = rand() % 10 + 1;
	//printf("thread = %d!\n", last_thread_idx_);

	JUNK_CODE_THREE
	switch (last_thread_idx_) {
	default:
		CREATE_THREAD(1);
		CREATE_THREAD(2);
		CREATE_THREAD(3);
		CREATE_THREAD(4);
		CREATE_THREAD(5);
		CREATE_THREAD(6);
		CREATE_THREAD(7);
		CREATE_THREAD(8);
		CREATE_THREAD(9);
		CREATE_THREAD(10);
	}
	if (!thread_) {
		return false;
	}

	thread_->start_thread();

	if (WAIT_TIMEOUT == thread_->wait_for(200)) 
	{
		if (thread_->is_valid()) {
			thread_->stop_thread(0);
			return false;
		}
	}

	BOOL under_debug_ = thread_->under_debug();
	
	delete thread_;

	return under_debug_ ? true : false;

#endif
	return false;
}
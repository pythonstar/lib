#include "stdafx.h"
#include "HookClass.h"

void EnablePrivilege(char*lpPriv,BOOL bFlags)
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;
	DWORD ReturnLen;

	OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,&hToken);
	LookupPrivilegeValue(NULL,lpPriv,&(tkp.Privileges[0].Luid));
	tkp.PrivilegeCount=1;
	if (bFlags)
	{
		tkp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
	}
	AdjustTokenPrivileges(hToken,false,&tkp,0,NULL,&ReturnLen);
	CloseHandle(hToken);
}

CHookClass::CHookClass(void)
{
	//EnablePrivilege()
}

void CHookClass::Create(PVOID OldFun,PVOID NewFun)
{
	DWORD nCount;

	OldFunction=OldFun;
	NewFunction=GetFuncRealBase(NewFun);
	//以特权方式打开当前进程
	hProcess=OpenProcess(PROCESS_ALL_ACCESS,FALSE, GetCurrentProcessId());
	//生成jmp xxxx的代码，共5字节
	Newcode.JmpCode=0x0E9;	//jmp指令的十六进制代码是E9
	Newcode.FuncAddr=DWORD(NewFunction)-DWORD(OldFunction)-5;
	//保存被截函数的前5个字节
	if (!ReadProcessMemory(hProcess,OldFunction,Oldcode,5,&nCount))
	{
		::MessageBox(NULL,"Hook failed on ReadProcessMemory",NULL,MB_OK|MB_ICONERROR);
	}
	bTrapped=FALSE;
}

CHookClass::~CHookClass(void)
{
	if (bTrapped)
	{
		Restore();
	}
	if (hProcess)
	{
		CloseHandle(hProcess);
		hProcess=NULL;
	}
	
}

//取函数的实际地址。如果函数的第一个指令是Jmp，则取出它的跳转地址（实际地址），这往往
//由于程序中含有Debug调试信息引起的
PVOID CHookClass::GetFuncRealBase(PVOID pFunc)
{
	PIMPORTCODE func;
	PVOID Result=pFunc;
	if(pFunc==NULL)
		return Result;
	func=PIMPORTCODE(pFunc);
	if (func->JumpInstruction==0x25FF)
	{
		//指令二进制码FF 25  汇编指令jmp [...]
		Result=*(func->AddressOfPointerToFunction);
	}
	return Result;
}


void CHookClass::Restore()
{
	DWORD nCount;

	if (bTrapped==FALSE||hProcess==NULL||OldFunction==NULL||NewFunction==NULL)
	{
		return;
	}

	//Thread::EnumAllThreads(SuspendAllThreads,GetCurrentProcessId());

	if (!WriteProcessMemory(hProcess, OldFunction,Oldcode, 5,&nCount))
	{
		::MessageBox(NULL,"Restore failed on WriteProcessMemory",NULL,MB_OK|MB_ICONERROR);
	}

	//Thread::EnumAllThreads(ResumedAllThreads,GetCurrentProcessId());

	bTrapped=FALSE;
}

void CHookClass::Change()
{
	DWORD nCount;

	if (bTrapped||hProcess==NULL||OldFunction==NULL||NewFunction==NULL)
	{
		return;
	}

	//Thread::EnumAllThreads(SuspendAllThreads,GetCurrentProcessId());

	if (!WriteProcessMemory(hProcess, OldFunction, &Newcode, 5,&nCount))
	{
		::MessageBox(NULL,"Change failed on WriteProcessMemory",NULL,MB_OK|MB_ICONERROR);
	}

	//Thread::EnumAllThreads(ResumedAllThreads,GetCurrentProcessId());

	bTrapped=TRUE;
}

BOOL CHookClass::SuspendAllThreads(THREADENTRY32*te)
{
	if (te->th32ThreadID==GetCurrentThreadId())
	{
		return FALSE;
	}

	HANDLE hThread=OpenThread(THREAD_ALL_ACCESS,FALSE,te->th32ThreadID);
	if(hThread==NULL)  
		return FALSE; 
	else
		::SuspendThread(hThread);

	CloseHandle(hThread);
	return TRUE;
}

BOOL CHookClass::ResumedAllThreads(THREADENTRY32*te)
{
	if (te->th32ThreadID==GetCurrentThreadId())
	{
		return FALSE;
	}

	HANDLE hThread=OpenThread(THREAD_ALL_ACCESS,FALSE,te->th32ThreadID);
	if(hThread==NULL)  
		return FALSE; 
	else
		::ResumeThread(hThread);

	CloseHandle(hThread);
	return TRUE;
}
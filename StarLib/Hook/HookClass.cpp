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
	//����Ȩ��ʽ�򿪵�ǰ����
	hProcess=OpenProcess(PROCESS_ALL_ACCESS,FALSE, GetCurrentProcessId());
	//����jmp xxxx�Ĵ��룬��5�ֽ�
	Newcode.JmpCode=0x0E9;	//jmpָ���ʮ�����ƴ�����E9
	Newcode.FuncAddr=DWORD(NewFunction)-DWORD(OldFunction)-5;
	//���汻�غ�����ǰ5���ֽ�
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

//ȡ������ʵ�ʵ�ַ����������ĵ�һ��ָ����Jmp����ȡ��������ת��ַ��ʵ�ʵ�ַ����������
//���ڳ����к���Debug������Ϣ�����
PVOID CHookClass::GetFuncRealBase(PVOID pFunc)
{
	PIMPORTCODE func;
	PVOID Result=pFunc;
	if(pFunc==NULL)
		return Result;
	func=PIMPORTCODE(pFunc);
	if (func->JumpInstruction==0x25FF)
	{
		//ָ���������FF 25  ���ָ��jmp [...]
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
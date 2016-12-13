#pragma once
#include <afx.h>

#include "../Process/Thread.h"

typedef struct IMPORTCODE
{
	WORD  JumpInstruction;
	PVOID*AddressOfPointerToFunction;
}*PIMPORTCODE;

#pragma pack(1)
typedef struct LONGJMP
{
	BYTE  JmpCode;
	DWORD FuncAddr;
}*PLONGJMP;

class CHookClass
{
public:
	CHookClass(void);
	void Create(PVOID OldFun,PVOID NewFun);
public:
	~CHookClass(void);
private:
	HANDLE	hProcess;
	char	Oldcode[5];
	LONGJMP	Newcode;
	BOOL	bTrapped;
public:
	PVOID OldFunction,NewFunction;
private:
	PVOID GetFuncRealBase(PVOID pFunc);
public:
	void Restore();
	void Change();
private:
	static BOOL SuspendAllThreads(THREADENTRY32*te);
	static BOOL ResumedAllThreads(THREADENTRY32*te);
};

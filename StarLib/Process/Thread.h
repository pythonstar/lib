

#pragma once
#include <Tlhelp32.h>
#include <Psapi.h>
#pragma comment(lib,"Psapi.lib")

/************************************************************************/
typedef enum _THREADINFOCLASS{  
	ThreadBasicInformation,  
	ThreadTimes,  
	ThreadPriority,  
	ThreadBasePriority,  
	ThreadAffinityMask,  
	ThreadImpersonationToken,  
	ThreadDescriptorTableEntry,  
	ThreadEnableAlignmentFaultFixup,  
	ThreadEventPair_Reusable,  
	ThreadQuerySetWin32StartAddress,  
	ThreadZeroTlsCell,  
	ThreadPerformanceCount,  
	ThreadAmILastThread,  
	ThreadIdealProcessor,  
	ThreadPriorityBoost,  
	ThreadSetTlsArrayAddress,  
	ThreadIsIoPending,  
	ThreadHideFromDebugger,  
	ThreadBreakOnTermination,  
	MaxThreadInfoClass  
}THREADINFOCLASS;  

typedef struct _CLIENT_ID{  
	HANDLE   UniqueProcess;  
	HANDLE   UniqueThread;  
}CLIENT_ID,*PCLIENT_ID;  

typedef struct _THREAD_BASIC_INFORMATION{   //   Information   Class   0  
	LONG		ExitStatus;  
	PVOID		TebBaseAddress;  
	CLIENT_ID   ClientId;  
	LONG		AffinityMask;  
	LONG		Priority;  
	LONG		BasePriority;  
}THREAD_BASIC_INFORMATION,*PTHREAD_BASIC_INFORMATION;  

typedef LONG (__stdcall*_pfnZwQueryInformationThread)(
	IN  HANDLE			ThreadHandle,  
	IN  THREADINFOCLASS	ThreadInformationClass,  
	OUT PVOID			ThreadInformation,  
	IN  ULONG			ThreadInformationLength,  
	OUT PULONG			ReturnLength OPTIONAL  
	); 
typedef LONG (__stdcall*_pfnRtlNtStatusToDosError)(IN ULONG status);
/************************************************************************/
typedef enum _MEMORY_INFORMATION_CLASS{
	MemoryBasicInformation,
	MemoryWorkingSetInformation,
	MemoryMappedFileNameInformation,
	MemoryRegionInformation,
	MemoryWorkingSetExInformation
}MEMORY_INFORMATION_CLASS,*PMEMORY_INFORMATION_CLASS;

typedef ULONG(WINAPI*_pfnZwQueryVirtualMemory)(
	HANDLE hProcess,PVOID BaseAddress,
	MEMORY_INFORMATION_CLASS MemoryInformationClass,
	PVOID MemoryInformation,SIZE_T MemoryInformationLength,
	PSIZE_T ReturnLength
	);
/************************************************************************/

class Thread
{
	typedef BOOL(*pfnEnumThreadCallBack)(THREADENTRY32*te);

public:
	static void EnumAllThreads(pfnEnumThreadCallBack EnumThreadCallBack,DWORD th32ProcessID);
	static BOOL GetDebugPrivilege();

public:
	static _pfnZwQueryInformationThread	ZwQueryInformationThread;
	static _pfnRtlNtStatusToDosError	RtlNtStatusToDosError;
};
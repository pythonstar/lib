#pragma once

#ifdef EXPORT_API
#define EXPORT_LIB __declspec(dllexport)
#else
#define EXPORT_LIB __declspec(dllimport)
#endif

namespace XGC
{
	namespace common
	{
		EXPORT_LIB void GetCpuUsage( int* pProcessUsage, int* pSystemUsage, int* pKernelUsage, int* pIdleUsage );
	}
}
#include "common/header.h"
#include "common/cpu.h"
#include "common/sync.h"

namespace XGC
{
	namespace common
	{
		section gCpuSection;
		volatile clock_t s_lastTime = 0;
		volatile xgc_int64 s_idleTime = 0;
		volatile xgc_int64 s_krnlTime = 0;
		volatile xgc_int64 s_userTime = 0;

		volatile xgc_int64 s_krnlTimeProcess = 0;
		volatile xgc_int64 s_userTimeProcess = 0;

		void GetCpuUsage( int* pProcessUsage, int* pSystemUsage, int* pKernelUsage, int* pIdleUsage )
		{
			clock_t thisTime;
			xgc_int64 idleTime;
			xgc_int64 kernelTime;
			xgc_int64 userTime;

			xgc_int64 createTime;
			xgc_int64 exitTime;

			xgc_int64 kernelTimeProcess;
			xgc_int64 userTimeProcess;

			thisTime = clock();
			if( s_lastTime == 0 )
			{
				autolock lock( gCpuSection );
				// for the system
				GetSystemTimes( (LPFILETIME)&idleTime, (LPFILETIME)&kernelTime, (LPFILETIME)&userTime );
				// printf( "idle = %lld, kernel = %lld, user = %lld all = %lld\n", idleTime, kernelTime, userTime, idleTime + kernelTime + userTime );

				s_idleTime = idleTime;
				s_krnlTime = kernelTime;
				s_userTime = userTime;

				// for this process
				GetProcessTimes( GetCurrentProcess(), (LPFILETIME)&createTime, (LPFILETIME)&exitTime, (LPFILETIME)&kernelTimeProcess, (LPFILETIME)&userTimeProcess );
				// printf( "process kernel = %lld, user = %lld", kernelTimeProcess, userTimeProcess );
				s_krnlTimeProcess = kernelTimeProcess;
				s_userTimeProcess = userTimeProcess;

				s_lastTime = thisTime;
			}
			else if( thisTime - s_lastTime > 200 )
			{
				autolock lock( gCpuSection );
				// for the system
				GetSystemTimes( (LPFILETIME)&idleTime, (LPFILETIME)&kernelTime, (LPFILETIME)&userTime );
				// printf( "idle = %lld, kernel = %lld, user = %lld all = %lld\n", idleTime, kernelTime, userTime, idleTime + kernelTime + userTime );

				xgc_int64 idleTime_t = idleTime - s_idleTime;
				xgc_int64 kernelTime_t = kernelTime - s_krnlTime;
				xgc_int64 userTime_t = userTime - s_userTime;

				s_idleTime = idleTime;
				s_krnlTime = kernelTime;
				s_userTime = userTime;

				// for this process
				GetProcessTimes( GetCurrentProcess(), (LPFILETIME)&createTime, (LPFILETIME)&exitTime, (LPFILETIME)&kernelTimeProcess, (LPFILETIME)&userTimeProcess );
				// printf( "process kernel = %lld, user = %lld", kernelTimeProcess, userTimeProcess );

				xgc_int64 kernelTimeProcess_t = kernelTimeProcess - s_krnlTimeProcess;
				xgc_int64 userTimeProcess_t = userTimeProcess - s_userTimeProcess;

				s_krnlTimeProcess = kernelTimeProcess;
				s_userTimeProcess = userTimeProcess;

				if( pKernelUsage )
					*pKernelUsage = (xgc_int32)( ( kernelTime_t - idleTime_t ) * 100 / ( kernelTime_t + userTime_t ));

				if( pIdleUsage )
					*pIdleUsage = (xgc_int32)( idleTime_t * 100 / ( kernelTime_t + userTime_t ) );

				if( pSystemUsage )
					*pSystemUsage = (xgc_int32)( ( kernelTime_t + userTime_t - idleTime_t ) * 100 / ( kernelTime_t + userTime_t ) );

				if( pProcessUsage )
					*pProcessUsage = (xgc_int32)( (kernelTimeProcess_t + userTimeProcess_t) * 100 / ( kernelTime_t + userTime_t ) );

				s_lastTime = thisTime;
			}
		}
	}
}
// OSVersion.cpp: implementation of the COSVersion class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "OSVersion.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COSVersion::COSVersion()
{

}

COSVersion::~COSVersion()
{

}

OSVERSION COSVersion::GetOSVersion()
{
	static OSVERSION nVersion = OSV_UNKNOWN;

	if (nVersion == OSV_UNKNOWN) // first time only
	{
		OSVERSIONINFO vinfo;
		vinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		
		BOOL rslt = GetVersionEx(&vinfo);
		
		if (rslt)
		{
			switch (vinfo.dwPlatformId)
			{
			case VER_PLATFORM_WIN32_NT:
				{
					switch (vinfo.dwMajorVersion)
					{
					case 3: // nt351
						ASSERT (0); // not supported
						break;
						
					case 4: // nt4
						nVersion = OSV_NT4;
						break;
						
					case 5: // >= w2k
						{
							switch (vinfo.dwMinorVersion)
							{
							case 0: // w2k
								nVersion = OSV_2K;
								break;
								
							case 1: // xp
								nVersion = OSV_XP;
								break;
								
							default: // > xp
								nVersion = OSV_XPP;
								break;
							}
						}
						break;
						
					case 6: // > XP
						{
							switch (vinfo.dwMinorVersion)
							{
							case 0: // vista
								nVersion = OSV_VISTA;
								break;
								
							case 1: // w7
								nVersion = OSV_WIN7;
								break;
								
							default: // > w7
								nVersion = OSV_WIN7P;
								break;
							}
						}
						break;
						
					default: // > w7
						nVersion = OSV_WIN7P;
						break;
					}
				}
				break;
				
			case VER_PLATFORM_WIN32_WINDOWS:
				{
					ASSERT (vinfo.dwMajorVersion == 4);
					
					switch (vinfo.dwMinorVersion)
					{
					case 0: 
						nVersion = OSV_95;
						break;
						
					case 10:
						nVersion = OSV_98;
						break;
						
					case 90:
						nVersion = OSV_ME;
						break;
						
					default:
						ASSERT (0);
						break;
					}

				}
				break;
				
			default:
				ASSERT (0);
				break;
			}
		}
	}

	return nVersion;
}

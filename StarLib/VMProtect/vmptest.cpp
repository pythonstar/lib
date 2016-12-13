// vmptest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "VMProtectSDK.h"
//#pragma comment(lib,"VMProtectSDK32.lib")

int _tmain(int argc, _TCHAR* argv[])
{
VMPBEGIN;
printf("VMProtect SDK Test.\n");
VMPEND;
return 0;
}
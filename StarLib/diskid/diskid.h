
#pragma once
#include <string>
using namespace std;
#define DISKID_MAXLEN	40

CString GetDiskId();
int GetHardDriveSerialNumber(char *sBuff,int nLen);

string GetReadPhysicalDriveInNTWithZeroRightsHexStr();
void safe_strcpy(char *pDst, int nDstLen, const char *pSrc, int nSrcLen = 0);

//  diskid32.cpp 
//  for displaying the details of hard drives in a command window 
//  06/11/00  Lynn McGuire  written with many contributions from others, 
//                            IDE drives only under Windows NT/2K and 9X, 
//                            maybe SCSI drives later 
//  11/20/03  Lynn McGuire  added MY_ReadPhysicalDriveInNTWithZeroRights 
//  10/26/05  Lynn McGuire  fix the MY_flipAndCodeBytes function 
//  12/8/06   Chunlin Deng  update the string function to security version. 
//  Testing Passed with Visul Studio 2005. 

#include "stdafx.h"
#include <stdlib.h> 
#include <stdio.h> 
#include <stddef.h> 
#include <string.h> 
#include "diskid.h"
#include <WinIoCtl.h>


extern int g_nDmpExceptionFlag;		//dump异常标志

//  Required to ensure correct PhysicalDrive IOCTL structure setup 
#pragma pack(1) 

#define  IDENTIFY_BUFFER_SIZE  512 

//  IOCTL commands 
#define  DFP_GET_VERSION				0x00074080 
#define  DFP_SEND_DRIVE_COMMAND			0x0007c084 
#define  DFP_RECEIVE_DRIVE_DATA			0x0007c088 
#define  FILE_DEVICE_SCSI				0x0000001b 
#define  IOCTL_SCSI_MINIPORT_IDENTIFY	((FILE_DEVICE_SCSI << 16) + 0x0501) 
#define  IOCTL_SCSI_MINIPORT			0x0004D008		//see NTDDSCSI.H for definition 

   //  GETVERSIONOUTPARAMS contains the data returned from the  
   //  Get Driver Version function. 
typedef struct _GETVERSIONOUTPARAMS 
{ 
   BYTE bVersion;      // Binary driver version. 
   BYTE bRevision;     // Binary driver revision. 
   BYTE bReserved;     // Not used. 
   BYTE bIDEDeviceMap; // Bit map of IDE devices. 
   DWORD fCapabilities; // Bit mask of driver capabilities. 
   DWORD dwReserved[4]; // For future use. 
} GETVERSIONOUTPARAMS, *PGETVERSIONOUTPARAMS, *LPGETVERSIONOUTPARAMS; 

   //  Bits returned in the fCapabilities member of GETVERSIONOUTPARAMS  
#define  CAP_IDE_ID_FUNCTION             1  // ATA ID command supported 
#define  CAP_IDE_ATAPI_ID                2  // ATAPI ID command supported 
#define  CAP_IDE_EXECUTE_SMART_FUNCTION  4  // SMART commannds supported 

   //  Valid values for the bCommandReg member of IDEREGS. 
#define  IDE_ATAPI_IDENTIFY  0xA1  //  Returns ID sector for ATAPI. 
#define  IDE_ATA_IDENTIFY    0xEC  //  Returns ID sector for ATA. 

   
   // The following struct defines the interesting part of the IDENTIFY 
   // buffer: 
typedef struct _IDSECTOR 
{ 
   USHORT  wGenConfig; 
   USHORT  wNumCyls; 
   USHORT  wReserved; 
   USHORT  wNumHeads; 
   USHORT  wBytesPerTrack; 
   USHORT  wBytesPerSector; 
   USHORT  wSectorsPerTrack; 
   USHORT  wVendorUnique[3]; 
   CHAR    sSerialNumber[20]; 
   USHORT  wBufferType; 
   USHORT  wBufferSize; 
   USHORT  wECCSize; 
   CHAR    sFirmwareRev[8]; 
   CHAR    sModelNumber[40]; 
   USHORT  wMoreVendorUnique; 
   USHORT  wDoubleWordIO; 
   USHORT  wCapabilities; 
   USHORT  wReserved1; 
   USHORT  wPIOTiming; 
   USHORT  wDMATiming; 
   USHORT  wBS; 
   USHORT  wNumCurrentCyls; 
   USHORT  wNumCurrentHeads; 
   USHORT  wNumCurrentSectorsPerTrack; 
   ULONG   ulCurrentSectorCapacity; 
   USHORT  wMultSectorStuff; 
   ULONG   ulTotalAddressableSectors; 
   USHORT  wSingleWordDMA; 
   USHORT  wMultiWordDMA; 
   BYTE    bReserved[128]; 
} IDSECTOR, *PIDSECTOR; 

typedef struct _SRB_IO_CONTROL 
{ 
   ULONG HeaderLength; 
   UCHAR Signature[8]; 
   ULONG Timeout; 
   ULONG ControlCode; 
   ULONG ReturnCode; 
   ULONG Length; 
} SRB_IO_CONTROL, *PSRB_IO_CONTROL; 

   // Define global buffers. 
BYTE MY_IdOutCmd [sizeof (SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1]; 

char *MY_ConvertToString (DWORD diskdata [256], int firstIndex, int lastIndex); 
void MY_PrintIdeInfo(int drive, DWORD diskdata [256],char *sDest,int nLen); 
BOOL MY_DoIDENTIFY (HANDLE, PSENDCMDINPARAMS, PSENDCMDOUTPARAMS, BYTE, BYTE, PDWORD); 
BOOL myIsalnum(char ch);

//判断字符串每一个字符是否是可见字符
BOOL IsAllAlbum(char *sBuff,int nLen);

CStringA FormatHexStr(const char *sMem,int nLen);

void safe_strcpy(char *pDst, int nDstLen, const char *pSrc, int nSrcLen)
{
	if ( pDst==NULL || pSrc==NULL ) {
		return;
	}

	if ( nSrcLen<=0 ) {
		nSrcLen = strlen(pSrc);
	}
	if ( nSrcLen==0 ) {
		pDst[0] = 0;
	}else{
		if ( nSrcLen < nDstLen ) {
			strncpy(pDst, pSrc, nSrcLen);
			pDst[nSrcLen] = 0;
		}else{
			strncpy(pDst, pSrc, nDstLen-1);
			pDst[nDstLen-1] = 0;
		}
	}
}

//  Max number of drives assuming primary/secondary, master/slave topology 
#define  MAX_IDE_DRIVES  16 

int MY_ReadPhysicalDriveInNTWithAdminRights(char *sBuff,int nLen)
{ 
	int nError = -1; 
	HANDLE hPhysicalDriveIOCTL = 0; 
	TCHAR driveName[MAX_PATH]; 
	TCHAR szPhysicalDriveFormat[] = { '\\', '\\', '.', '\\', 'P', 'h', 'y', 's', 'i', 'c', 'a', 'l', 'D', 'r', 'i', 'v', 'e', '%', 'd', '\0' };

	for ( int drive = 0; drive < MAX_IDE_DRIVES; drive++ ){ 
		//  Try to get a handle to PhysicalDrive IOCTL, report failure 
		//  and exit if can''t. 
		_stprintf_s (driveName, szPhysicalDriveFormat, drive); 
		//  Windows NT, Windows 2000, must have admin rights 
		hPhysicalDriveIOCTL = CreateFile (driveName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE , NULL, OPEN_EXISTING, 0, NULL); 
		
		// if (hPhysicalDriveIOCTL == INVALID_HANDLE_VALUE) 
		//    printf ("Unable to open physical drive %d, error code: 0x%lX\n", 
		//            drive, GetLastError ()); 
		
		if ( hPhysicalDriveIOCTL != INVALID_HANDLE_VALUE ){ 
			GETVERSIONOUTPARAMS VersionParams; 
			DWORD               cbBytesReturned = 0; 
			
			// Get the version, etc of PhysicalDrive IOCTL 
			memset ((void*) &VersionParams, 0, sizeof(VersionParams)); 
			if ( !DeviceIoControl (hPhysicalDriveIOCTL, DFP_GET_VERSION, NULL, 0, &VersionParams, sizeof(VersionParams), &cbBytesReturned, NULL) ){          
				// printf ("DFP_GET_VERSION failed for drive %d\n", i); 
				// continue; 
			} 
			
			// If there is a IDE device at number "i" issue commands 
			// to the device 
			if ( VersionParams.bIDEDeviceMap > 0 ){ 
				BYTE             bIDCmd = 0;   // IDE or ATAPI IDENTIFY cmd 
				SENDCMDINPARAMS  scip; 
				//SENDCMDOUTPARAMS OutCmd; 
				// Now, get the ID sector for all IDE devices in the system. 
				// If the device is ATAPI use the IDE_ATAPI_IDENTIFY command, 
				// otherwise use the IDE_ATA_IDENTIFY command 
				bIDCmd = (VersionParams.bIDEDeviceMap >> drive & 0x10) ? IDE_ATAPI_IDENTIFY : IDE_ATA_IDENTIFY; 
				memset (&scip, 0, sizeof(scip)); 
				memset (MY_IdOutCmd, 0, sizeof(MY_IdOutCmd)); 
				if ( MY_DoIDENTIFY(hPhysicalDriveIOCTL,&scip,(PSENDCMDOUTPARAMS)&MY_IdOutCmd, (BYTE)bIDCmd, (BYTE)drive, &cbBytesReturned) ){ 
					DWORD diskdata [256]; 
					int ijk = 0; 
					USHORT *pIdSector = (USHORT *)((PSENDCMDOUTPARAMS) MY_IdOutCmd) -> bBuffer; 
					for (ijk = 0; ijk < 256; ijk++) {
						diskdata [ijk] = pIdSector [ijk]; 
					}
					MY_PrintIdeInfo (drive, diskdata,sBuff,nLen); 
					nError = 0; 
				} 
			} 

			CloseHandle(hPhysicalDriveIOCTL); 
		} 
	}//end for
	return nError; 
} 

//  Required to ensure correct PhysicalDrive IOCTL structure setup 
#pragma pack(4) 

// 
// IOCTL_STORAGE_QUERY_PROPERTY 
// 
// Input Buffer: 
//      a STORAGE_PROPERTY_QUERY structure which describes what type of query 
//      is being done, what property is being queried for, and any additional 
//      parameters which a particular property query requires. 
// 
//  Output Buffer: 
//      Contains a buffer to place the results of the query into.  Since all 
//      property descriptors can be cast into a STORAGE_DESCRIPTOR_HEADER, 
//      the IOCTL can be called once with a small buffer then again using 
//      a buffer as large as the header reports is necessary. 
// 

#define IOCTL_STORAGE_QUERY_PROPERTY   CTL_CODE(IOCTL_STORAGE_BASE, 0x0500, METHOD_BUFFERED, FILE_ANY_ACCESS) 

// 
// Device property descriptor - this is really just a rehash of the inquiry 
// data retrieved from a scsi device 
// 
// This may only be retrieved from a target device.  Sending this to the bus 
// will result in an error 
// 
#pragma pack(4) 

//  function to decode the serial numbers of IDE hard drives 
//  using the IOCTL_STORAGE_QUERY_PROPERTY command  
/*
需要转换：
32 30 32 30 32 30 32 30 32 30 32 30 33 32 35 33 35 39 33 38 33 39 34 61 34 32 33 30 33 32 33 36 33 37 33 30 33 33 33 39 00 00  
20202020202032535938394a4230323637303339

无需转换：
0x02F7C1E2  20 20 20 20 20 20 20 20 20 20 20 20 36 56 4d 56              6VMV
0x02F7C1F2  30 33 43 42 00 00 00 00 00 00 00 00 00 00 00 00  03CB............
*/
string MY_flipAndCodeBytes(char * str) 
{
	char flipped[1000] = {0};
	int nIndex = 0;
	int nSize = 0;
	if ( str!=NULL ) {
		nSize = strlen(str);
	}

	for (int i = 0; i < nSize; i += 4) {
		for (int j = 1; j >= 0; j--) { 
			int sum = 0; 
			for (int k = 0; k < 2; k++) { 
				sum *= 16; 
				switch (str [i + j * 2 + k]) 
				{ 
				case '0':  sum += 0; break;
				case '1':  sum += 1; break;
				case '2':  sum += 2; break;
				case '3':  sum += 3; break;
				case '4':  sum += 4; break;
				case '5':  sum += 5; break;
				case '6':  sum += 6; break;
				case '7':  sum += 7; break;
				case '8':  sum += 8; break;
				case '9':  sum += 9; break;
				case 'a':  sum += 10; break;
				case 'b':  sum += 11; break;
				case 'c':  sum += 12; break;
				case 'd':  sum += 13; break;
				case 'e':  sum += 14; break;
				case 'f':  sum += 15; break;
				case 'A':  sum += 10; break;
				case 'B':  sum += 11; break;
				case 'C':  sum += 12; break;
				case 'D':  sum += 13; break;
				case 'E':  sum += 14; break;
				case 'F':  sum += 15; break;
				} 
			} //end for

			if ( sum > 0 ) { 
				flipped[nIndex] = (char)sum;
				nIndex++;
				if ( nIndex >= _countof(flipped) - 1 ) {
					break;
				}
			} 

		} //end for

		if (nIndex >= _countof(flipped) - 1) {
			break;
		}

	} //end for

	return flipped;
} 

typedef struct _MEDIA_SERAL_NUMBER_DATA { 
  ULONG  SerialNumberLength;  
  ULONG  Result; 
  ULONG  Reserved[2]; 
  UCHAR  SerialNumberData[1]; 
} MEDIA_SERIAL_NUMBER_DATA, *PMEDIA_SERIAL_NUMBER_DATA; 

string GetReadPhysicalDriveInNTWithZeroRightsHexStr()
{
	string strResult;
	int done = FALSE; 

	HANDLE hPhysicalDriveIOCTL = 0; 
	TCHAR driveName [MAX_PATH]; 
	TCHAR szPhysicalDriveFormat[] = { '\\', '\\', '.', '\\', 'P', 'h', 'y', 's', 'i', 'c', 'a', 'l', 'D', 'r', 'i', 'v', 'e', '%', 'd', '\0' };

	for (int drive = 0; drive < MAX_IDE_DRIVES; drive++){ 
		//  Try to get a handle to PhysicalDrive IOCTL, report failure 
		//  and exit if can''t. 
		_stprintf_s (driveName, szPhysicalDriveFormat, drive); 
		//  Windows NT, Windows 2000, Windows XP - admin rights not required 
		hPhysicalDriveIOCTL = CreateFile (driveName, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL); 

		if ( hPhysicalDriveIOCTL != INVALID_HANDLE_VALUE ) { 
			STORAGE_PROPERTY_QUERY query; 
			DWORD cbBytesReturned = 0; 
			char buffer [10000]; 

			memset ((void *) & query, 0, sizeof (query)); 
			query.PropertyId = StorageDeviceProperty; 
			query.QueryType = PropertyStandardQuery; 
			memset (buffer, 0, sizeof (buffer)); 
			if ( DeviceIoControl (hPhysicalDriveIOCTL, IOCTL_STORAGE_QUERY_PROPERTY, & query, sizeof(query), &buffer, sizeof(buffer), &cbBytesReturned, NULL) ){          
				STORAGE_DEVICE_DESCRIPTOR * descrip = (STORAGE_DEVICE_DESCRIPTOR *) & buffer; 
				
				if ( done==FALSE ){
					CStringA str = FormatHexStr(buffer,cbBytesReturned);
					strResult = str;
					done = TRUE; 
				}

			}else{ 
				DWORD err = GetLastError(); 
				//TracklogU (_T("1 drive : %d, DeviceIOControl IOCTL_STORAGE_QUERY_PROPERTY error = %d\n"), drive, err); 
			} 
			memset (buffer, 0, sizeof (buffer)); 
			if ( DeviceIoControl (hPhysicalDriveIOCTL, IOCTL_STORAGE_GET_MEDIA_SERIAL_NUMBER, NULL, 0, &buffer, sizeof(buffer), &cbBytesReturned, NULL) ) {          
				MEDIA_SERIAL_NUMBER_DATA * mediaSerialNumber = (MEDIA_SERIAL_NUMBER_DATA *) & buffer; 
				
				if ( done==FALSE ){
					CStringA str = FormatHexStr(buffer,cbBytesReturned);
					strResult = str;
					done = TRUE; 
				}

				//printf ("\n**** MEDIA_SERIAL_NUMBER_DATA for drive %d ****\nSerial Number = %s\n", drive, serialNumber); 
			}else{ 
				if ( done==FALSE ){
					CStringA str;
					str.Format("2 drive : %d, DeviceIOControl IOCTL_STORAGE_GET_MEDIA_SERIAL_NUMBER error = %d\n", drive, GetLastError()); 
					strResult = str;
				}
			} 

			CloseHandle (hPhysicalDriveIOCTL); 

		} else{
			if ( done==FALSE ){
				CStringA str;
				str.Format("Unable to open physical drive %d, error code: %d\n", drive, GetLastError()); 
				strResult = str;
			}
		}


		if ( done==TRUE ){
			break;
		}
	}//end for

	return strResult; 
}

int MY_ReadPhysicalDriveInNTWithZeroRights(char *sBuff, int nLen, char *pModuleNumber = NULL, int nModuleLen = 100)
{ 
	int nError = -1;
	if ( sBuff==NULL || nLen<=0 ){
		return nError;
	}

	HANDLE hPhysicalDriveIOCTL = 0; 
	TCHAR driveName [MAX_PATH]; 
	TCHAR szPhysicalDriveFormat[] = { '\\', '\\', '.', '\\', 'P', 'h', 'y', 's', 'i', 'c', 'a', 'l', 'D', 'r', 'i', 'v', 'e', '%', 'd', '\0' };

	for (int drive = 0; drive < MAX_IDE_DRIVES; drive++){ 
		//  Try to get a handle to PhysicalDrive IOCTL, report failure 
		//  and exit if can''t. 
		_stprintf_s (driveName, szPhysicalDriveFormat, drive); 
		//  Windows NT, Windows 2000, Windows XP - admin rights not required 
		hPhysicalDriveIOCTL = CreateFile (driveName, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL); 
		
		if ( hPhysicalDriveIOCTL != INVALID_HANDLE_VALUE ) { 
			STORAGE_PROPERTY_QUERY query; 
			DWORD cbBytesReturned = 0; 
			char buffer [10000]; 

			memset ((void *) & query, 0, sizeof (query)); 
			query.PropertyId = StorageDeviceProperty; 
			query.QueryType = PropertyStandardQuery; 
			memset (buffer, 0, sizeof (buffer)); 
			if ( DeviceIoControl (hPhysicalDriveIOCTL, IOCTL_STORAGE_QUERY_PROPERTY, & query, sizeof(query), &buffer, sizeof(buffer), &cbBytesReturned, NULL) ){          
				STORAGE_DEVICE_DESCRIPTOR *descrip = (STORAGE_DEVICE_DESCRIPTOR *)&buffer; 

				char serialNumber [1000] = {0}; 
				char modelNumber [1000] = {0}; 
				int nSerialNumberOffset = (int)descrip->SerialNumberOffset;
				if ( nSerialNumberOffset>=0 && nSerialNumberOffset < (int)cbBytesReturned ) {

					safe_strcpy(serialNumber, sizeof(serialNumber), MY_flipAndCodeBytes(&buffer[nSerialNumberOffset]).c_str()); 
					//判断转换后是不是可见字符，转换是2个字符转换一个，所以长度除以2
					if ( IsAllAlbum(serialNumber,strlen(&buffer[nSerialNumberOffset])/2)==FALSE ){
						strcpy_s (serialNumber,&buffer[nSerialNumberOffset]); 
					}

					safe_strcpy (modelNumber, sizeof(modelNumber), & buffer [descrip -> ProductIdOffset]); 
					//TracklogU(_T("serialNumber : %s, model number : %s"), serialNumber, modelNumber);
					if(pModuleNumber != NULL && nModuleLen > 0)
					{
						safe_strcpy(pModuleNumber, nModuleLen, modelNumber);
					}
					//g_strError += _T("serial:\n") + FormatMem((PBYTE)serialNumber,1000);
					//g_strError += _T("model:\n") + FormatMem((PBYTE)modelNumber,1000);
					nError = -2;
					if ( 0 == sBuff[0] ){
						//说明硬盘序列号没获取到，已经获取到了就不再覆盖
						int nChSize = strlen(serialNumber);
						if ( IsAllAlbum(serialNumber,nChSize)==TRUE ){
							safe_strcpy(sBuff, nLen, serialNumber, nChSize); 
						}else{
							CStringA str = FormatHexStr(serialNumber,nChSize);
							if ( str.GetLength()>=nLen ){
								str = str.Left(nLen-1);
							}
							safe_strcpy(sBuff, nLen, str, str.GetLength()); 
						}
						nError = 0; 
					}
				}else{
					////可疑较多，暂时屏蔽
					//if ( nSerialNumberOffset > (int)cbBytesReturned && descrip->SerialNumberOffset!=0xFFFFFFFF ) {
					//	//出现此可疑时 八成用了硬盘id修改工具
					//	g_nDmpExceptionFlag |= 0x10;
					//}
					nError = -3;
				}
			}else{ 
				DWORD err = GetLastError(); 
				//TracklogU (_T("1 drive : %d, DeviceIOControl IOCTL_STORAGE_QUERY_PROPERTY error = %d\n"), drive, err); 
			} 
			if ( nError!=0 ) {
				memset (buffer, 0, sizeof (buffer)); 
				if ( DeviceIoControl (hPhysicalDriveIOCTL, IOCTL_STORAGE_GET_MEDIA_SERIAL_NUMBER, NULL, 0, &buffer, sizeof(buffer), &cbBytesReturned, NULL) ) {          
					MEDIA_SERIAL_NUMBER_DATA * mediaSerialNumber = (MEDIA_SERIAL_NUMBER_DATA *) & buffer; 
					char serialNumber [1000] = {0}; 
					// char modelNumber [1000]; 
					safe_strcpy (serialNumber, sizeof(serialNumber), (char *) mediaSerialNumber -> SerialNumberData); 
					// strcpy_s (modelNumber, & buffer [descrip -> ProductIdOffset]); 
					//g_strError += _T("serial:\n") + FormatMem((PBYTE)serialNumber,1000);
					nError = -4;
					if ( 0 == sBuff[0] ){
						//说明硬盘序列号没获取到，已经获取到了就不再覆盖
						int nChSize = strlen(serialNumber);
						if ( IsAllAlbum(serialNumber,nChSize)==TRUE ){
							safe_strcpy(sBuff, nLen, serialNumber, nChSize); 
						}else{
							CStringA str = FormatHexStr(serialNumber,nChSize);
							if ( str.GetLength()>=nLen ){
								str = str.Left(nLen-1);
							}
							safe_strcpy(sBuff, nLen, str, str.GetLength()); 
						}
						nError = 0; 
					}

					//printf ("\n**** MEDIA_SERIAL_NUMBER_DATA for drive %d ****\nSerial Number = %s\n", drive, serialNumber); 
				}else{ 
					nError = GetLastError();
					//TracklogU (_T("2 drive : %d, DeviceIOControl IOCTL_STORAGE_GET_MEDIA_SERIAL_NUMBER error = %d\n"), drive, GetLastError()); 
					//DWORD err = GetLastError (); 
					//switch (err) 
					//{ 
					//case 1:  
					//	printf ("\nDeviceIOControl IOCTL_STORAGE_GET_MEDIA_SERIAL_NUMBER error = \n" 
					//		"              The request is not valid for this device.\n\n"); 
					//	break; 
					//case 50: 
					//	printf ("\nDeviceIOControl IOCTL_STORAGE_GET_MEDIA_SERIAL_NUMBER error = \n" 
					//		"              The request is not supported for this device.\n\n"); 
					//	break; 
					//default: 
					//	printf ("\nDeviceIOControl IOCTL_STORAGE_GET_MEDIA_SERIAL_NUMBER error = %d\n\n", err); 
					//} 
				} 
			}

			CloseHandle (hPhysicalDriveIOCTL); 
		} else {
			nError = GetLastError();
		}

		
		if ( nError==0 ){
			break;
		}
	}//end for
	return nError; 
} 

// MY_DoIDENTIFY 
// FUNCTION: Send an IDENTIFY command to the drive 
// bDriveNum = 0-3 
// bIDCmd = IDE_ATA_IDENTIFY or IDE_ATAPI_IDENTIFY 
BOOL MY_DoIDENTIFY (HANDLE hPhysicalDriveIOCTL, PSENDCMDINPARAMS pSCIP, PSENDCMDOUTPARAMS pSCOP, BYTE bIDCmd, BYTE bDriveNum, PDWORD lpcbBytesReturned) 
{ 
   // Set up data structures for IDENTIFY command. 
   pSCIP -> cBufferSize = IDENTIFY_BUFFER_SIZE; 
   pSCIP -> irDriveRegs.bFeaturesReg = 0; 
   pSCIP -> irDriveRegs.bSectorCountReg = 1; 
   //pSCIP -> irDriveRegs.bSectorNumberReg = 1; 
   pSCIP -> irDriveRegs.bCylLowReg = 0; 
   pSCIP -> irDriveRegs.bCylHighReg = 0; 
   // Compute the drive number. 
   pSCIP -> irDriveRegs.bDriveHeadReg = 0xA0 | ((bDriveNum & 1) << 4); 
   // The command can either be IDE identify or ATAPI identify. 
   pSCIP -> irDriveRegs.bCommandReg = bIDCmd; 
   pSCIP -> bDriveNumber = bDriveNum; 
   pSCIP -> cBufferSize = IDENTIFY_BUFFER_SIZE; 
   return ( DeviceIoControl (hPhysicalDriveIOCTL, DFP_RECEIVE_DRIVE_DATA, (LPVOID) pSCIP, sizeof(SENDCMDINPARAMS) - 1, (LPVOID) pSCOP, sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1, lpcbBytesReturned, NULL) ); 
} 

//  --------------------------------------------------- 
   // (* Output Bbuffer for the VxD (rt_IdeDinfo record) *) 
typedef struct _rt_IdeDInfo_ 
{ 
    BYTE IDEExists[4]; 
    BYTE DiskExists[8]; 
    WORD DisksRawInfo[8*256]; 
} rt_IdeDInfo, *pt_IdeDInfo; 

   // (* IdeDinfo "data fields" *) 
typedef struct _rt_DiskInfo_ 
{ 
   BOOL DiskExists; 
   BOOL ATAdevice; 
   BOOL RemovableDevice; 
   WORD TotLogCyl; 
   WORD TotLogHeads; 
   WORD TotLogSPT; 
   char SerialNumber[20]; 
   char FirmwareRevision[8]; 
   char ModelNumber[40]; 
   WORD CurLogCyl; 
   WORD CurLogHeads; 
   WORD CurLogSPT; 
} rt_DiskInfo; 

#define  m_cVxDFunctionIdesDInfo  1 

//////////////////////////////////////////////////////////////////////////

int MY_ReadDrivePortsInWin9X(char *sBuff,int nLen)
{ 
	int done = FALSE; 
	HANDLE VxDHandle = 0; 
	pt_IdeDInfo pOutBufVxD = 0; 
	DWORD lpBytesReturned = 0; 
	TCHAR szIDE21201VXDFormat[] = { '\\', '\\', '.', '\\', 'I', 'D', 'E', '2', '1', '2', '0', '1', '.', 'V', 'X', 'D', '\0' };

	//  set the thread priority high so that we get exclusive access to the disk 
	BOOL status = SetPriorityClass (GetCurrentProcess (), REALTIME_PRIORITY_CLASS); 
	if (0 == status){
		//printf ("\nERROR: Could not SetPriorityClass, LastError: %d\n", GetLastError ()); 
	}  
	// 1. Make an output buffer for the VxD 
	rt_IdeDInfo info; 
	pOutBufVxD = &info; 
	// ***************** 
	// KLUDGE WARNING!!! 
	// HAVE to zero out the buffer space for the IDE information! 
	// If this is NOT done then garbage could be in the memory 
	// locations indicating if a disk exists or not. 
	ZeroMemory (&info, sizeof(info)); 
	// 1. Try to load the VxD 
	//  must use the short file name path to open a VXD file 
	//char StartupDirectory [2048]; 
	//char shortFileNamePath [2048]; 
	//char *p = NULL; 
	//char vxd [2048]; 
	//  get the directory that the exe was started from 
	//GetModuleFileName (hInst, (LPSTR) StartupDirectory, sizeof (StartupDirectory)); 
	//  cut the exe name from string 
	//p = &(StartupDirectory [strlen (StartupDirectory) - 1]); 
	//while (p >= StartupDirectory && *p && ''\\'' != *p) p--; 
	//*p = ''\0'';    
	//GetShortPathName (StartupDirectory, shortFileNamePath, 2048); 
	//sprintf_s (vxd, "\\\\.\\%s\\IDE21201.VXD", shortFileNamePath); 
	//VxDHandle = CreateFile (vxd, 0, 0, 0, 
	//               0, FILE_FLAG_DELETE_ON_CLOSE, 0);    
	VxDHandle = CreateFile (szIDE21201VXDFormat, 0, 0, 0, 0, FILE_FLAG_DELETE_ON_CLOSE, 0); 
	if (VxDHandle != INVALID_HANDLE_VALUE) { 
		// 2. Run VxD function 
		DeviceIoControl (VxDHandle, m_cVxDFunctionIdesDInfo, 0, 0, pOutBufVxD, sizeof(pt_IdeDInfo), &lpBytesReturned, 0); 
		// 3. Unload VxD 
		CloseHandle (VxDHandle); 
	}else{
		//MessageBox (NULL, "ERROR: Could not open IDE21201.VXD file",TITLE, MB_ICONSTOP); 
	}
 
		// 4. Translate and store data 
	for (int i=0; i<8; i++) { 
		if((pOutBufVxD->DiskExists[i]) && (pOutBufVxD->IDEExists[i/2])) { 
			DWORD diskinfo [256]; 
			for (int j = 0; j < 256; j++)  
			diskinfo [j] = pOutBufVxD -> DisksRawInfo [i * 256 + j]; 
					// process the information for this buffer 
			MY_PrintIdeInfo (i, diskinfo,sBuff,nLen); 
			done = TRUE; 
		} 
	} 
	//  reset the thread priority back to normal 
	// SetThreadPriority (GetCurrentThread(), THREAD_PRIORITY_NORMAL); 
	SetPriorityClass (GetCurrentProcess (), NORMAL_PRIORITY_CLASS); 
	return done; 
} 

#define  SENDIDLENGTH  sizeof (SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE 

int MY_ReadIdeDriveAsScsiDriveInNT(char *sBuff,int nLen)
{ 
   int nError = -1;
   int controller = 0; 
   TCHAR szScsiFormat[] = { '\\', '\\', '.', '\\', 'S', 'c', 's', 'i', '%', 'd', ':', '"', '\0' };

   for (controller = 0; controller < 16; controller++) 
   { 
      HANDLE hScsiDriveIOCTL = 0; 
      TCHAR  driveName [MAX_PATH]; 
         //  Try to get a handle to PhysicalDrive IOCTL, report failure 
         //  and exit if can''t. 
      _stprintf_s (driveName, szScsiFormat, controller); 
         //  Windows NT, Windows 2000, any rights should do 
      hScsiDriveIOCTL = CreateFile (driveName, 
                               GENERIC_READ | GENERIC_WRITE,  
                               FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, 
                               OPEN_EXISTING, 0, NULL); 
      // if (hScsiDriveIOCTL == INVALID_HANDLE_VALUE) 
      //    printf ("Unable to open SCSI controller %d, error code: 0x%lX\n", 
      //            controller, GetLastError ()); 
      if (hScsiDriveIOCTL != INVALID_HANDLE_VALUE) 
      { 
         int drive = 0; 
         for (drive = 0; drive < 2; drive++) 
         { 
            char buffer [sizeof (SRB_IO_CONTROL) + SENDIDLENGTH]; 
            SRB_IO_CONTROL *p = (SRB_IO_CONTROL *) buffer; 
            SENDCMDINPARAMS *pin = 
                   (SENDCMDINPARAMS *) (buffer + sizeof (SRB_IO_CONTROL)); 
            DWORD dummy; 
    
            memset (buffer, 0, sizeof (buffer)); 
            p -> HeaderLength = sizeof (SRB_IO_CONTROL); 
            p -> Timeout = 10000; 
            p -> Length = SENDIDLENGTH; 
            p -> ControlCode = IOCTL_SCSI_MINIPORT_IDENTIFY; 
            strncpy_s ((char *) p -> Signature,9,"SCSIDISK", 8); 
              
            pin -> irDriveRegs.bCommandReg = IDE_ATA_IDENTIFY; 
            pin -> bDriveNumber = drive; 
            if (DeviceIoControl (hScsiDriveIOCTL, IOCTL_SCSI_MINIPORT,  
                                 buffer, 
                                 sizeof (SRB_IO_CONTROL) + 
                                         sizeof (SENDCMDINPARAMS) - 1, 
                                 buffer, 
                                 sizeof (SRB_IO_CONTROL) + SENDIDLENGTH, 
                                 &dummy, NULL)) 
            { 
               SENDCMDOUTPARAMS *pOut = 
                    (SENDCMDOUTPARAMS *) (buffer + sizeof (SRB_IO_CONTROL)); 
               IDSECTOR *pId = (IDSECTOR *) (pOut -> bBuffer); 
               if (pId -> sModelNumber [0]) 
               { 
                  DWORD diskdata [256]; 
                  int ijk = 0; 
                  USHORT *pIdSector = (USHORT *) pId; 
           
                  for (ijk = 0; ijk < 256; ijk++) 
                     diskdata [ijk] = pIdSector [ijk]; 
				  MY_PrintIdeInfo (controller * 2 + drive, diskdata,sBuff,nLen); 
                  nError = 0; 
               } 
            } 
         } 
         CloseHandle (hScsiDriveIOCTL); 
      } 
   } 
   return nError; 
} 


//////////////////////////////////////////////////////////////////////////


//
// IDENTIFY data (from ATAPI driver source)
//

#pragma pack(1)

typedef struct _IDENTIFY_DATA {
	USHORT GeneralConfiguration;            // 00 00
	USHORT NumberOfCylinders;               // 02  1
	USHORT Reserved1;                       // 04  2
	USHORT NumberOfHeads;                   // 06  3
	USHORT UnformattedBytesPerTrack;        // 08  4
	USHORT UnformattedBytesPerSector;       // 0A  5
	USHORT SectorsPerTrack;                 // 0C  6
	USHORT VendorUnique1[3];                // 0E  7-9
	USHORT SerialNumber[10];                // 14  10-19
	USHORT BufferType;                      // 28  20
	USHORT BufferSectorSize;                // 2A  21
	USHORT NumberOfEccBytes;                // 2C  22
	USHORT FirmwareRevision[4];             // 2E  23-26
	USHORT ModelNumber[20];                 // 36  27-46
	UCHAR  MaximumBlockTransfer;            // 5E  47
	UCHAR  VendorUnique2;                   // 5F
	USHORT DoubleWordIo;                    // 60  48
	USHORT Capabilities;                    // 62  49
	USHORT Reserved2;                       // 64  50
	UCHAR  VendorUnique3;                   // 66  51
	UCHAR  PioCycleTimingMode;              // 67
	UCHAR  VendorUnique4;                   // 68  52
	UCHAR  DmaCycleTimingMode;              // 69
	USHORT TranslationFieldsValid:1;        // 6A  53
	USHORT Reserved3:15;
	USHORT NumberOfCurrentCylinders;        // 6C  54
	USHORT NumberOfCurrentHeads;            // 6E  55
	USHORT CurrentSectorsPerTrack;          // 70  56
	ULONG  CurrentSectorCapacity;           // 72  57-58
	USHORT CurrentMultiSectorSetting;       //     59
	ULONG  UserAddressableSectors;          //     60-61
	USHORT SingleWordDMASupport : 8;        //     62
	USHORT SingleWordDMAActive : 8;
	USHORT MultiWordDMASupport : 8;         //     63
	USHORT MultiWordDMAActive : 8;
	USHORT AdvancedPIOModes : 8;            //     64
	USHORT Reserved4 : 8;
	USHORT MinimumMWXferCycleTime;          //     65
	USHORT RecommendedMWXferCycleTime;      //     66
	USHORT MinimumPIOCycleTime;             //     67
	USHORT MinimumPIOCycleTimeIORDY;        //     68
	USHORT Reserved5[2];                    //     69-70
	USHORT ReleaseTimeOverlapped;           //     71
	USHORT ReleaseTimeServiceCommand;       //     72
	USHORT MajorRevision;                   //     73
	USHORT MinorRevision;                   //     74
	USHORT Reserved6[50];                   //     75-126
	USHORT SpecialFunctionsEnabled;         //     127
	USHORT Reserved7[128];                  //     128-255
} IDENTIFY_DATA, *PIDENTIFY_DATA;

#pragma pack()


int MY_ReadPhysicalDriveInNTUsingSmart(char *sBuff,int nLen)
{
	int nError = -1;
	int drive = 0;
	TCHAR szPhysicalDriveFormat[] = { '\\', '\\', '.', '\\', 'P', 'h', 'y', 's', 'i', 'c', 'a', 'l', 'D', 'r', 'i', 'v', 'e', '%', 'd', '\0' };

	for (drive = 0; drive < MAX_IDE_DRIVES; drive++)
	{
		HANDLE hPhysicalDriveIOCTL = 0;

		//  Try to get a handle to PhysicalDrive IOCTL, report failure
		//  and exit if can't.
		TCHAR driveName [MAX_PATH];

		_stprintf (driveName, szPhysicalDriveFormat, drive);

		//  Windows NT, Windows 2000, Windows Server 2003, Vista
		hPhysicalDriveIOCTL = CreateFile (driveName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		// if (hPhysicalDriveIOCTL == INVALID_HANDLE_VALUE)
		//    printf ("Unable to open physical drive %d, error code: 0x%lX\n",
		//            drive, GetLastError ());

		if (hPhysicalDriveIOCTL == INVALID_HANDLE_VALUE){
			//if (PRINT_DEBUG) 
			//	printf ("\n%d MY_ReadPhysicalDriveInNTUsingSmart ERROR"
			//	"\nCreateFile(%s) returned INVALID_HANDLE_VALUE\n"
			//	"Error Code %d\n",
			//	__LIMY__, driveName, GetLastError ());
		}else{
			GETVERSIONINPARAMS GetVersionParams;
			DWORD cbBytesReturned = 0;

			// Get the version, etc of PhysicalDrive IOCTL
			memset ((void*) & GetVersionParams, 0, sizeof(GetVersionParams));

			if ( ! DeviceIoControl (hPhysicalDriveIOCTL, SMART_GET_VERSION, NULL, 0, &GetVersionParams, sizeof (GETVERSIONINPARAMS), &cbBytesReturned, NULL) ){         
				//if (PRINT_DEBUG)
				//{
				//	DWORD err = GetLastError ();
				//	printf ("\n%d MY_ReadPhysicalDriveInNTUsingSmart ERROR"
				//		"\nDeviceIoControl(%d, SMART_GET_VERSION) returned 0, error is %d\n",
				//		__LIMY__, (int) hPhysicalDriveIOCTL, (int) err);
				//}
			}else{
				// Print the SMART version
				// PrintVersion (& GetVersionParams);
				// Allocate the command buffer
				ULONG CommandSize = sizeof(SENDCMDINPARAMS) + IDENTIFY_BUFFER_SIZE;
				PSENDCMDINPARAMS Command = (PSENDCMDINPARAMS)new BYTE[CommandSize];
				// Retrieve the IDENTIFY data
				// Prepare the command
				#define ID_CMD          0xEC            // Returns ID sector for ATA
				Command -> irDriveRegs.bCommandReg = ID_CMD;
				DWORD BytesReturned = 0;
				if ( ! DeviceIoControl (hPhysicalDriveIOCTL,SMART_RCV_DRIVE_DATA, Command, sizeof(SENDCMDINPARAMS), Command, CommandSize, &BytesReturned, NULL) ){
					// Print the error
					//PrintError ("SMART_RCV_DRIVE_DATA IOCTL", GetLastError());
				}else{
					// Print the IDENTIFY data
					DWORD diskdata [256];
					USHORT *pIdSector = (USHORT *)(PIDENTIFY_DATA) ((PSENDCMDOUTPARAMS) Command) -> bBuffer;

					for (int ijk = 0; ijk < 256; ijk++){
						diskdata [ijk] = pIdSector [ijk];
					}

					MY_PrintIdeInfo (drive, diskdata,sBuff,nLen); 
					nError = 0;
				}
				// Done
				delete []Command;
			}

			CloseHandle (hPhysicalDriveIOCTL);
		}
	}

	return nError;
}

BOOL myIsalnum(char ch)
{
	if ( (unsigned)(ch+1)<=256 ){	
		return isalnum(ch);
	}
	return FALSE;
}

//判断字符串每一个字符是否是可见字符，空格除外
BOOL IsAllAlbum(char *sBuff,int nLen)
{
	BOOL bYes = TRUE;

	if ( sBuff==NULL || nLen==0 ){
		return FALSE;
	}

	if ( nLen<0 ){
		nLen = strlen(sBuff);
	}

	char ch = 0;
	for ( int i=0; i<nLen; ++i ){
		ch = sBuff[i];
		if ( ch!=' ' && ch!='-' && myIsalnum(sBuff[i])==FALSE ){
			bYes = FALSE;
			break;
		}
	}//endfor

	return bYes;
}

void MY_PrintIdeInfo(int drive, DWORD diskdata [256],char *sDest,int nLen) 
{ 
   char string1 [1024]; 
   __int64 sectors = 0; 
   __int64 bytes = 0; 
      //  copy the hard drive serial number to the buffer 
   strcpy_s (string1, MY_ConvertToString (diskdata, 10, 19)); 
   if (0 == sDest[0] && 
            //  serial number must be alphanumeric 
            //  (but there can be leading spaces on IBM drives) 
       (myIsalnum (string1 [0]) || myIsalnum (string1 [19]))) 
   { 
      strcpy_s (sDest, nLen, string1); 
      //strcpy_s (HardDriveModelNumber, MY_ConvertToString (diskdata, 27, 46)); 
   } 
} 

char *MY_ConvertToString (DWORD diskdata [256], int firstIndex, int lastIndex) 
{ 
   static char string [1024]; 
   int position = 0; 
      //  each integer has two characters stored in it backwards 
   for (int index = firstIndex; index <= lastIndex; index++) 
   { 
         //  get high byte for 1st character 
      string [position] = (char) (diskdata [index] / 256); 
      position++; 
         //  get low byte for 2nd character 
      string [position] = (char) (diskdata [index] % 256); 
      position++; 
   } 
      //  end the string  
   string [position] = '\0'; 
      //  cut off the trailing blanks 
   for (int index = position - 1; index > 0 && ' ' == string [index]; index--) 
      string [index] = '\0'; 
   return string; 
} 

//返回0代表成功，否则返回错误码
int GetHardDriveSerialNumber(char *sBuff,int nLen)
{
	int nError = -1;
	int nSNSize = 0;
	OSVERSIONINFO version = {0};
	version.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
	GetVersionEx (&version);

	if ( sBuff==NULL || nLen<=0 ) {
		return nError;
	}


	if (version.dwPlatformId == VER_PLATFORM_WIN32_NT) {

		//  this works under WinNT4 or Win2K or WinXP if you have any rights
		//printf ("\nTrying to read the drive IDs using physical access with zero rights\n");
		memset(sBuff,0,nLen);
		nError = MY_ReadPhysicalDriveInNTWithZeroRights(sBuff,nLen);
		nSNSize = strlen(sBuff);
		//TracklogU(_T("MY_ReadPhysicalDriveInNTWithZeroRights : %s"),sBuff);


		//  this works under WinNT4 or Win2K if you have admin rights
		//printf ("\nTrying to read the drive IDs using physical access with admin rights\n");
		if ( nError!=0 || nSNSize<6 ){ 
			memset(sBuff,0,nLen);
			nError = MY_ReadPhysicalDriveInNTWithAdminRights(sBuff,nLen);
			nSNSize = strlen(sBuff);
			//TracklogU(_T("MY_ReadPhysicalDriveInNTWithAdminRights : %s"),sBuff);
		}


		//  this works under WinNT4 or Win2K or WinXP or Windows Server 2003 or Vista if you have any rights
		//printf ("\nTrying to read the drive IDs using Smart\n");
		if ( nError!=0 || nSNSize<6 ){ 
			memset(sBuff,0,nLen);
			nError = MY_ReadPhysicalDriveInNTUsingSmart(sBuff,nLen);
			nSNSize = strlen(sBuff);
			//TracklogU(_T("MY_ReadPhysicalDriveInNTUsingSmart : %s"),sBuff);
		}


		//  this should work in WinNT or Win2K if previous did not work
		//  this is kind of a backdoor via the SCSI mini port driver into
		//     the IDE drives
		//printf ("\nTrying to read the drive IDs using the SCSI back door\n");
		if ( nError!=0 || nSNSize<6 ){ 
			memset(sBuff,0,nLen);
			nError = MY_ReadIdeDriveAsScsiDriveInNT(sBuff,nLen);
			nSNSize = strlen(sBuff);
			//TracklogU(_T("MY_ReadIdeDriveAsScsiDriveInNT : %s"),sBuff);
		}

	}

	return nError;
}

CStringA FormatHexStr(const char *sMem,int nLen)
{
	CStringA strText;
	CHAR tzText[MAX_PATH]={0};

	if ( sMem==NULL || nLen==0 ){
		return strText;
	}

	if ( nLen<0 ){
		nLen = strlen(sMem);
	}

	for ( int i=0; i<nLen; ++i ){
		sprintf_s(tzText,MAX_PATH, "%02X", (unsigned char)sMem[i]);
		strText += tzText;
	}//endfor

	return strText;
}

BOOL IsAllNumber(const CString&str)
{
	BOOL bYes = TRUE;
	int nLen = str.GetLength();
	for ( int i = 0; i < nLen; ++i ) {
		if ( iswdigit(str.GetAt(i))==0 ) {
			bYes = FALSE;
			break;
		}
	}//endfor

	return bYes;
}

CString FilterDiskId(const CString&strDiskId)
{
	CString str = strDiskId;
	CString strTemp;

	if ( str.GetLength()>=DISKID_MAXLEN ) {
		str = str.Left(DISKID_MAXLEN-1);
	}

	if ( str.GetLength()<6 || IsAllNumber(str)==TRUE ){
		//太短丢弃，不可信
		str.Empty();
	}

	strTemp = str;

	try{
		//指定以下字符串在多字节版本下会异常
		//strTemp = _T("\xFC\xA1\xEC\x24\x25\x26\x7B\x7D\x28\x29\x5B\x5D\x2E\x3F\x3F\xA8\xB9\x3F");
		strTemp.MakeLower();
	}catch (...){
		//str = Star::Common::Str2HexStr(str, str.GetLength());
		//strTemp = str;
		strTemp.Empty();
	}

	if ( strTemp.Find(_T("unknow"))!=-1 || strTemp.Find(_T("oem"))!=-1 || strTemp.Find(_T("o.e.m"))!=-1 || strTemp.Find(_T("invalid"))!=-1 || strTemp.Find(_T("diy"))!=-1 ) {
		str.Empty();
	}

	return str;
}

//获取硬盘序列号
CString GetDiskId()
{
	char szDiskId[MAX_PATH] = {0};
	int nError = 0;
	CString strTemp;
	nError = GetHardDriveSerialNumber(szDiskId,MAX_PATH);
	strTemp = szDiskId;
	strTemp.Trim();

	//strTemp=_T("000000000000000000000000000000000000000ddddd");
	//太长则截断
	strTemp = FilterDiskId(strTemp);
	strTemp.MakeUpper();
	return strTemp;
}
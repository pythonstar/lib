/* pelib.cpp --

   This file is part of the "PE Maker".

   Copyright (C) 2005-2006 Ashkbiz Danehkar
   All Rights Reserved.

   "PE Maker" library are free software; you can redistribute them
   and/or modify them under the terms of the GNU General Public License as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYRIGHT.TXT.
   If not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   yodap's Forum:
   http://yodapforum.has.it/

   yodap's Site:
   http://yodap.has.it
   http://yodap.cjb.net
   http://yodap.sourceforge.net

   Ashkbiz Danehkar
   <ashkbiz@yahoo.com>
*/

#include "stdafx.h"
#include "pelib.h"
#include <imagehlp.h>//#include <Dbghelp.h>
using namespace Star;

//================================================================
//----------------------------------------------------------------
Star::CPELibrary::CPELibrary()
{
	image_dos_header=new (IMAGE_DOS_HEADER);
	dwDosStubSize=0;
	image_nt_headers=new (IMAGE_NT_HEADERS);
	for(int i=0;i<MAX_SECTION_NUM;i++) image_section_header[i]=new (IMAGE_SECTION_HEADER);
	image_tls_directory=NULL;
}
//----------------------------------------------------------------
Star::CPELibrary::~CPELibrary()
{
	delete []image_dos_header;
	dwDosStubSize=0;
	delete []image_nt_headers;
	for(int i=0;i<MAX_SECTION_NUM;i++) delete []image_section_header[i];
	if(image_tls_directory!=NULL) delete image_tls_directory;
}
//================================================================
//----------------------------------------------------------------
// returns aligned value
DWORD Star::CPELibrary::PEAlign(DWORD dwTarNum,DWORD dwAlignTo)
{	
	return(((dwTarNum+dwAlignTo-1)/dwAlignTo)*dwAlignTo);
}
//----------------------------------------------------------------
void Star::CPELibrary::AlignmentSections()
{
	int i;
	for(i=0;i<image_nt_headers->FileHeader.NumberOfSections;i++)
	{
		image_section_header[i]->VirtualAddress=
			PEAlign(image_section_header[i]->VirtualAddress,
			image_nt_headers->OptionalHeader.SectionAlignment);

		image_section_header[i]->Misc.VirtualSize=
			PEAlign(image_section_header[i]->Misc.VirtualSize,
			image_nt_headers->OptionalHeader.SectionAlignment);

		image_section_header[i]->PointerToRawData=
			PEAlign(image_section_header[i]->PointerToRawData,
			image_nt_headers->OptionalHeader.FileAlignment);

		image_section_header[i]->SizeOfRawData=
			PEAlign(image_section_header[i]->SizeOfRawData,
			image_nt_headers->OptionalHeader.FileAlignment);
	}
	image_nt_headers->OptionalHeader.SizeOfImage=image_section_header[i-1]->VirtualAddress+
		image_section_header[i-1]->Misc.VirtualSize;
	image_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG].VirtualAddress=0;
	image_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG].Size=0;
	image_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress=0;
	image_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].Size=0;
	image_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress=0;
	image_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].Size=0;
}
//================================================================
//----------------------------------------------------------------
// calulates the Offset from a RVA
// Base    - base of the MMF
// dwRVA - the RVA to calculate
// returns 0 if an error occurred else the calculated Offset will be returned
DWORD Star::CPELibrary::RVA2Offset(DWORD dwRVA)
{
	DWORD _offset;
	PIMAGE_SECTION_HEADER section;
	section=ImageRVA2Section(dwRVA);//ImageRvaToSection(pimage_nt_headers,Base,dwRVA);
	if(section==NULL)
	{
		return(0);
	}
	_offset=dwRVA+section->PointerToRawData-section->VirtualAddress;
	return(_offset);
}
//----------------------------------------------------------------
// calulates the RVA from a Offset
// Base    - base of the MMF
// dwRO - the Offset to calculate
// returns 0 if an error occurred else the calculated Offset will be returned
DWORD Star::CPELibrary::Offset2RVA(DWORD dwRO)
{
	PIMAGE_SECTION_HEADER section;
	section=ImageOffset2Section(dwRO);
	if(section==NULL)
	{
		return(0);
	}
	return(dwRO+section->VirtualAddress-section->PointerToRawData);
}
//================================================================
//----------------------------------------------------------------
PIMAGE_SECTION_HEADER Star::CPELibrary::ImageRVA2Section(DWORD dwRVA)
{
	int i;
	for(i=0;i<image_nt_headers->FileHeader.NumberOfSections;i++)
	{
		if((dwRVA>=image_section_header[i]->VirtualAddress) && (dwRVA<=(image_section_header[i]->VirtualAddress+image_section_header[i]->SizeOfRawData)))
		{
			return ((PIMAGE_SECTION_HEADER)image_section_header[i]);
		}
	}
	return(NULL);
}

//----------------------------------------------------------------
//The ImageOffset2Section function locates a Off Set address (RO) 
//within the image header of a file that is mapped as a file and
//returns a pointer to the section table entry for that virtual 
//address.
PIMAGE_SECTION_HEADER Star::CPELibrary::ImageOffset2Section(DWORD dwRO)
{
	for(int i=0;i<image_nt_headers->FileHeader.NumberOfSections;i++)
	{
		if((dwRO>=image_section_header[i]->PointerToRawData) && (dwRO<(image_section_header[i]->PointerToRawData+image_section_header[i]->SizeOfRawData)))
		{
			return ((PIMAGE_SECTION_HEADER)image_section_header[i]);
		}
	}
	return(NULL);
}
//================================================================
//----------------------------------------------------------------
// retrieve Enrty Point Section Number
// Base    - base of the MMF
// dwRVA - the RVA to calculate
// returns -1 if an error occurred else the calculated Offset will be returned
DWORD Star::CPELibrary::ImageOffset2SectionNum(DWORD dwRO)
{
	for(int i=0;i<image_nt_headers->FileHeader.NumberOfSections;i++)
	{
		if((dwRO>=image_section_header[i]->PointerToRawData) && (dwRO<(image_section_header[i]->PointerToRawData+image_section_header[i]->SizeOfRawData)))
		{
			return (i);
		}
	}
	return(-1);
}
//----------------------------------------------------------------
PIMAGE_SECTION_HEADER Star::CPELibrary::AddNewSection(char* szName,DWORD dwSize)
{
	DWORD roffset,rsize,voffset,vsize;
	int i=image_nt_headers->FileHeader.NumberOfSections;
	rsize=PEAlign(dwSize,
				image_nt_headers->OptionalHeader.FileAlignment);
	vsize=PEAlign(rsize,
				image_nt_headers->OptionalHeader.SectionAlignment);
	roffset=PEAlign(image_section_header[i-1]->PointerToRawData+image_section_header[i-1]->SizeOfRawData,
				image_nt_headers->OptionalHeader.FileAlignment);
	voffset=PEAlign(image_section_header[i-1]->VirtualAddress+image_section_header[i-1]->Misc.VirtualSize,
				image_nt_headers->OptionalHeader.SectionAlignment);
	memset(image_section_header[i],0,(size_t)sizeof(IMAGE_SECTION_HEADER));
	image_section_header[i]->PointerToRawData=roffset;
	image_section_header[i]->VirtualAddress=voffset;
	image_section_header[i]->SizeOfRawData=rsize;
	image_section_header[i]->Misc.VirtualSize=vsize;
	image_section_header[i]->Characteristics=0xC0000040;
	memcpy(image_section_header[i]->Name,szName,(size_t)strlen(szName));
	image_section[i]=(char*)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,rsize);
	image_nt_headers->FileHeader.NumberOfSections++;
	return (PIMAGE_SECTION_HEADER)image_section_header[i];
}
//================================================================
//----------------------------------------------------------------
void Star::CPELibrary::OpenFile(char* FileName)
{
	DWORD	dwBytesRead		= 0;
	HANDLE	hFile= NULL;
	DWORD SectionNum;
	DWORD i;
	DWORD dwRO_first_section;
	pMem=NULL;
	//----------------------------------------
	hFile=CreateFile(FileName,
					 GENERIC_READ,
					 FILE_SHARE_WRITE | FILE_SHARE_READ,
	                 NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile==INVALID_HANDLE_VALUE)
	{
		ShowErr(FileErr);
		return;
	}
	dwFileSize=GetFileSize(hFile,0);
	if(dwFileSize == 0)
	{
		CloseHandle(hFile);
		ShowErr(FsizeErr);
		return;
	}
	pMem=(char*)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,dwFileSize);
	if(pMem == NULL)
	{
		CloseHandle(hFile);
		ShowErr(MemErr);
		return;
	}
	ReadFile(hFile,pMem,dwFileSize,&dwBytesRead,NULL);
	CloseHandle(hFile);
	//----------------------------------------
	memcpy(image_dos_header,pMem,sizeof(IMAGE_DOS_HEADER));
	dwDosStubSize=image_dos_header->e_lfanew-sizeof(IMAGE_DOS_HEADER);
	dwDosStubOffset=sizeof(IMAGE_DOS_HEADER);
	pDosStub=new CHAR[dwDosStubSize];
	if((dwDosStubSize&0x80000000)==0x00000000)
	{
		CopyMemory(pDosStub,pMem+dwDosStubOffset,dwDosStubSize);
	}
	memcpy(image_nt_headers,
		       pMem+image_dos_header->e_lfanew,
			   sizeof(IMAGE_NT_HEADERS));
	dwRO_first_section=image_dos_header->e_lfanew+sizeof(IMAGE_NT_HEADERS);
	if(image_dos_header->e_magic!=IMAGE_DOS_SIGNATURE)// MZ
	{
		ShowErr(PEErr);
		GlobalFree(pMem);
		return;
	}
	if(image_nt_headers->Signature!=IMAGE_NT_SIGNATURE)// PE00
	{
		ShowErr(PEErr);
		GlobalFree(pMem);
		return;
	}
	//----------------------------------------
	SectionNum=image_nt_headers->FileHeader.NumberOfSections;
	//----------------------------------------
	for( i=0;i<SectionNum;i++) 
	{
		CopyMemory(image_section_header[i],pMem+dwRO_first_section+i*sizeof(IMAGE_SECTION_HEADER),
			sizeof(IMAGE_SECTION_HEADER));
	}
	//----------------------------------------
	for(i=0;i<SectionNum;i++)
	{
			image_section[i]=(char*)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,
				PEAlign(image_section_header[i]->SizeOfRawData,
				image_nt_headers->OptionalHeader.FileAlignment));

			CopyMemory(image_section[i],
					pMem+image_section_header[i]->PointerToRawData,
					image_section_header[i]->SizeOfRawData);
	}
	//----------------------------------------
	if(image_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress!=0)
	{
		image_tls_directory = new (IMAGE_TLS_DIRECTORY32);
		DWORD dwOffset=RVA2Offset(image_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);
		memcpy(image_tls_directory,
				pMem+dwOffset,
				sizeof(IMAGE_TLS_DIRECTORY32));		
	}
	//----------------------------------------
	GlobalFree(pMem);
}
//----------------------------------------------------------------
void Star::CPELibrary::SaveFile(char* FileName)
{
	DWORD	dwBytesWritten	= 0;
	DWORD i;
	DWORD dwRO_first_section;
	DWORD SectionNum;
	HANDLE	hFile= NULL;
	pMem=NULL;
	//----------------------------------------
	hFile=CreateFile(FileName,
					 GENERIC_WRITE,
					 FILE_SHARE_WRITE | FILE_SHARE_READ,
	                 NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile==INVALID_HANDLE_VALUE)
	{
		hFile=CreateFile(FileName,
					 GENERIC_WRITE,
					 FILE_SHARE_WRITE | FILE_SHARE_READ,
	                 NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
		if(hFile==INVALID_HANDLE_VALUE)
		{
			ShowErr(FileErr);
			return;
		}
	}
	//----------------------------------------
	AlignmentSections();
	//----------------------------------------
	i=image_nt_headers->FileHeader.NumberOfSections;
	dwFileSize=image_section_header[i-1]->PointerToRawData+
		image_section_header[i-1]->SizeOfRawData;

	pMem=(char*)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,dwFileSize);
	if(pMem == NULL)
	{
		CloseHandle(hFile);
		ShowErr(MemErr);
		return;
	}
	//----------------------------------------
	memcpy(pMem,image_dos_header,sizeof(IMAGE_DOS_HEADER));
	if((dwDosStubSize&0x80000000)==0x00000000)
	{
		memcpy(pMem+dwDosStubOffset,pDosStub,dwDosStubSize);
	}
	memcpy(pMem+image_dos_header->e_lfanew,
		image_nt_headers,
		sizeof(IMAGE_NT_HEADERS));

	dwRO_first_section=image_dos_header->e_lfanew+sizeof(IMAGE_NT_HEADERS);

	SectionNum=image_nt_headers->FileHeader.NumberOfSections;
	//----------------------------------------
	for( i=0;i<SectionNum;i++) 
	{
		CopyMemory(pMem+dwRO_first_section+i*sizeof(IMAGE_SECTION_HEADER),
			image_section_header[i],
			sizeof(IMAGE_SECTION_HEADER));
	}
	//----------------------------------------
	for(i=0;i<SectionNum;i++)
	{
		CopyMemory(pMem+image_section_header[i]->PointerToRawData,
				image_section[i],
				image_section_header[i]->SizeOfRawData);
	}
	// ----- WRITE FILE MEMORY TO DISK -----
	SetFilePointer(hFile,0,NULL,FILE_BEGIN);
	WriteFile(hFile,pMem,dwFileSize,&dwBytesWritten,NULL);
	
	// ------ FORCE CALCULATED FILE SIZE ------
	SetFilePointer(hFile,dwFileSize,NULL,FILE_BEGIN);
	SetEndOfFile(hFile);
	CloseHandle(hFile);
	//----------------------------------------
	GlobalFree(pMem);
}
//----------------------------------------------------------------
//--------------------------------------------
const CHAR* CPELibrary::szErrorMsg[]=
{	"",															// Non-Error
"File access error :(",										// MemErr
"Invalid PE file!",											// PEErr
"Not enough memory :(",										// FileErr
"Files with a filesize of 0 aren't allowed!",				// NoRoom4SectionErr
"There's no room for a new section :(",						// FsizeErr
"Too many sections!",										// SecNumErr
"Too much ImageImportDescriptors!",							// IIDErr	
"File already was protected!",								// FileISProtect
"Invalid PE file! It might be protected by another tool.",	// PEnotValid	
"This Version does not support COM Runtime structure.",		// PEisCOMRuntime
"This Version does not support dynamic link library.",		// DLLnotSupport
"This Version does not support windows driver model.",		// WDMnotSupport
"This Version does not support terminal server aware.",		// TServernotSupport
"This Version does not support system file.",				// SYSnotSupport
"No SE handler resides in this PE.",						// NOSEHnotSupport	
"Can not support PE file with no bind.",					// NOBINDnotSupport
"Section's Name is not recognized :("						// PackSectionName	
};
//--------------------------------------------
//----- ERROR MESSAGES ----
//The ShowErr display message by receiving its Error Number
void Star::CPELibrary::ShowErr(unsigned char numErr)
{
	::MessageBox(NULL,szErrorMsg[numErr],"´íÎó",MB_OK | MB_ICONERROR );
}
//================================================================

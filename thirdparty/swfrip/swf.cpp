#include <sys/stat.h>
#include <windows.h>
#include "swf.h"
//#include <zconf.h>
//#include <zlib.h>

UI32 CSWF::AddTag(void *lpvTag)
{
	if(!lpvTag) return 0;

	if(IsCharacter(lpvTag))
	{
		UI16 usCharacterID=((UNKNOWNTAG*)lpvTag)->usCharacterID;
		if(!usCharacterID || lprgvpCharacters[usCharacterID]) return 0;
		lprgvpCharacters[usCharacterID]=lpvTag;
	}

	lprgvpTags[nTags]=lpvTag; nTags++;
	((UNKNOWNTAG*)lpvTag)->rhTagHeader.usTagCode;
		
	if(((UNKNOWNTAG*)lpvTag)->rhTagHeader.usTagCode==TAGCODE_JPEGTABLES)
		lpjtJPEGTables=(JPEGTABLES*)lpvTag;

	return nTags;
}

UI32 CSWF::CreateFileName(char *szFileName, char *szSubDir, UI32 ulID, char *szExt)
{
	strcpy(szFileName,szSWFName);
	sprintf(BaseName(szFileName),"%s",BaseName(szSWFName));
	sprintf(strrchr(szFileName,'.'),"\\%s\\%s",szSubDir,BaseName(szSWFName));
	sprintf(strrchr(szFileName,'.'),"_%s_%05lu.%s",szSubDir,ulID,szExt);
	strcpy(szLastRes,szFileName);
	
	return MakeDir(szFileName);
	//return mkdir(szFileName);
}

UI32 CSWF::MakeDir(char *szFileName)
{
	char szPath[4096];

	strcpy(szPath,szFileName);
	*strrchr(szPath,'\\')='\0';

	if(szPath[strlen(szPath)-1]==':') return 0;

	MakeDir(szPath);

	return CreateDirectory(szPath,NULL);
}

UI32 CSWF::ReadUnknownTag(UNKNOWNTAG *lputUnknown)
{
	memset(lputUnknown,0,sizeof(UNKNOWNTAG));
	memcpy(&lputUnknown->rhTagHeader,&rhTagHeader,sizeof(RECORDHEADER));

	if(lputUnknown->rhTagHeader.ulLength)
	{
		lputUnknown->prgbData=new UI8[lputUnknown->rhTagHeader.ulLength];
		bsInFileStream.ReadBytes(lputUnknown->prgbData,lputUnknown->rhTagHeader.ulLength);
		lputUnknown->usCharacterID=*((UI16*)lputUnknown->prgbData);
	}

	return lputUnknown->rhTagHeader.ulLength;
}

UI32 CSWF::WriteUnknownTag(UNKNOWNTAG *lputUnknown, FILE *pfileSWF)
{
	WriteTagHeader(&lputUnknown->rhTagHeader,pfileSWF);
	
	if(lputUnknown->rhTagHeader.ulLength)
		fwrite(lputUnknown->prgbData,1,lputUnknown->rhTagHeader.ulLength,pfileSWF);
	
	return lputUnknown->rhTagHeader.ulLength;
}

UI16 CSWF::IsCharacter(void *lpvUnknownTag)
{
	switch(((UNKNOWNTAG*)lpvUnknownTag)->rhTagHeader.usTagCode)
	{
		case TAGCODE_DEFINEBITS:
		case TAGCODE_DEFINEBITSJPEG2:
		case TAGCODE_DEFINEBITSJPEG3:
		case TAGCODE_DEFINEBITSLOSSLESS:
		case TAGCODE_DEFINEBITSLOSSLESS2:
		case TAGCODE_DEFINEBUTTON:
		case TAGCODE_DEFINEBUTTON2:
		case TAGCODE_DEFINESOUND:
		case TAGCODE_DEFINESHAPE:
		case TAGCODE_DEFINESHAPE2:
		case TAGCODE_DEFINESHAPE3:
		case TAGCODE_DEFINESPRITE:			
		case TAGCODE_DEFINEFONT:			
		case TAGCODE_DEFINEFONT2:			
		case TAGCODE_DEFINETEXT:
		case TAGCODE_DEFINETEXT2:
		case TAGCODE_DEFINEEDITTEXT:
		case TAGCODE_DEFINEMORPHSHAPE:
		case TAGCODE_DEFINEVIDEOSTREAM:
		return ((UNKNOWNTAG*)lpvUnknownTag)->usCharacterID; 
		break;
	}

	return 0;
}

int CSWF::GetCharacterType(UI16 usCharacterID)
{
	if(!lprgvpCharacters[usCharacterID]) return -1;

	switch(((UNKNOWNTAG*)lprgvpCharacters[usCharacterID])->rhTagHeader.usTagCode)
	{
		case TAGCODE_DEFINEBITS:
		case TAGCODE_DEFINEBITSJPEG2:
		case TAGCODE_DEFINEBITSJPEG3:
		case TAGCODE_DEFINEBITSLOSSLESS:
		case TAGCODE_DEFINEBITSLOSSLESS2:	return SWF_IMAGE;
		case TAGCODE_DEFINEBUTTON:
		case TAGCODE_DEFINEBUTTON2:			return SWF_BUTTON;
		case TAGCODE_DEFINESOUND: 			return SWF_SOUND;
		case TAGCODE_DEFINESHAPE:
		case TAGCODE_DEFINESHAPE2:
		case TAGCODE_DEFINESHAPE3:			return SWF_SHAPE;
		case TAGCODE_DEFINESPRITE:			return SWF_SPRITE;
		case TAGCODE_DEFINEFONT:			
		case TAGCODE_DEFINEFONT2:			return SWF_FONT;
		case TAGCODE_DEFINETEXT:
		case TAGCODE_DEFINETEXT2:
		case TAGCODE_DEFINEEDITTEXT:		return SWF_TEXT;
		case TAGCODE_DEFINEMORPHSHAPE:		return SWF_MORPH;
		case TAGCODE_DEFINEVIDEOSTREAM:		return SWF_VIDEO;
	}

	return -1;
}

UI32 CSWF::WriteCharacter(UI16 usCharacterID, FILE *pfileSWF)
{
	if(usCharacterID>nTags) return 0;
	if(!lprgvpCharacters[usCharacterID]) return 0;
	
	switch(((DEFINEBITS*)lprgvpCharacters[usCharacterID])->rhTagHeader.usTagCode)
	{
		case TAGCODE_DEFINESHAPE:
		case TAGCODE_DEFINESHAPE2:
			WriteDefineShape((DEFINESHAPE*)lprgvpCharacters[usCharacterID],false,pfileSWF);
			break;

		case TAGCODE_DEFINESHAPE3:
			WriteDefineShape((DEFINESHAPE*)lprgvpCharacters[usCharacterID],true,pfileSWF);
			break;

		case TAGCODE_DEFINEMORPHSHAPE:
			WriteDefineMorphShape((DEFINEMORPHSHAPE*)lprgvpCharacters[usCharacterID],pfileSWF);
			break;

		case TAGCODE_DEFINEBITS:
			WriteDefineBits((DEFINEBITS*)lprgvpCharacters[usCharacterID],pfileSWF);
			break;
										
		case TAGCODE_DEFINEBITSJPEG2:
			WriteDefineBitsJPEG2((DEFINEBITSJPEG2*)lprgvpCharacters[usCharacterID],pfileSWF);
			break;

		case TAGCODE_DEFINEBITSJPEG3:
			WriteDefineBitsJPEG3((DEFINEBITSJPEG3*)lprgvpCharacters[usCharacterID],pfileSWF);
			break;

		case TAGCODE_DEFINEBITSLOSSLESS:
			WriteDefineBitsLossless((DEFINEBITSLOSSLESS*)lprgvpCharacters[usCharacterID],pfileSWF);
			break;

		case TAGCODE_DEFINEBITSLOSSLESS2:
			WriteDefineBitsLossless((DEFINEBITSLOSSLESS*)lprgvpCharacters[usCharacterID],pfileSWF);
			break;

		case TAGCODE_DEFINEBUTTON:
			WriteDefineButton((DEFINEBUTTON*)lprgvpCharacters[usCharacterID],pfileSWF);
			break;

		case TAGCODE_DEFINEBUTTON2:
			WriteDefineButton2((DEFINEBUTTON2*)lprgvpCharacters[usCharacterID],pfileSWF);
			break;

		case TAGCODE_DEFINEFONT:
			WriteDefineFont((DEFINEFONT*)lprgvpCharacters[usCharacterID],pfileSWF);
			break;

		case TAGCODE_DEFINEFONT2:
			WriteDefineFont2((DEFINEFONT2*)lprgvpCharacters[usCharacterID],pfileSWF);
			break;

		case TAGCODE_DEFINESOUND:
			WriteDefineSound((DEFINESOUND*)lprgvpCharacters[usCharacterID],pfileSWF);
			break;

		case TAGCODE_DEFINESPRITE:
			WriteDefineSprite((DEFINESPRITE*)lprgvpCharacters[usCharacterID],pfileSWF);
			break;

		case TAGCODE_DEFINETEXT:
			WriteDefineText((DEFINETEXT*)lprgvpCharacters[usCharacterID],false,pfileSWF);
			break;

		case TAGCODE_DEFINETEXT2:
			WriteDefineText((DEFINETEXT*)lprgvpCharacters[usCharacterID],true,pfileSWF);
			break;

		case TAGCODE_DEFINEVIDEOSTREAM:
			WriteDefineVideoStream((DEFINEVIDEOSTREAM*)lprgvpCharacters[usCharacterID],pfileSWF);
			break;

		case TAGCODE_DEFINEEDITTEXT:
			WriteDefineEditText((DEFINEEDITTEXT*)lprgvpCharacters[usCharacterID],pfileSWF);
			break;

		default:
			WriteUnknownTag((UNKNOWNTAG*)lprgvpCharacters[usCharacterID],pfileSWF);
		}
		
	return 1;
}

UI32 CSWF::SaveCharacter(UI16 usCharacterID)
{
	if(!lprgvpCharacters[usCharacterID]) return 0;

	switch(((DEFINEBITS*)lprgvpCharacters[usCharacterID])->rhTagHeader.usTagCode)
	{
		case TAGCODE_DEFINESHAPE:
		case TAGCODE_DEFINESHAPE2:
		case TAGCODE_DEFINESHAPE3:
			switch(ubShapeFormat)
			{
				case 0:
					SaveDefineShape((DEFINESHAPE*)lprgvpCharacters[usCharacterID]);
					break;

				case 1:
					SaveDefineShapeAsSVG((DEFINESHAPE*)lprgvpCharacters[usCharacterID]);
					break;
			}
			break;

		case TAGCODE_DEFINEMORPHSHAPE:
			SaveDefineMorphShape((DEFINEMORPHSHAPE*)lprgvpCharacters[usCharacterID]);
			break;

		case TAGCODE_DEFINEBITS:
			SaveDefineBitsAsJPEG((DEFINEBITS*)lprgvpCharacters[usCharacterID]);
			break;
										
		case TAGCODE_DEFINEBITSJPEG2:
			SaveDefineBitsJPEG2AsJPEG((DEFINEBITSJPEG2*)lprgvpCharacters[usCharacterID]);
			break;

		case TAGCODE_DEFINEBITSJPEG3:
			SaveDefineBitsJPEG3AsJPEG((DEFINEBITSJPEG3*)lprgvpCharacters[usCharacterID]);
			break;

		case TAGCODE_DEFINEBITSLOSSLESS:
		case TAGCODE_DEFINEBITSLOSSLESS2:
			switch(ubImageFormat)
			{
				case 0:
					SaveDefineBitsLosslessAsFile((DEFINEBITSLOSSLESS*)lprgvpCharacters[usCharacterID],"bmp");
					break;
				case 1:
					SaveDefineBitsLosslessAsFile((DEFINEBITSLOSSLESS*)lprgvpCharacters[usCharacterID],"pcx");
					break;
				case 2:
					SaveDefineBitsLosslessAsFile((DEFINEBITSLOSSLESS*)lprgvpCharacters[usCharacterID],"png");
					break;
			}
			break;

		case TAGCODE_DEFINEBUTTON:
		case TAGCODE_DEFINEBUTTON2:
			SaveDefineButton((DEFINEBUTTON*)lprgvpCharacters[usCharacterID]);
			break;

		case TAGCODE_DEFINEFONT:
		case TAGCODE_DEFINEFONT2:
			SaveDefineFont(lprgvpCharacters[usCharacterID]);
			break;

		case TAGCODE_DEFINESOUND:
			SaveDefineSoundAsFile((DEFINESOUND*)lprgvpCharacters[usCharacterID]);
			break;

		case TAGCODE_DEFINESPRITE:
			SaveDefineSprite((DEFINESPRITE*)lprgvpCharacters[usCharacterID],0);
			break;

		case TAGCODE_DEFINETEXT:
		case TAGCODE_DEFINETEXT2:
			switch(ubTextFormat)
			{
				case 0: 
					SaveDefineTextAsSWF((DEFINETEXT*)lprgvpCharacters[usCharacterID]);
					break;

				case 1:
					SaveDefineTextAsTXT((DEFINETEXT*)lprgvpCharacters[usCharacterID]);
					break;
			}
			break;

		case TAGCODE_DEFINEVIDEOSTREAM:
			SaveDefineVideoStream((DEFINEVIDEOSTREAM*)lprgvpCharacters[usCharacterID]);
			break;
	}
	
	return 1;
}

UI32 CSWF::SaveAll(UI8 ubType)
{
	UI16 usTagCode, usFrame=0;

	for(UI32 iTag=0; iTag<nTags; iTag++) 
	{
		if(!lprgvpTags[iTag]) continue;

		usTagCode=((UNKNOWNTAG*)lprgvpTags[iTag])->rhTagHeader.usTagCode;

		switch(ubType)
		{
			case SWF_ALL: 
				SaveCharacter(((UNKNOWNTAG*)lprgvpTags[iTag])->usCharacterID);
				break;

			case SWF_SHAPE:
				if(usTagCode==TAGCODE_DEFINESHAPE || usTagCode==TAGCODE_DEFINESHAPE2 || usTagCode==TAGCODE_DEFINESHAPE3)
					SaveCharacter(((UNKNOWNTAG*)lprgvpTags[iTag])->usCharacterID);
				break;

			case SWF_IMAGE:
				if( usTagCode==TAGCODE_DEFINEBITS || 
					usTagCode==TAGCODE_DEFINEBITSJPEG2 ||
					usTagCode==TAGCODE_DEFINEBITSJPEG3 || 
					usTagCode==TAGCODE_DEFINEBITSLOSSLESS ||
					usTagCode==TAGCODE_DEFINEBITSLOSSLESS2)
					SaveCharacter(((UNKNOWNTAG*)lprgvpTags[iTag])->usCharacterID);
				break;

			case SWF_MORPH:
				if(usTagCode==TAGCODE_DEFINEMORPHSHAPE)
					SaveCharacter(((UNKNOWNTAG*)lprgvpTags[iTag])->usCharacterID);
				break;

			case SWF_FONT:
				if(usTagCode==TAGCODE_DEFINEFONT || usTagCode==TAGCODE_DEFINEFONT2)
					SaveCharacter(((UNKNOWNTAG*)lprgvpTags[iTag])->usCharacterID);
				break;

			case SWF_TEXT:
				if(usTagCode==TAGCODE_DEFINETEXT || usTagCode==TAGCODE_DEFINETEXT2)
					SaveCharacter(((UNKNOWNTAG*)lprgvpTags[iTag])->usCharacterID);
				break;

			case SWF_SOUND:
				if(usTagCode==TAGCODE_DEFINESOUND)
					SaveCharacter(((UNKNOWNTAG*)lprgvpTags[iTag])->usCharacterID);
				if(usTagCode==TAGCODE_DEFINESPRITE) 
					SaveSoundStream((DEFINESPRITE*)lprgvpTags[iTag]);
				break;

			case SWF_BUTTON:
				if(usTagCode==TAGCODE_DEFINEBUTTON || usTagCode==TAGCODE_DEFINEBUTTON2)
					SaveCharacter(((UNKNOWNTAG*)lprgvpTags[iTag])->usCharacterID);
				break;

			case SWF_SPRITE:
				if(usTagCode==TAGCODE_DEFINESPRITE)
					SaveCharacter(((UNKNOWNTAG*)lprgvpTags[iTag])->usCharacterID);
				break;

			case SWF_VIDEO:
				if(usTagCode==TAGCODE_DEFINEVIDEOSTREAM)
					SaveCharacter(((UNKNOWNTAG*)lprgvpTags[iTag])->usCharacterID);
				break;

			case SWF_FRAME:
				if(usTagCode==TAGCODE_SHOWFRAME)
					SaveFrame(0,++usFrame);
				break;

			case SWF_SCRIPT:
				if( usTagCode==TAGCODE_DEFINESPRITE ||
					usTagCode==TAGCODE_DEFINEBUTTON ||
					usTagCode==TAGCODE_DEFINEBUTTON2)
					SaveScript(((UNKNOWNTAG*)lprgvpTags[iTag])->usCharacterID);
				break;
		}
	}

	if(ubType==SWF_TEXT) SaveText();
	if(ubType==SWF_SOUND) SaveSoundStream(0);
	if(ubType==SWF_SCRIPT) SaveScript(0);
	return 0;
}

UI8 CSWF::GetExportFormat(UI8 ubCategory)
{
	switch(ubCategory)
	{
		case SWF_SHAPE:		return ubShapeFormat;
		case SWF_IMAGE:		return ubImageFormat;
		case SWF_MORPH:		return 0;
		case SWF_TEXT:		return ubTextFormat;
		case SWF_SOUND:		return 0;
		case SWF_BUTTON:	return 0;
		case SWF_SPRITE:	return 0;
		case SWF_VIDEO:		return 0;
		case SWF_FRAME:		return ubFrameFormat;
	}
	return 0;
}

UI8 CSWF::SetExportFormat(UI8 ubCategory, UI8 ubFormat)
{
	switch(ubCategory)
	{
		case SWF_SHAPE:		return ubShapeFormat=ubFormat;
		case SWF_IMAGE:		return ubImageFormat=ubFormat;
		case SWF_MORPH:		return 0;
		case SWF_TEXT:		return ubTextFormat=ubFormat;
		case SWF_SOUND:		return 0;
		case SWF_BUTTON:	return 0;
		case SWF_SPRITE:	return 0;
		case SWF_VIDEO:		return 0;
		case SWF_FRAME:		return ubFrameFormat=ubFormat;
	}
	return 0;
}

UI32 CSWF::GetTagArray(UI16 usClipID, void **&lprgvpTagArray, UI32 &ulNumTags)
{
	DEFINESPRITE *lpdsSprite;

	if(!usClipID)
	{
		lprgvpTagArray=lprgvpTags;
		ulNumTags=nTags;
	}

	else
	{
		lpdsSprite=(DEFINESPRITE*)lprgvpCharacters[usClipID];
		if(lpdsSprite->rhTagHeader.usTagCode!=TAGCODE_DEFINESPRITE) return 0;
		lprgvpTagArray=lpdsSprite->lprgvpControlTags;
		ulNumTags=lpdsSprite->ulNumControlTags;
	}

	return 1;
}

UI32 CSWF::HasSoundStream(UI16 usClipID)
{
	void **lprgvpTagArray;
	UI32 ulNumTags;

	if(!GetTagArray(usClipID,lprgvpTagArray,ulNumTags))
		return 0;

	for(UI32 iTag=0; iTag<ulNumTags; iTag++)
	{
		if(((UNKNOWNTAG*)lprgvpTagArray[iTag])->rhTagHeader.usTagCode==TAGCODE_SOUNDSTREAMBLOCK)
			return 1;
	}

	return 0;
}

UI32 CSWF::HasActions(UI16 usCharacterID)
{
	void **lprgvpTagArray;
	UI32 ulNumTags;
	UI16 usTagCode;

	if(!GetTagArray(usCharacterID,lprgvpTagArray,ulNumTags))
	{
		usTagCode=((UNKNOWNTAG*)lprgvpCharacters[usCharacterID])->rhTagHeader.usTagCode;
		if(usTagCode==TAGCODE_DEFINEBUTTON)
			return ((DEFINEBUTTON*)lprgvpCharacters[usCharacterID])->aaActionArray.ulNumActions;
		if(usTagCode==TAGCODE_DEFINEBUTTON2)
			return ((DEFINEBUTTON2*)lprgvpCharacters[usCharacterID])->ulNumActions;
		return 0;
	}

	else for(UI32 iTag=0; iTag<ulNumTags; iTag++)
	{
		usTagCode=((UNKNOWNTAG*)lprgvpTagArray[iTag])->rhTagHeader.usTagCode;
		
		if(usTagCode==TAGCODE_DOACTION || usTagCode==TAGCODE_DOINITACTION)
			return 1;

		if(usTagCode==TAGCODE_PLACEOBJECT2)
			((PLACEOBJECT2*)lprgvpTagArray[iTag])->caClipActions.ulNumClipActionRecords;
	}

	return 0;
}
/*
UI32 CSWF::LoadZLIB()
{
	HMODULE hzlib=LoadLibrary("zlib1.dll");
	
	if(!hzlib) return 0;

	compress=(compress_type)GetProcAddress(hzlib,"compress");
	uncompress=(uncompress_type)GetProcAddress(hzlib,"uncompress");

	return 1;
}
*/
UI32 CSWF::Load(const char *szFileName)
{
	FILE *pfileSWF;
	UI8 *prgbData;
	UI32 ulDataLength;
	char szLogName[MAX_FILEPATH];

	/*Open SWF File & Read File Data*/
	if(!(pfileSWF=fopen(szFileName,"rb"))) return 0;

	Clear();
	
	/*Copy filename*/
	szSWFName=new char[strlen(szFileName)+1];
	strcpy(szSWFName, szFileName);

	/*Read FILEHEADER*/
	fread(&fhFileHeader,1,sizeof(SWFFILEHEADER),pfileSWF);
	ulDataLength=fhFileHeader.ulFileLength-sizeof(SWFFILEHEADER);

	if(fhFileHeader.bSignature2!='W' || fhFileHeader.bSignature3!='S') return 0;
	
	prgbData=new UI8[ulDataLength];
	
	/*File is compressed*/
	if(fhFileHeader.bSignature1=='C')
	{
		UI32 dwCompDataLength;
		UI8 *prgbCompData;
		
		//if(!fZLIB) return 0;

		/*Read compressed data*/
		fseek(pfileSWF,0,SEEK_END);
		dwCompDataLength=ftell(pfileSWF)-sizeof(SWFFILEHEADER);
		fseek(pfileSWF,sizeof(SWFFILEHEADER),SEEK_SET);
		prgbCompData=new UI8[dwCompDataLength];
		fread(prgbCompData,1,dwCompDataLength,pfileSWF);

		/*Uncompress data*/
		uncompress(prgbData,&ulDataLength,prgbCompData,dwCompDataLength);
		delete prgbCompData;

		if(ulDataLength!=fhFileHeader.ulFileLength-sizeof(SWFFILEHEADER))
		{
			delete prgbData;
			Clear();
			return 0;
		}		
	}
		
	/*File is not compressed*/
	else fread(prgbData,1,ulDataLength,pfileSWF);
	fclose(pfileSWF);

	bsInFileStream.Create(ulDataLength,prgbData);
	delete prgbData;
	
	/*Read 2nd Header*/
	ReadFrameHeader(&fhFrameHeader);
		
	/*parse log*/
	strcpy(szLogName,szSWFName);
	strcpy(strrchr(szLogName,'.'),"\\info.txt");
	MakeDir(szLogName);
	pfileInfo=fopen(szLogName,"w");

	/*action log*/
	strcpy(strrchr(szLogName,'\\'),"\\actions.txt");
	pfileActions=fopen(szLogName,"w");

	nTags=ReadTags(lprgvpTags,&nFrames,lprgflpLabels,&usNumLabels);

	fclose(pfileInfo);
	fclose(pfileActions);

	bsInFileStream.Clear();

	fLoaded=true;
	return nTags;
}

UI32 CSWF::GetInfo(SWFINFO *lpswfiInfo)
{
	lpswfiInfo->bVersion=fhFileHeader.bVersion;
	
	if(fhFileHeader.bSignature1=='C') lpswfiInfo->bCompressed=1;
	else lpswfiInfo->bCompressed=0;

	if(lpbcBackground)
	{
		lpswfiInfo->rgbaBackground.ubRed=lpbcBackground->rgbBackgroundColor.ubRed,
		lpswfiInfo->rgbaBackground.ubGreen=lpbcBackground->rgbBackgroundColor.ubGreen,
		lpswfiInfo->rgbaBackground.ubBlue=lpbcBackground->rgbBackgroundColor.ubBlue;
	}

	lpswfiInfo->ulFileLength=fhFileHeader.ulFileLength;
	lpswfiInfo->srFrameRect.slXMin=fhFrameHeader.srFrameSize.slXMin/20;
	lpswfiInfo->srFrameRect.slXMax=fhFrameHeader.srFrameSize.slXMax/20;
	lpswfiInfo->srFrameRect.slYMin=fhFrameHeader.srFrameSize.slYMin/20;
	lpswfiInfo->srFrameRect.slYMax=fhFrameHeader.srFrameSize.slYMax/20;
	lpswfiInfo->spfFrameRate=FP16TOFLOAT(fhFrameHeader.sfpFrameRate);
	lpswfiInfo->usFrameCount=fhFrameHeader.usFrameCount;
	lpswfiInfo->ulNumTags=nTags;

	return 0;
}

UI32 CSWF::SetInfo(SWFINFO *lpswfiInfo)
{
	//fhFileHeader.bVersion=lpswfiInfo->bVersion;
	
	//if(lpswfiInfo->bCompressed=1) fhFileHeader.bSignature1=='C';
	//else fhFileHeader.bSignature1=='F';

	//fhFileHeader.ulFileLength=lpswfiInfo->ulFileLength;
	
	if(lpbcBackground)
	{
		lpbcBackground->rgbBackgroundColor.ubRed=lpswfiInfo->rgbaBackground.ubRed;
		lpbcBackground->rgbBackgroundColor.ubGreen=lpswfiInfo->rgbaBackground.ubGreen;
		lpbcBackground->rgbBackgroundColor.ubBlue=lpswfiInfo->rgbaBackground.ubBlue;
	}

	fhFrameHeader.srFrameSize.slXMin=lpswfiInfo->srFrameRect.slXMin*20;
	fhFrameHeader.srFrameSize.slXMax=lpswfiInfo->srFrameRect.slXMax*20;
	fhFrameHeader.srFrameSize.slYMin=lpswfiInfo->srFrameRect.slYMin*20;
	fhFrameHeader.srFrameSize.slYMax=lpswfiInfo->srFrameRect.slYMax*20;
	//fhFrameHeader.sfpFrameRate=FLOATTOFP32(lpswfiInfo->spfFrameRate);

	//fhFrameHeader.usFrameCount=lpswfiInfo->usFrameCount;
	//nTags=lpswfiInfo->ulNumTags;

	return 0;
}

UI32 CSWF::GetTagInfo(SI32 usClipID, UI32 ulTag, TAGINFO *lptiInfo)
{
	void **lprgvpTagArray;
	UI32 ulNumTags;
	UI16 usCharacter=0;
	char szTemp[1024];
	UNKNOWNTAG* lputTag;
	DEFINESPRITE *lpdsSprite;
	FRAMELABEL *lpflLabel;

	if(!lptiInfo) return 0;

	memset(lptiInfo,0,sizeof(TAGINFO));

	if(usClipID==-1) 
	{
		if(ulTag>0xFFFF) return 0;
		lputTag=(UNKNOWNTAG*)lprgvpCharacters[ulTag];
		if(!lputTag) return 0;
	}
	
	else
	{
		if(!GetTagArray(usClipID,lprgvpTagArray,ulNumTags))
			return 0;
		
		if(ulTag>ulNumTags) return 0;

		lputTag=(UNKNOWNTAG*)lprgvpTagArray[ulTag];

		/*get frame label*/
		for(UI32 iTag=ulTag; iTag<ulNumTags; iTag++)
		{
			if(((UNKNOWNTAG*)lprgvpTagArray[ulTag])->rhTagHeader.usTagCode==TAGCODE_SHOWFRAME)
			{
				lptiInfo->usFrame=((SHOWFRAME*)lprgvpTagArray[ulTag])->usFrame;
				lpflLabel=GetFrameLabel(usClipID,lptiInfo->usFrame);
			
				if(lpflLabel)
				{
					//lptiInfo->szName=new char[strlen(lpflLabel->szLabel)+1];
					strcpy(lptiInfo->szName,lpflLabel->szLabel);
				}

				break;
			}
		}
	}

	lptiInfo->usTagCode=lputTag->rhTagHeader.usTagCode;
	lptiInfo->usCharacterID=lputTag->usCharacterID;
	strcpy(lptiInfo->szTagName,TagNames[lptiInfo->usTagCode]);

	strcpy(lptiInfo->szInfo,"");

	if(IsCharacter(lputTag))
	{
		usCharacter=lputTag->usCharacterID;
		sprintf(lptiInfo->szInfo,"Character ID: %lu",usCharacter);
	}

	switch(lputTag->rhTagHeader.usTagCode)
	{
		case TAGCODE_SETBACKGROUNDCOLOR:
				sprintf(lptiInfo->szInfo,"RGB(%lu, %lu, %lu)",
						((SETBACKGROUNDCOLOR*)lputTag)->rgbBackgroundColor.ubRed,
						((SETBACKGROUNDCOLOR*)lputTag)->rgbBackgroundColor.ubGreen,
						((SETBACKGROUNDCOLOR*)lputTag)->rgbBackgroundColor.ubBlue);
				break;
			
			case TAGCODE_DEFINESHAPE:
			case TAGCODE_DEFINESHAPE2:
			case TAGCODE_DEFINESHAPE3:
			
				break;

			case TAGCODE_DEFINEMORPHSHAPE:
				break;

			case TAGCODE_JPEGTABLES:
				break;

			case TAGCODE_DEFINEBITS:
				break;
			
			case TAGCODE_DEFINEBITSJPEG2:
				break;

			case TAGCODE_DEFINEBITSJPEG3:
				break;

			case TAGCODE_DEFINEBITSLOSSLESS:
				break;

			case TAGCODE_DEFINEBUTTON:
				break;

			case TAGCODE_DEFINEBUTTON2:
				break;

			case TAGCODE_DEFINEBUTTONSOUND:
				break;

			case TAGCODE_DEFINEBUTTONCXFORM:
				break;

			case TAGCODE_DEFINEFONT:
				break;

			case TAGCODE_DEFINEFONT2:
				break;

			case TAGCODE_DEFINEFONTINFO:
				break;
		
			case TAGCODE_DEFINESOUND:
				break;

			case TAGCODE_DEFINESPRITE:
				break;

			case TAGCODE_DEFINEVIDEOSTREAM:
				break;

			case TAGCODE_VIDEOFRAME:
				break;

			case TAGCODE_DEFINETEXT:
			case TAGCODE_DEFINETEXT2:
				break;

			case TAGCODE_DEFINEEDITTEXT:
				break;

			case TAGCODE_DOACTION:
			case TAGCODE_DOINITACTION:
				break;

			case TAGCODE_EXPORTASSETS:
				break;

			case TAGCODE_FRAMELABEL:
				sprintf(lptiInfo->szInfo,"Label: \"%s\"\n",((FRAMELABEL*)lputTag)->szLabel);
				break;

			case TAGCODE_PLACEOBJECT:
				usCharacter=((PLACEOBJECT*)lputTag)->usCharacterID;
				sprintf(szTemp,"Character ID: %lu\n",usCharacter);
				strcat(lptiInfo->szInfo,szTemp);
				
				sprintf(szTemp,"Depth",((PLACEOBJECT*)lputTag)->usDepth);
				strcat(lptiInfo->szInfo,szTemp);
				
				sprintf(szTemp,"Matrix:\n[\t%.3f\t%.3f\t]\n[\t%.3f\t%.3f\t]\n[\t%.2f\t%.2f\t]\n",
						FP32TOFLOAT(((PLACEOBJECT*)lputTag)->mMatrix.lfpScaleX),
						FP32TOFLOAT(((PLACEOBJECT*)lputTag)->mMatrix.lfpRotateSkew0),
						FP32TOFLOAT(((PLACEOBJECT*)lputTag)->mMatrix.lfpRotateSkew1),
						FP32TOFLOAT(((PLACEOBJECT*)lputTag)->mMatrix.lfpScaleY),
						float(((PLACEOBJECT*)lputTag)->mMatrix.slTranslateX)/20,
						float(((PLACEOBJECT*)lputTag)->mMatrix.slTranslateY)/20);
				strcat(lptiInfo->szInfo,szTemp);

				sprintf(szTemp,"CXFORM:\nR*%.3f+%i, G*%.3f+%i, B*%.3f+%i)\n",
					FP16TOFLOAT(((PLACEOBJECT*)lputTag)->cxfColorTransform.sfpRedMultTerm),
					((PLACEOBJECT*)lputTag)->cxfColorTransform.sbRedAddTerm,
					FP16TOFLOAT(((PLACEOBJECT*)lputTag)->cxfColorTransform.sfpGreenMultTerm),
					((PLACEOBJECT*)lputTag)->cxfColorTransform.sbGreenAddTerm,
					FP16TOFLOAT(((PLACEOBJECT*)lputTag)->cxfColorTransform.sfpBlueMultTerm),
					((PLACEOBJECT*)lputTag)->cxfColorTransform.sbBlueAddTerm);
				strcat(lptiInfo->szInfo,szTemp);
					
				break;

			case TAGCODE_PLACEOBJECT2:
				//if(((PLACEOBJECT2*)lputTag)->fPlaceFlagHasClipActions)

				if(((PLACEOBJECT2*)lputTag)->fPlaceFlagHasCharacter)
				{
					usCharacter=((PLACEOBJECT2*)lputTag)->usCharacterID;
					sprintf(szTemp,"Character ID: %lu\n",usCharacter);
					strcat(lptiInfo->szInfo,szTemp);
				}

				if(((PLACEOBJECT2*)lputTag)->fPlaceFlagHasName)
				{
					sprintf(szTemp,"Name: %s\n",((PLACEOBJECT2*)lputTag)->szName);
					strcat(lptiInfo->szInfo,szTemp);
				}

				sprintf(szTemp,"Depth: %lu\n",((PLACEOBJECT2*)lputTag)->usDepth);
				strcat(lptiInfo->szInfo,szTemp);
		
				if(((PLACEOBJECT2*)lputTag)->fPlaceFlagMove)
				{
					sprintf(szTemp,"Move\n");
					strcat(lptiInfo->szInfo,szTemp);
				}
				
				if(((PLACEOBJECT2*)lputTag)->fPlaceFlagHasRatio)
				{
					sprintf(szTemp,"Ratio: %lu\n",((PLACEOBJECT2*)lputTag)->usRatio);
					strcat(lptiInfo->szInfo,szTemp);
				}

				if(((PLACEOBJECT2*)lputTag)->fPlaceFlagHasMatrix)
				{
					sprintf(szTemp,"Matrix:\n[\t%.3f\t%.3f\t]\n[\t%.3f\t%.3f\t]\n[\t%.2f\t%.2f\t]\n",
						FP32TOFLOAT(((PLACEOBJECT2*)lputTag)->mMatrix.lfpScaleX),
						FP32TOFLOAT(((PLACEOBJECT2*)lputTag)->mMatrix.lfpRotateSkew0),
						FP32TOFLOAT(((PLACEOBJECT2*)lputTag)->mMatrix.lfpRotateSkew1),
						FP32TOFLOAT(((PLACEOBJECT2*)lputTag)->mMatrix.lfpScaleY),
						float(((PLACEOBJECT2*)lputTag)->mMatrix.slTranslateX)/20,
						float(((PLACEOBJECT2*)lputTag)->mMatrix.slTranslateY)/20);
					strcat(lptiInfo->szInfo,szTemp);
				}

				if(((PLACEOBJECT2*)lputTag)->fPlaceFlagHasColorTransform)
				{
					sprintf(szTemp,"CXFORM:\nR*%.3f+%i, G*%.3f+%i, B*%.3f+%i, A*%.3f+%i\n",
					FP16TOFLOAT(((PLACEOBJECT2*)lputTag)->cxfwaColorTransform.sfpRedMultTerm),
					((PLACEOBJECT2*)lputTag)->cxfwaColorTransform.sbRedAddTerm,
					FP16TOFLOAT(((PLACEOBJECT2*)lputTag)->cxfwaColorTransform.sfpGreenMultTerm),
					((PLACEOBJECT2*)lputTag)->cxfwaColorTransform.sbGreenAddTerm,
					FP16TOFLOAT(((PLACEOBJECT2*)lputTag)->cxfwaColorTransform.sfpBlueMultTerm),
					((PLACEOBJECT2*)lputTag)->cxfwaColorTransform.sbBlueAddTerm,
					FP16TOFLOAT(((PLACEOBJECT2*)lputTag)->cxfwaColorTransform.sfpAlphaMultTerm),
					((PLACEOBJECT2*)lputTag)->cxfwaColorTransform.sbAlphaAddTerm);
					strcat(lptiInfo->szInfo,szTemp);
				}

				break;

			case TAGCODE_PROTECT:
				break;

			case TAGCODE_REMOVEOBJECT:
				usCharacter=((REMOVEOBJECT*)lputTag)->usCharacterID;
				sprintf(szTemp,"Character ID: %lu\n",usCharacter);
				strcat(lptiInfo->szInfo,szTemp);
			
				sprintf(szTemp,"Depth: %lu\n",((REMOVEOBJECT*)lputTag)->usDepth);
				strcat(lptiInfo->szInfo,szTemp);
				break;

			case TAGCODE_REMOVEOBJECT2:
				sprintf(szTemp,"Depth: %lu\n",((REMOVEOBJECT2*)lputTag)->usDepth);
				strcat(lptiInfo->szInfo,szTemp);
				break;

			case TAGCODE_SOUNDSTREAMBLOCK:
				break;

			case TAGCODE_STARTSOUND:
				usCharacter=((STARTSOUND*)lputTag)->usSoundID;
				sprintf(szTemp,"Character ID: %lu\n",usCharacter);
				strcat(lptiInfo->szInfo,szTemp);
				break;

			case TAGCODE_SHOWFRAME:
				sprintf(szTemp,"Frame: %lu\n",((SHOWFRAME*)lputTag)->usFrame);
				strcat(lptiInfo->szInfo,szTemp);
				break;

		default: strcpy(lptiInfo->szInfo,"");
	}
	
	return usCharacter;
}

UI32 CSWF::Save(char *szFileName)
{
	//Write uncompressed file
	SWFFILEHEADER fhUCFileHeader;
	FILE *pfileSWF=fopen(szFileName,"wb");
	
	/*Write file header*/
	memcpy(&fhUCFileHeader,&fhFileHeader,sizeof(SWFFILEHEADER));
	fhUCFileHeader.bSignature1='F';
	fwrite(&fhUCFileHeader,1,sizeof(SWFFILEHEADER),pfileSWF);
	
	/*Write frame header*/
	WriteFrameHeader(&fhFrameHeader,pfileSWF);
	
	/*Write tags*/
	WriteTags(lprgvpTags,nTags,pfileSWF);

	/*Go back and write the file length in the SWFFILEHEADER*/
	UI32 ulLength=ftell(pfileSWF);
	fseek(pfileSWF,4,SEEK_SET);
	fwrite(&ulLength,1,sizeof(UI32),pfileSWF);
	fclose(pfileSWF);

	return fhUCFileHeader.ulFileLength;
}

void CSWF::Clear() 
{
	if(fDeleteTags) DeleteTags(lprgvpTags,nTags);

	if(szSWFName) delete szSWFName;
	
	szSWFName=NULL;
	
	memset(lprgvpTags,0,4*MAX_TAGS); 
	memset(lprgvpCharacters,0,4*0x10000);
	//memset(lprgflpLabels,0,4*0x10000);
	if(lprgflpLabels) delete lprgflpLabels;
	fLoaded=0;
	nTags=0;
	nFrames=0;
	lpjtJPEGTables=NULL;
}
#include "swf.h"

UI32 CSWF::ReadDefineSprite(DEFINESPRITE *lpdsSprite)
{
	UI32 ulTagLength=bsInFileStream.GetByteOffset();

	memset(lpdsSprite,0,sizeof(DEFINESPRITE));
	memcpy(&lpdsSprite->rhTagHeader,&rhTagHeader,sizeof(RECORDHEADER));

	lpdsSprite->usSpriteID=bsInFileStream.ReadUI16();
	lpdsSprite->usFrameCount=bsInFileStream.ReadUI16();

	void **lprgvpControlTags=new void*[MAX_TAGS];
	lpdsSprite->ulNumControlTags=ReadTags(lprgvpControlTags,&lpdsSprite->usFrameCount,
										  lpdsSprite->lprgflpLabels,&lpdsSprite->usNumLabels);
	lpdsSprite->lprgvpControlTags=new void*[lpdsSprite->ulNumControlTags];
	memcpy(lpdsSprite->lprgvpControlTags,lprgvpControlTags,lpdsSprite->ulNumControlTags<<2);
	delete lprgvpControlTags;

	bsInFileStream.SeekNextByte();
	ulTagLength=bsInFileStream.GetByteOffset()-ulTagLength;

	return ulTagLength;
}

UI32 CSWF::WriteDefineSprite(DEFINESPRITE *lpdsSprite, FILE *pfileSWF)
{
	if(lpdsSprite->usSpriteID==2252)
		lpdsSprite=lpdsSprite;
	UI32 ulHeader=ftell(pfileSWF);
	
	WriteTagHeader(&lpdsSprite->rhTagHeader,pfileSWF);

	UI32 ulLength=ftell(pfileSWF);

	fwrite(&lpdsSprite->usSpriteID,1,sizeof(UI16),pfileSWF);

	fwrite(&lpdsSprite->usFrameCount,1,sizeof(UI16),pfileSWF);
	WriteTags(lpdsSprite->lprgvpControlTags,lpdsSprite->ulNumControlTags,pfileSWF);

	ulLength=ftell(pfileSWF)-ulLength;
	fseek(pfileSWF,ulHeader,SEEK_SET);
	
	lpdsSprite->rhTagHeader.ulLength=ulLength;
	WriteTagHeader(&lpdsSprite->rhTagHeader,pfileSWF);
	fseek(pfileSWF,ulLength,SEEK_CUR);

	return lpdsSprite->rhTagHeader.ulLength;
}

UI32 CSWF::ExportSpriteDependancies(DEFINESPRITE* lpdsSprite, CSWF *lpcswfSpriteFlash)
{
	UI16 usCharacterID;
	UNKNOWNTAG *lputControlTag, *lputCharacter;
		
	for(int iTag=0; iTag<lpdsSprite->ulNumControlTags; iTag++)
	{
		lputControlTag=(UNKNOWNTAG*)lpdsSprite->lprgvpControlTags[iTag];

		if(lputControlTag->rhTagHeader.usTagCode==TAGCODE_PLACEOBJECT)
			usCharacterID=((PLACEOBJECT*)lpdsSprite->lprgvpControlTags[iTag])->usCharacterID;

		else if(lputControlTag->rhTagHeader.usTagCode==TAGCODE_PLACEOBJECT2)
			usCharacterID=((PLACEOBJECT2*)lpdsSprite->lprgvpControlTags[iTag])->usCharacterID;
		
		else if(lputControlTag->rhTagHeader.usTagCode==TAGCODE_STARTSOUND)
			usCharacterID=((STARTSOUND*)lpdsSprite->lprgvpControlTags[iTag])->usSoundID;

		else if(lputControlTag->rhTagHeader.usTagCode==TAGCODE_DEFINEEDITTEXT)
			usCharacterID=((DEFINEEDITTEXT*)lprgvpCharacters[usCharacterID])->usCharacterID;

		else continue;

		lputCharacter=(UNKNOWNTAG*)lprgvpCharacters[usCharacterID];
	
		if(!lputCharacter) continue;

		/*export this characters dependancies*/
		if(lputCharacter->rhTagHeader.usTagCode==TAGCODE_DEFINESHAPE ||
		   lputCharacter->rhTagHeader.usTagCode==TAGCODE_DEFINESHAPE2) 
			ExportShapeDependancies((DEFINESHAPE*)lprgvpCharacters[usCharacterID],lpcswfSpriteFlash);

		if(lputCharacter->rhTagHeader.usTagCode==TAGCODE_DEFINEMORPHSHAPE)
			ExportMorphShapeDependancies((DEFINEMORPHSHAPE*)lprgvpCharacters[usCharacterID],lpcswfSpriteFlash);
		
		if(lputCharacter->rhTagHeader.usTagCode==TAGCODE_DEFINESPRITE)
			ExportSpriteDependancies((DEFINESPRITE*)lprgvpCharacters[usCharacterID],lpcswfSpriteFlash);

		if(lputCharacter->rhTagHeader.usTagCode==TAGCODE_DEFINEBUTTON ||
		   lputCharacter->rhTagHeader.usTagCode==TAGCODE_DEFINEBUTTON2)
			ExportButtonDependancies((DEFINEBUTTON*)lprgvpCharacters[usCharacterID],lpcswfSpriteFlash);

		/*If it's text, write the proper font*/
		if(lputCharacter->rhTagHeader.usTagCode==TAGCODE_DEFINETEXT ||
		   lputCharacter->rhTagHeader.usTagCode==TAGCODE_DEFINETEXT2)
				ExportTextDependancies((DEFINETEXT*)lprgvpCharacters[usCharacterID],lpcswfSpriteFlash);

		lpcswfSpriteFlash->AddTag(lprgvpCharacters[usCharacterID]);
	}

	return 1;
}

UI32 CSWF::SaveDefineSprite(DEFINESPRITE* lpdsSprite, UI16 usFrames)
{
	UI32 ulLength;
	UI16 usFrameCount=0;
	char szSpriteFileName[4096];
	CSWF cswfSpriteFlash;
	void **lprgvpTagArray;
	UI32 ulNumTags;
	SI32 slWidth=(fhFrameHeader.srFrameSize.slXMax-fhFrameHeader.srFrameSize.slXMin)/2;
	SI32 slHeight=(fhFrameHeader.srFrameSize.slYMax-fhFrameHeader.srFrameSize.slYMin)/2;

	/*Create file*/
	if(usFrames)
		CreateFileName(szSpriteFileName,"frame",usFrames,"swf");
	
	else CreateFileName(szSpriteFileName,"sprite",lpdsSprite->usSpriteID,"swf");

	/*SWFFILEHEADER*/
	SWFFILEHEADER fhSpriteFileHeader;
	fhSpriteFileHeader.bSignature1='F';
	fhSpriteFileHeader.bSignature2='W';
	fhSpriteFileHeader.bSignature3='S';
	fhSpriteFileHeader.bVersion=6;
	fhSpriteFileHeader.ulFileLength=0;
	cswfSpriteFlash.SetFileHeader(&fhSpriteFileHeader);

	/*SETBACKGROUNDCOLOR*/
	SETBACKGROUNDCOLOR sbcSpriteBackground;
	sbcSpriteBackground.rhTagHeader.usTagCode=TAGCODE_SETBACKGROUNDCOLOR;
	sbcSpriteBackground.rhTagHeader.ulLength=3;
	sbcSpriteBackground.rgbBackgroundColor.ubRed=128;
	sbcSpriteBackground.rgbBackgroundColor.ubGreen=128;
	sbcSpriteBackground.rgbBackgroundColor.ubBlue=128;
	cswfSpriteFlash.AddTag(&sbcSpriteBackground);
	
	/*Sprite*/
	cswfSpriteFlash.AddTag(lpjtJPEGTables);
	ExportSpriteDependancies(lpdsSprite,&cswfSpriteFlash);
	//cswfSpriteFlash.AddTag(lpdsSprite);

	GetTagArray(lpdsSprite->usSpriteID,lprgvpTagArray,ulNumTags);

	for(UI32 iTag=0; iTag<ulNumTags-1; iTag++)
	{
		if(((UNKNOWNTAG*)lprgvpTagArray[iTag])->rhTagHeader.usTagCode==TAGCODE_SHOWFRAME)
		{
			if(!usFrames) 
				cswfSpriteFlash.AddTag(lprgvpTagArray[iTag]);
			usFrameCount++;
		}
		
		else cswfSpriteFlash.AddTag(lprgvpTagArray[iTag]);

		if(usFrames && usFrameCount==usFrames) break;
	}

	/*SWFFRAMEHEADER*/
	SWFFRAMEHEADER fhSpriteFrameHeader;
	fhSpriteFrameHeader.sfpFrameRate=fhFrameHeader.sfpFrameRate;
	if(usFrames) fhSpriteFrameHeader.usFrameCount=1;
	else fhSpriteFrameHeader.usFrameCount=lpdsSprite->usFrameCount;
	fhSpriteFrameHeader.srFrameSize.bNbits=31;
	fhSpriteFrameHeader.srFrameSize.slXMin=-1*slWidth;
	fhSpriteFrameHeader.srFrameSize.slYMin=-1*slHeight;
	fhSpriteFrameHeader.srFrameSize.slXMax=slWidth;
	fhSpriteFrameHeader.srFrameSize.slYMax=slHeight;
	cswfSpriteFlash.SetFrameHeader(&fhSpriteFrameHeader);
	
	/*PLACEOBJECT(2)*/
	/*MATRIX mMatrix;
	mMatrix.fHasScale=0;
	mMatrix.lfpScaleX=1;
	mMatrix.lfpScaleY=1;
	mMatrix.fHasRotate=0;
	mMatrix.ubNTranslateBits=31;
	mMatrix.slTranslateX=0;
	mMatrix.slTranslateY=0;

	PLACEOBJECT2 po2Place;
	memset(&po2Place,0,sizeof(PLACEOBJECT2));
	po2Place.rhTagHeader.usTagCode=TAGCODE_PLACEOBJECT2;
	po2Place.fPlaceFlagHasMatrix=4;
	po2Place.fPlaceFlagHasCharacter=2;
	po2Place.usCharacterID=lpdsSprite->usSpriteID;
	po2Place.usDepth=1;
	memcpy(&po2Place.mMatrix,&mMatrix,sizeof(MATRIX));
	cswfSpriteFlash.AddTag(&po2Place);*/

	
	/*SHOWFRAME*/
	/*UNKNOWNTAG utShowFrame;
	memset(&utShowFrame,0,sizeof(UNKNOWNTAG));
	utShowFrame.rhTagHeader.usTagCode=TAGCODE_SHOWFRAME;
	cswfSpriteFlash.AddTag(&utShowFrame);*/

	/*END*/
	UNKNOWNTAG utEnd;
	memset(&utEnd,0,sizeof(UNKNOWNTAG));
	utEnd.rhTagHeader.usTagCode=TAGCODE_END;
	cswfSpriteFlash.AddTag(&utEnd);

	cswfSpriteFlash.Save(szSpriteFileName);
	return 0;
}
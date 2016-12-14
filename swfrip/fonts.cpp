#include "swf.h"

UI32 CSWF::ReadDefineFont(DEFINEFONT *lpdfDefineFont)
{
	UI32 ulTagLength=bsInFileStream.GetByteOffset();
	UI16 iGlyph;

	memset(lpdfDefineFont,0,sizeof(DEFINEFONT));
	memcpy(&lpdfDefineFont->rhTagHeader,&rhTagHeader,sizeof(RECORDHEADER));

	lpdfDefineFont->usFontID=bsInFileStream.ReadUI16();
	lpdfDefineFont->usNumGlyphs=bsInFileStream.GetUI16()/2;
	lpdfDefineFont->lprgulOffsetTable=new UI32[lpdfDefineFont->usNumGlyphs];

	for(iGlyph=0; iGlyph<lpdfDefineFont->usNumGlyphs; iGlyph++)
		lpdfDefineFont->lprgulOffsetTable[iGlyph]=bsInFileStream.ReadUI16();

	/*Read glyph shapes*/
	lpdfDefineFont->lprgsGlyphShapeTable=new SHAPE[lpdfDefineFont->usNumGlyphs];
	for(iGlyph=0; iGlyph<lpdfDefineFont->usNumGlyphs; iGlyph++)
		ReadShape(&lpdfDefineFont->lprgsGlyphShapeTable[iGlyph]);

	bsInFileStream.SeekNextByte();
	ulTagLength=bsInFileStream.GetByteOffset()-ulTagLength;

	return ulTagLength;
}

UI32 CSWF::WriteDefineFont(DEFINEFONT *lpdfDefineFont, FILE *pfileSWF)
{
	bsOutFileStream.Create(sizeof(DEFINEFONT)+lpdfDefineFont->rhTagHeader.ulLength);
	UI16 iGlyph;

	bsOutFileStream.WriteUI16(lpdfDefineFont->usFontID);

	for(iGlyph=0; iGlyph<lpdfDefineFont->usNumGlyphs; iGlyph++)
		bsOutFileStream.WriteUI16(lpdfDefineFont->lprgulOffsetTable[iGlyph]);

	/*Write glyph shapes*/
	for(iGlyph=0; iGlyph<lpdfDefineFont->usNumGlyphs; iGlyph++)
		WriteShape(&lpdfDefineFont->lprgsGlyphShapeTable[iGlyph]);

	lpdfDefineFont->rhTagHeader.ulLength=bsOutFileStream.GetByteOffset();
	WriteTagHeader(&lpdfDefineFont->rhTagHeader,pfileSWF);
	bsOutFileStream.WriteToFile(pfileSWF);

	return bsOutFileStream.GetByteOffset();
}

UI32 CSWF::DeleteDefineFont(DEFINEFONT *lpdfDefineFont)
{
	void **lprgvpShapeRecords;
	UI32 ulNumShapeRecords;

	delete lpdfDefineFont->lprgulOffsetTable;

	for(UI32 iGlyph=0; iGlyph<lpdfDefineFont->usNumGlyphs; iGlyph++)
	{
		lprgvpShapeRecords=lpdfDefineFont->lprgsGlyphShapeTable[iGlyph].lprgvpShapeRecords;
		ulNumShapeRecords=lpdfDefineFont->lprgsGlyphShapeTable[iGlyph].ulNumShapeRecords;
		DeleteShapeRecords(lprgvpShapeRecords,ulNumShapeRecords);
	}

	delete lpdfDefineFont->lprgsGlyphShapeTable;

	return 0;
}

UI32 CSWF::ReadDefineFont2(DEFINEFONT2 *lpdfDefineFont)
{
	UI32 ulTagLength=bsInFileStream.GetByteOffset();
	UI16 iGlyph;

	memset(lpdfDefineFont,0,sizeof(DEFINEFONT2));
	memcpy(&lpdfDefineFont->rhTagHeader,&rhTagHeader,sizeof(RECORDHEADER));
	
	lpdfDefineFont->usFontID=bsInFileStream.ReadUI16();
	lpdfDefineFont->fFontFlagsHasLayout=bsInFileStream.ReadUB(1);
	lpdfDefineFont->fFontFlagsShiftJIS=bsInFileStream.ReadUB(1);
	lpdfDefineFont->fFontFlagsSmallText=bsInFileStream.ReadUB(1);
	lpdfDefineFont->fFontFlagsANSI=bsInFileStream.ReadUB(1);
	lpdfDefineFont->fFontFlagsWideOffsets=bsInFileStream.ReadUB(1);
	lpdfDefineFont->fFontFlagsWideCodes=bsInFileStream.ReadUB(1);
	lpdfDefineFont->fFontFlagsItalic=bsInFileStream.ReadUB(1);
	lpdfDefineFont->fFontFlagsBold=bsInFileStream.ReadUB(1);
	lpdfDefineFont->ubLanguageCode=bsInFileStream.ReadUI8();
	lpdfDefineFont->ubFontNameLen=bsInFileStream.ReadUI8();
	lpdfDefineFont->lprgubFontName=new UI8[lpdfDefineFont->ubFontNameLen+1];
	bsInFileStream.ReadBytes(lpdfDefineFont->lprgubFontName,lpdfDefineFont->ubFontNameLen);
	lpdfDefineFont->lprgubFontName[lpdfDefineFont->ubFontNameLen]='\0';
	lpdfDefineFont->usNumGlyphs=bsInFileStream.ReadUI16();

	lpdfDefineFont->lprgulOffsetTable=new UI32[lpdfDefineFont->usNumGlyphs];

	if(lpdfDefineFont->fFontFlagsWideOffsets)
	{
		for(UI16 iGlyph=0; iGlyph<lpdfDefineFont->usNumGlyphs; iGlyph++)
			lpdfDefineFont->lprgulOffsetTable[iGlyph]=bsInFileStream.ReadUI32();
		lpdfDefineFont->ulCodeTableOffset=bsInFileStream.ReadUI32();
	}
	
	else 	
	{
		for(iGlyph=0; iGlyph<lpdfDefineFont->usNumGlyphs; iGlyph++)
			lpdfDefineFont->lprgulOffsetTable[iGlyph]=bsInFileStream.ReadUI16();
		lpdfDefineFont->ulCodeTableOffset=bsInFileStream.ReadUI16();
	}

	/*Read glyph shapes*/
	lpdfDefineFont->lprgsGlyphShapeTable=new SHAPE[lpdfDefineFont->usNumGlyphs];
	for(iGlyph=0; iGlyph<lpdfDefineFont->usNumGlyphs; iGlyph++)
		ReadShape(&lpdfDefineFont->lprgsGlyphShapeTable[iGlyph]);


	lpdfDefineFont->lprgusCodeTable=new UI16[lpdfDefineFont->usNumGlyphs];
	for(iGlyph=0; iGlyph<lpdfDefineFont->usNumGlyphs; iGlyph++)
	{
		if(lpdfDefineFont->fFontFlagsWideCodes)
			lpdfDefineFont->lprgusCodeTable[iGlyph]=bsInFileStream.ReadUI16();
		else lpdfDefineFont->lprgusCodeTable[iGlyph]=bsInFileStream.ReadUI8();
	}

	if(lpdfDefineFont->fFontFlagsHasLayout)
	{
		lpdfDefineFont->ssFontAscent=bsInFileStream.ReadSI16();
		lpdfDefineFont->ssFontDescent=bsInFileStream.ReadSI16();
		lpdfDefineFont->ssFontLeading=bsInFileStream.ReadSI16();
		
		lpdfDefineFont->lprgssFontAdvanceTable=new SI16[lpdfDefineFont->usNumGlyphs];

		for(iGlyph=0; iGlyph<lpdfDefineFont->usNumGlyphs; iGlyph++)
			lpdfDefineFont->lprgssFontAdvanceTable[iGlyph]=bsInFileStream.ReadSI16();

		lpdfDefineFont->lprgrFontBoundsTable=new SRECT[lpdfDefineFont->usNumGlyphs];

		for(iGlyph=0; iGlyph<lpdfDefineFont->usNumGlyphs; iGlyph++)
			ReadRect(&lpdfDefineFont->lprgrFontBoundsTable[iGlyph]);

		/*Read KERNINGRECORD array*/
		lpdfDefineFont->usKerningCount=bsInFileStream.ReadUI16();
		lpdfDefineFont->lprgkrFontKerningTable=new KERNINGRECORD[lpdfDefineFont->usKerningCount];

		for(iGlyph=0; iGlyph<lpdfDefineFont->usKerningCount; iGlyph++)
		{
			if(lpdfDefineFont->fFontFlagsWideCodes)
			{
				lpdfDefineFont->lprgkrFontKerningTable[iGlyph].usFontKerningCode1=bsInFileStream.ReadUI16();
				lpdfDefineFont->lprgkrFontKerningTable[iGlyph].usFontKerningCode2=bsInFileStream.ReadUI16();
			}

			else 
			{
				lpdfDefineFont->lprgkrFontKerningTable[iGlyph].usFontKerningCode1=bsInFileStream.ReadUI8();
				lpdfDefineFont->lprgkrFontKerningTable[iGlyph].usFontKerningCode2=bsInFileStream.ReadUI8();
			}

			lpdfDefineFont->lprgkrFontKerningTable[iGlyph].usFontKerningAdjustment=bsInFileStream.ReadSI16();
		}
	}
		
	bsInFileStream.SeekNextByte();
	ulTagLength=bsInFileStream.GetByteOffset()-ulTagLength;

	return ulTagLength;
}


UI32 CSWF::WriteDefineFont2(DEFINEFONT2 *lpdfDefineFont, FILE *pfileSWF)
{
	bsOutFileStream.Create(sizeof(DEFINEFONT2)+lpdfDefineFont->rhTagHeader.ulLength);
	UI16 iGlyph;

	bsOutFileStream.WriteUI16(lpdfDefineFont->usFontID);
	bsOutFileStream.WriteUB(lpdfDefineFont->fFontFlagsHasLayout,1);
	bsOutFileStream.WriteUB(lpdfDefineFont->fFontFlagsShiftJIS,1);
	bsOutFileStream.WriteUB(lpdfDefineFont->fFontFlagsSmallText,1);
	bsOutFileStream.WriteUB(lpdfDefineFont->fFontFlagsANSI,1);
	bsOutFileStream.WriteUB(lpdfDefineFont->fFontFlagsWideOffsets,1);
	bsOutFileStream.WriteUB(lpdfDefineFont->fFontFlagsWideCodes,1);
	bsOutFileStream.WriteUB(lpdfDefineFont->fFontFlagsItalic,1);
	bsOutFileStream.WriteUB(lpdfDefineFont->fFontFlagsBold,1);
	bsOutFileStream.WriteUI8(lpdfDefineFont->ubLanguageCode);
	bsOutFileStream.WriteUI8(lpdfDefineFont->ubFontNameLen);
	bsOutFileStream.WriteBytes(lpdfDefineFont->lprgubFontName,lpdfDefineFont->ubFontNameLen);
	bsOutFileStream.WriteUI16(lpdfDefineFont->usNumGlyphs);

	if(lpdfDefineFont->fFontFlagsWideOffsets)
	{
		for(iGlyph=0; iGlyph<lpdfDefineFont->usNumGlyphs; iGlyph++)
			bsOutFileStream.WriteUI32(lpdfDefineFont->lprgulOffsetTable[iGlyph]);
		bsOutFileStream.WriteUI32(lpdfDefineFont->ulCodeTableOffset);
	}
	
	else 	
	{
		for(iGlyph=0; iGlyph<lpdfDefineFont->usNumGlyphs; iGlyph++)
			bsOutFileStream.WriteUI16(lpdfDefineFont->lprgulOffsetTable[iGlyph]);
		bsOutFileStream.WriteUI16(lpdfDefineFont->ulCodeTableOffset);
	}

	/*Write glyph shapes*/
	for(iGlyph=0; iGlyph<lpdfDefineFont->usNumGlyphs; iGlyph++)
		WriteShape(&lpdfDefineFont->lprgsGlyphShapeTable[iGlyph]);

	for(iGlyph=0; iGlyph<lpdfDefineFont->usNumGlyphs; iGlyph++)
	{
		if(lpdfDefineFont->fFontFlagsWideCodes)
			bsOutFileStream.WriteUI16(lpdfDefineFont->lprgusCodeTable[iGlyph]);
		else bsOutFileStream.WriteUI8(lpdfDefineFont->lprgusCodeTable[iGlyph]);
	}

	if(lpdfDefineFont->fFontFlagsHasLayout)
	{
		bsOutFileStream.WriteSI16(lpdfDefineFont->ssFontAscent);
		bsOutFileStream.WriteSI16(lpdfDefineFont->ssFontDescent);
		bsOutFileStream.WriteSI16(lpdfDefineFont->ssFontLeading);
		
		for(iGlyph=0; iGlyph<lpdfDefineFont->usNumGlyphs; iGlyph++)
			bsOutFileStream.WriteSI16(lpdfDefineFont->lprgssFontAdvanceTable[iGlyph]);

		for(iGlyph=0; iGlyph<lpdfDefineFont->usNumGlyphs; iGlyph++)
			WriteRect(&lpdfDefineFont->lprgrFontBoundsTable[iGlyph]);

		/*Read KERNINGRECORD array*/
		bsOutFileStream.WriteUI16(lpdfDefineFont->usKerningCount);
		
		for(iGlyph=0; iGlyph<lpdfDefineFont->usKerningCount; iGlyph++)
		{
			if(lpdfDefineFont->fFontFlagsWideCodes)
			{
				bsOutFileStream.WriteUI16(lpdfDefineFont->lprgkrFontKerningTable[iGlyph].usFontKerningCode1);
				bsOutFileStream.WriteUI16(lpdfDefineFont->lprgkrFontKerningTable[iGlyph].usFontKerningCode2);
			}

			else 
			{
				bsOutFileStream.WriteUI8(lpdfDefineFont->lprgkrFontKerningTable[iGlyph].usFontKerningCode1);
				bsOutFileStream.WriteUI8(lpdfDefineFont->lprgkrFontKerningTable[iGlyph].usFontKerningCode2);
			}

			bsOutFileStream.WriteSI16(lpdfDefineFont->lprgkrFontKerningTable[iGlyph].usFontKerningAdjustment);
		}
	}
		
	lpdfDefineFont->rhTagHeader.ulLength=bsOutFileStream.GetByteOffset();
	WriteTagHeader(&lpdfDefineFont->rhTagHeader,pfileSWF);
	bsOutFileStream.WriteToFile(pfileSWF);

	return bsOutFileStream.GetByteOffset();
}

UI32 CSWF::DeleteDefineFont2(DEFINEFONT2 *lpdfDefineFont)
{
	void **lprgvpShapeRecords;
	UI32 ulNumShapeRecords;

	if(lpdfDefineFont->lprgubFontName) delete lpdfDefineFont->lprgubFontName;
	delete lpdfDefineFont->lprgulOffsetTable;

	for(UI32 iGlyph=0; iGlyph<lpdfDefineFont->usNumGlyphs; iGlyph++)
	{
		lprgvpShapeRecords=lpdfDefineFont->lprgsGlyphShapeTable[iGlyph].lprgvpShapeRecords;
		ulNumShapeRecords=lpdfDefineFont->lprgsGlyphShapeTable[iGlyph].ulNumShapeRecords;
		DeleteShapeRecords(lprgvpShapeRecords,ulNumShapeRecords);
	}

	delete lpdfDefineFont->lprgsGlyphShapeTable;

	delete lpdfDefineFont->lprgusCodeTable;
	delete lpdfDefineFont->lprgssFontAdvanceTable;
	delete lpdfDefineFont->lprgrFontBoundsTable;
	delete lpdfDefineFont->lprgkrFontKerningTable;

	return 0;
}

UI32 CSWF::ReadDefineFontInfo(DEFINEFONTINFO *lpdfiDefineFontInfo)
{
	UI32 ulTagLength=bsInFileStream.GetByteOffset();
	
	memset(lpdfiDefineFontInfo,0,sizeof(DEFINEFONTINFO));
	memcpy(&lpdfiDefineFontInfo->rhTagHeader,&rhTagHeader,sizeof(RECORDHEADER));

	lpdfiDefineFontInfo->usFontID=bsInFileStream.ReadUI16();
	lpdfiDefineFontInfo->usNumGlyphs=((DEFINEFONT*)lprgvpCharacters[lpdfiDefineFontInfo->usFontID])->usNumGlyphs;
	((DEFINEFONT*)lprgvpCharacters[lpdfiDefineFontInfo->usFontID])->lpdfiFontInfo=lpdfiDefineFontInfo;
	lpdfiDefineFontInfo->ubFontNameLen=bsInFileStream.ReadUI8();
	lpdfiDefineFontInfo->lprgubFontName=new UI8[lpdfiDefineFontInfo->ubFontNameLen+1];
	bsInFileStream.ReadBytes(lpdfiDefineFontInfo->lprgubFontName,lpdfiDefineFontInfo->ubFontNameLen);
	lpdfiDefineFontInfo->lprgubFontName[lpdfiDefineFontInfo->ubFontNameLen]='\0';

	lpdfiDefineFontInfo->ubFontFlagsReserved=bsInFileStream.ReadUB(2);
	lpdfiDefineFontInfo->fFontFlagsSmallText=bsInFileStream.ReadUB(1);
	lpdfiDefineFontInfo->fFontFlagsShiftJIS=bsInFileStream.ReadUB(1);
	lpdfiDefineFontInfo->fFontFlagsANSI=bsInFileStream.ReadUB(1);
	lpdfiDefineFontInfo->fFontFlagsItalic=bsInFileStream.ReadUB(1);
	lpdfiDefineFontInfo->fFontFlagsBold=bsInFileStream.ReadUB(1);
	lpdfiDefineFontInfo->fFontFlagsWideCodes=bsInFileStream.ReadUB(1);
	if(lpdfiDefineFontInfo->rhTagHeader.usTagCode==TAGCODE_DEFINEFONTINFO2)
		lpdfiDefineFontInfo->ubLanguageCode=bsInFileStream.ReadUI8();

	lpdfiDefineFontInfo->lprgusCodeTable=new UI16[lpdfiDefineFontInfo->usNumGlyphs];
	for(UI16 iGlyph=0; iGlyph<lpdfiDefineFontInfo->usNumGlyphs; iGlyph++)
	{
		if(lpdfiDefineFontInfo->fFontFlagsWideCodes)
			lpdfiDefineFontInfo->lprgusCodeTable[iGlyph]=bsInFileStream.ReadUI16();
		else lpdfiDefineFontInfo->lprgusCodeTable[iGlyph]=bsInFileStream.ReadUI8();
	}

	bsInFileStream.SeekNextByte();
	ulTagLength=bsInFileStream.GetByteOffset()-ulTagLength;

	return ulTagLength;
}

UI32 CSWF::WriteDefineFontInfo(DEFINEFONTINFO *lpdfiDefineFontInfo, FILE *pfileSWF)
{
	bsOutFileStream.Create(sizeof(DEFINEFONTINFO)+lpdfiDefineFontInfo->rhTagHeader.ulLength);
	UI16 iGlyph;

	bsOutFileStream.WriteUI16(lpdfiDefineFontInfo->usFontID);
	bsOutFileStream.WriteUI8(lpdfiDefineFontInfo->ubFontNameLen);
	bsOutFileStream.WriteBytes(lpdfiDefineFontInfo->lprgubFontName,lpdfiDefineFontInfo->ubFontNameLen);
	bsOutFileStream.WriteUB(lpdfiDefineFontInfo->ubFontFlagsReserved,2);
	bsOutFileStream.WriteUB(lpdfiDefineFontInfo->fFontFlagsSmallText,1);
	bsOutFileStream.WriteUB(lpdfiDefineFontInfo->fFontFlagsShiftJIS,1);
	bsOutFileStream.WriteUB(lpdfiDefineFontInfo->fFontFlagsANSI,1);
	bsOutFileStream.WriteUB(lpdfiDefineFontInfo->fFontFlagsItalic,1);
	bsOutFileStream.WriteUB(lpdfiDefineFontInfo->fFontFlagsBold,1);
	bsOutFileStream.WriteUB(lpdfiDefineFontInfo->fFontFlagsWideCodes,1);
	if(lpdfiDefineFontInfo->rhTagHeader.usTagCode==TAGCODE_DEFINEFONTINFO2)
		bsOutFileStream.WriteUI8(lpdfiDefineFontInfo->ubLanguageCode);
	
	for(iGlyph=0; iGlyph<lpdfiDefineFontInfo->usNumGlyphs; iGlyph++)
	{
		if(lpdfiDefineFontInfo->fFontFlagsWideCodes)
			bsOutFileStream.WriteUI16(lpdfiDefineFontInfo->lprgusCodeTable[iGlyph]);
		else bsOutFileStream.WriteUI8(lpdfiDefineFontInfo->lprgusCodeTable[iGlyph]);
	}

	lpdfiDefineFontInfo->rhTagHeader.ulLength=bsOutFileStream.GetByteOffset();
	WriteTagHeader(&lpdfiDefineFontInfo->rhTagHeader,pfileSWF);
	bsOutFileStream.WriteToFile(pfileSWF);

	return bsOutFileStream.GetByteOffset();
}

TEXTRECORD *MakeTextRecord(UI8 *ubString, UI32 ulLength, UI16 usFontID)
{
	if(!ulLength) return 0;

	TEXTRECORD *lptrTextRecord=new TEXTRECORD;
	memset(lptrTextRecord,0,sizeof(TEXTRECORD));

	lptrTextRecord->ubTextRecordType=1;
	lptrTextRecord->ubStyleFlagsReserved=0;
	lptrTextRecord->fStyleFlagsHasFont=true;
	lptrTextRecord->fStyleFlagsHasColor=true;
	lptrTextRecord->fStyleFlagsHasXOffset=false;
	lptrTextRecord->fStyleFlagsHasYOffset=false;
	lptrTextRecord->rgbaTextColor.ubRed=255;
	lptrTextRecord->rgbaTextColor.ubGreen=255;
	lptrTextRecord->rgbaTextColor.ubBlue=255;
	lptrTextRecord->rgbaTextColor.ubAlpha=255;
	lptrTextRecord->usFontID=usFontID;
	lptrTextRecord->usTextHeight=140;
	lptrTextRecord->ssXOffset=0xa0;
	lptrTextRecord->ssYOffset=0xdb;
	lptrTextRecord->ubGlyphCount=ulLength;
	lptrTextRecord->lprggeGlyphEntries=new GLYPHENTRY[lptrTextRecord->ubGlyphCount];

	for(UI32 iGlyph=0; iGlyph<lptrTextRecord->ubGlyphCount; iGlyph++)
	{
		lptrTextRecord->lprggeGlyphEntries[iGlyph].ulGlyphIndex=ubString[iGlyph];
		lptrTextRecord->lprggeGlyphEntries[iGlyph].slGlyphAdvance=160;
	}

	return lptrTextRecord;	
}

UI32 CSWF::SaveDefineFont(void* lpvFont)
{
	UI32 ulLength, ulGlyphs;
	char szFontFileName[MAX_FILEPATH];
	UI8 *ubString;
	UI16 usFontID;
	CSWF cswfFontFlash;
	DEFINEFONT *lpdfFont=NULL;
	DEFINEFONT2 *lpdf2Font=NULL;

	cswfFontFlash.SetDeleteTags(false);
	
	/*SWFFILEHEADER*/
	SWFFILEHEADER fhFontFileHeader;
	fhFontFileHeader.bSignature1='F';
	fhFontFileHeader.bSignature2='W';
	fhFontFileHeader.bSignature3='S';
	fhFontFileHeader.bVersion=6;
	fhFontFileHeader.ulFileLength=0;
	cswfFontFlash.SetFileHeader(&fhFontFileHeader);
	
	/*SWFFRAMEHEADER*/
	SWFFRAMEHEADER fhFontFrameHeader;
	fhFontFrameHeader.usFrameCount=1;
	fhFontFrameHeader.sfpFrameRate=1;
	fhFontFrameHeader.srFrameSize.slXMin=0;
	fhFontFrameHeader.srFrameSize.slXMax=2000;
	fhFontFrameHeader.srFrameSize.slYMin=-200;
	fhFontFrameHeader.srFrameSize.slYMax=200;
	cswfFontFlash.SetFrameHeader(&fhFontFrameHeader);

	/*SETBACKGROUNDCOLOR*/
	SETBACKGROUNDCOLOR sbcFontBackground;
	sbcFontBackground.rhTagHeader.usTagCode=TAGCODE_SETBACKGROUNDCOLOR;
	sbcFontBackground.rhTagHeader.ulLength=3;
	sbcFontBackground.rgbBackgroundColor.ubRed=128;
	sbcFontBackground.rgbBackgroundColor.ubGreen=128;
	sbcFontBackground.rgbBackgroundColor.ubBlue=128;
	cswfFontFlash.AddTag(&sbcFontBackground);

	/*DEFINEFONT*/
	switch(((UNKNOWNTAG*)lpvFont)->rhTagHeader.usTagCode)
	{
		case TAGCODE_DEFINEFONT:
			lpdfFont=(DEFINEFONT*)lpvFont;
			usFontID=lpdfFont->usFontID;
			ulGlyphs=lpdfFont->usNumGlyphs;
			cswfFontFlash.AddTag(lpdfFont);
			cswfFontFlash.AddTag(lpdfFont->lpdfiFontInfo);
			break;

		case TAGCODE_DEFINEFONT2:
			lpdf2Font=(DEFINEFONT2*)lpvFont;
			usFontID=lpdf2Font->usFontID;
			ulGlyphs=lpdf2Font->usNumGlyphs;
			cswfFontFlash.AddTag(lpdf2Font);
			break;

		default: return 0;
	}
	
	EXPORTASSETS eaExport;
	eaExport.rhTagHeader.usTagCode=TAGCODE_EXPORTASSETS;
	eaExport.usCount=1;
	eaExport.lprgaAssets=new ASSET;
	eaExport.lprgaAssets->usCharacterID=((UNKNOWNTAG*)lpvFont)->usCharacterID;
	eaExport.lprgaAssets->szName=NULL;
	cswfFontFlash.AddTag(&eaExport);

	/*display string*/
	if(ulGlyphs>50) ulGlyphs=50;
	ubString=new UI8[ulGlyphs];
	for(UI32 iGlyph=0; iGlyph<ulGlyphs; iGlyph++) 
		ubString[iGlyph]=iGlyph;

	/*DEFINETEXT*/
	DEFINETEXT dtText;
	memset(&dtText,0,sizeof(DEFINETEXT));
	dtText.rhTagHeader.usTagCode=TAGCODE_DEFINETEXT;
	dtText.usCharacterID=3;
	/*dtText.rTextBounds.slXMin=0;
	dtText.rTextBounds.slXMax=5;
	dtText.rTextBounds.slYMin=0;
	dtText.rTextBounds.slYMax=5;*/
	//dtText.mTextMatrix.slTranslateX=-1000;

	dtText.ubGlyphBits=32;
	dtText.ubAdvanceBits=32;
	
	if(ulGlyphs)
	{
		dtText.ulNumTextRecords=1;
		dtText.lprgtrpTextRecords=new TEXTRECORD*[1];
		dtText.lprgtrpTextRecords[0]=MakeTextRecord(ubString,ulGlyphs,((DEFINEFONT*)lpvFont)->usFontID);
	}
	else dtText.ulNumTextRecords=0;
	cswfFontFlash.AddTag(&dtText);

	/*PLACEOBJECT2*/
	PLACEOBJECT2 po2Place;
	memset(&po2Place,0,sizeof(PLACEOBJECT2));
	po2Place.rhTagHeader.usTagCode=TAGCODE_PLACEOBJECT2;
	po2Place.fPlaceFlagHasCharacter=true;
	po2Place.fPlaceFlagHasClipDepth=true;
	po2Place.usCharacterID=dtText.usCharacterID;
	po2Place.usDepth=1;
	cswfFontFlash.AddTag(&po2Place);

	/*SHOWFRAME and END*/
	UNKNOWNTAG utShowFrame;
	UNKNOWNTAG utEnd;
	memset(&utShowFrame,0,sizeof(UNKNOWNTAG));
	memset(&utEnd,0,sizeof(UNKNOWNTAG));
	utShowFrame.rhTagHeader.usTagCode=TAGCODE_SHOWFRAME;
	utEnd.rhTagHeader.usTagCode=TAGCODE_END;
	cswfFontFlash.AddTag(&utShowFrame);
	cswfFontFlash.AddTag(&utEnd);

	/*Save*/
	CreateFileName(szFontFileName,"font",usFontID,"swf");
	cswfFontFlash.Save(szFontFileName);

	if(ulGlyphs)
	{
		delete dtText.lprgtrpTextRecords[0];
		delete dtText.lprgtrpTextRecords;
	}

	return 0;
}
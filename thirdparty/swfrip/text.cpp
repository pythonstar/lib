#include "swf.h"

UI32 CSWF::ReadDefineText(DEFINETEXT *lpdtDefineText, UI8 fRGBA)
{
	UI32 ulTagLength=bsInFileStream.GetByteOffset();
	TEXTRECORD **lprgtrpTextRecords;

	memset(lpdtDefineText,0,sizeof(DEFINETEXT));
	memcpy(&lpdtDefineText->rhTagHeader,&rhTagHeader,sizeof(RECORDHEADER));

	lpdtDefineText->usCharacterID=bsInFileStream.ReadUI16();
	ReadRect(&lpdtDefineText->rTextBounds);
	ReadMatrix(&lpdtDefineText->mTextMatrix);
	nGlyphBits=lpdtDefineText->ubGlyphBits=bsInFileStream.ReadUI8();
	nAdvanceBits=lpdtDefineText->ubAdvanceBits=bsInFileStream.ReadUI8();
	
	lprgtrpTextRecords=new TEXTRECORD*[0x10000];

	while(bsInFileStream.GetUI8())
	{
		lprgtrpTextRecords[lpdtDefineText->ulNumTextRecords]=new TEXTRECORD;
		ReadTextRecord(lprgtrpTextRecords[lpdtDefineText->ulNumTextRecords],fRGBA);
		lpdtDefineText->ulNumTextRecords++;
	}

	lpdtDefineText->lprgtrpTextRecords=new TEXTRECORD*[lpdtDefineText->ulNumTextRecords];
	memcpy(lpdtDefineText->lprgtrpTextRecords,lprgtrpTextRecords,lpdtDefineText->ulNumTextRecords<<2);
	delete[] lprgtrpTextRecords;

	bsInFileStream.ReadUI8();
	ulTagLength=bsInFileStream.GetByteOffset()-ulTagLength;

	return ulTagLength;
}

UI32 CSWF::WriteDefineText(DEFINETEXT *lpdtDefineText, UI8 fRGBA, FILE *pfileSWF)
{
	if(!lpdtDefineText->rhTagHeader.ulLength)
		lpdtDefineText->rhTagHeader.ulLength=lpdtDefineText->ulNumTextRecords*(sizeof(TEXTRECORD)+0x1000);
	
	bsOutFileStream.Create(sizeof(DEFINETEXT)+lpdtDefineText->rhTagHeader.ulLength);

	bsOutFileStream.WriteUI16(lpdtDefineText->usCharacterID);
	WriteRect(&lpdtDefineText->rTextBounds);
	WriteMatrix(&lpdtDefineText->mTextMatrix);
	
	nGlyphBits=lpdtDefineText->ubGlyphBits;
	nAdvanceBits=lpdtDefineText->ubAdvanceBits; //unsignedbits and all
		
	bsOutFileStream.WriteUI8(lpdtDefineText->ubGlyphBits);
	bsOutFileStream.WriteUI8(lpdtDefineText->ubAdvanceBits);
	
	for(UI32 ulTextRecord=0; ulTextRecord<lpdtDefineText->ulNumTextRecords; ulTextRecord++)
		WriteTextRecord(lpdtDefineText->lprgtrpTextRecords[ulTextRecord],fRGBA,pfileSWF);

	bsOutFileStream.WriteUI8(0);
	lpdtDefineText->rhTagHeader.ulLength=bsOutFileStream.GetByteOffset();
	WriteTagHeader(&lpdtDefineText->rhTagHeader,pfileSWF);
	bsOutFileStream.WriteToFile(pfileSWF);

	return lpdtDefineText->rhTagHeader.ulLength;
}

UI32 CSWF::DeleteDefineText(DEFINETEXT *lpdtDefineText)
{
	for(UI32 ulTextRecord=0; ulTextRecord<lpdtDefineText->ulNumTextRecords; ulTextRecord++)
	{
		delete lpdtDefineText->lprgtrpTextRecords[ulTextRecord]->lprggeGlyphEntries;
		delete lpdtDefineText->lprgtrpTextRecords[ulTextRecord];
	}

	delete lpdtDefineText->lprgtrpTextRecords;

	return 0;
}

UI32 CSWF::ReadTextRecord(TEXTRECORD *lptrTextRecord, UI8 fRGBA)
{
	UI32 ulByteLength=bsInFileStream.GetByteOffset();

	memset(lptrTextRecord,0,sizeof(TEXTRECORD));
	
	lptrTextRecord->ubTextRecordType=(UI8)bsInFileStream.ReadUB(1);
	lptrTextRecord->ubStyleFlagsReserved=(UI8)bsInFileStream.ReadUB(3);
	lptrTextRecord->fStyleFlagsHasFont=(UI8)bsInFileStream.ReadUB(1);
	lptrTextRecord->fStyleFlagsHasColor=(UI8)bsInFileStream.ReadUB(1);
	lptrTextRecord->fStyleFlagsHasYOffset=(UI8)bsInFileStream.ReadUB(1);
	lptrTextRecord->fStyleFlagsHasXOffset=(UI8)bsInFileStream.ReadUB(1);
	
	if(lptrTextRecord->fStyleFlagsHasFont) lptrTextRecord->usFontID=bsInFileStream.ReadUI16();
	if(lptrTextRecord->fStyleFlagsHasColor) ReadRGB(&lptrTextRecord->rgbaTextColor,fRGBA);

	if(lptrTextRecord->fStyleFlagsHasXOffset) lptrTextRecord->ssXOffset=bsInFileStream.ReadSI16();
	if(lptrTextRecord->fStyleFlagsHasYOffset) lptrTextRecord->ssYOffset=bsInFileStream.ReadSI16();
	if(lptrTextRecord->fStyleFlagsHasFont) lptrTextRecord->usTextHeight=bsInFileStream.ReadUI16();
	
	lptrTextRecord->ubGlyphCount=bsInFileStream.ReadUI8();
	lptrTextRecord->lprggeGlyphEntries=new GLYPHENTRY[lptrTextRecord->ubGlyphCount];

	for(UI32 ulGlyphEntry=0; ulGlyphEntry<lptrTextRecord->ubGlyphCount; ulGlyphEntry++)
		ReadGlyphEntry(&lptrTextRecord->lprggeGlyphEntries[ulGlyphEntry]);

	bsInFileStream.SeekNextByte();
	ulByteLength=bsInFileStream.GetByteOffset()-ulByteLength;
	return ulByteLength;
}

UI32 CSWF::WriteTextRecord(TEXTRECORD *lptrTextRecord, UI8 fRGBA, FILE *pfileSWF)
{
	UI32 ulByteLength=bsOutFileStream.GetByteOffset();
	
	bsOutFileStream.WriteUB(lptrTextRecord->ubTextRecordType,1);
	bsOutFileStream.WriteUB(lptrTextRecord->ubStyleFlagsReserved,3);
	bsOutFileStream.WriteUB(lptrTextRecord->fStyleFlagsHasFont,1);
	bsOutFileStream.WriteUB(lptrTextRecord->fStyleFlagsHasColor,1);
	bsOutFileStream.WriteUB(lptrTextRecord->fStyleFlagsHasYOffset,1);
	bsOutFileStream.WriteUB(lptrTextRecord->fStyleFlagsHasXOffset,1);
	
	if(lptrTextRecord->fStyleFlagsHasFont) bsOutFileStream.WriteUI16(lptrTextRecord->usFontID);
	if(lptrTextRecord->fStyleFlagsHasColor) WriteRGB(&lptrTextRecord->rgbaTextColor,fRGBA);

	if(lptrTextRecord->fStyleFlagsHasXOffset) bsOutFileStream.WriteSI16(lptrTextRecord->ssXOffset);
	if(lptrTextRecord->fStyleFlagsHasYOffset) bsOutFileStream.WriteSI16(lptrTextRecord->ssYOffset);
	if(lptrTextRecord->fStyleFlagsHasFont) bsOutFileStream.WriteUI16(lptrTextRecord->usTextHeight);	
	
	bsOutFileStream.WriteUI8(lptrTextRecord->ubGlyphCount);
	
	for(UI32 ulGlyphEntry=0; ulGlyphEntry<lptrTextRecord->ubGlyphCount; ulGlyphEntry++)
		WriteGlyphEntry(&lptrTextRecord->lprggeGlyphEntries[ulGlyphEntry],pfileSWF);

	bsOutFileStream.SeekNextByte();
	ulByteLength=bsOutFileStream.GetByteOffset()-ulByteLength;
	return ulByteLength;
}

UI32 CSWF::ReadGlyphEntry(GLYPHENTRY *lpgeGlyphEntry)
{
	UI32 ulBitLength=bsInFileStream.GetBitOffset();

	lpgeGlyphEntry->ulGlyphIndex=bsInFileStream.ReadUB(nGlyphBits);
	lpgeGlyphEntry->slGlyphAdvance=bsInFileStream.ReadSB(nAdvanceBits);

	ulBitLength=bsInFileStream.GetBitOffset()-ulBitLength;
	return ulBitLength;
}

UI32 CSWF::WriteGlyphEntry(GLYPHENTRY *lpgeGlyphEntry, FILE *pfileSWF)
{
	UI32 ulBitLength=bsOutFileStream.GetBitOffset();

	bsOutFileStream.WriteUB(lpgeGlyphEntry->ulGlyphIndex,nGlyphBits);
	bsOutFileStream.WriteSB(lpgeGlyphEntry->slGlyphAdvance,nAdvanceBits);

	ulBitLength=bsOutFileStream.GetBitOffset()-ulBitLength;
	return ulBitLength;
}

UI32 CSWF::ExportTextDependancies(DEFINETEXT* lpdtDefineText, CSWF *lpcswfTextFlash)
{
	for(UI32 iTextRecord=0; iTextRecord<lpdtDefineText->ulNumTextRecords; iTextRecord++)
	{
		DEFINEFONT *lpdfDefineFont=(DEFINEFONT*)lprgvpCharacters[lpdtDefineText->lprgtrpTextRecords[iTextRecord]->usFontID];
		
		if(lpdfDefineFont)
		{
			lpcswfTextFlash->AddTag(lpdfDefineFont);

			if(lpdfDefineFont->rhTagHeader.usTagCode==TAGCODE_DEFINEFONT && lpdfDefineFont->lpdfiFontInfo)
				lpcswfTextFlash->AddTag(lpdfDefineFont->lpdfiFontInfo);
		}
	}

	return 0;
}

UI32 CSWF::SaveDefineTextAsSWF(DEFINETEXT* lpdtDefineText)
{
	UI32 ulLength;
	char szTextFileName[4096];
	CSWF cswfTextFlash;
	SI32 slWidth=(fhFrameHeader.srFrameSize.slXMax-fhFrameHeader.srFrameSize.slXMin)/2;
	SI32 slHeight=(fhFrameHeader.srFrameSize.slYMax-fhFrameHeader.srFrameSize.slYMin)/2;
	
	/*SWFFILEHEADER*/
	SWFFILEHEADER fhTextFileHeader;
	fhTextFileHeader.bSignature1='F';
	fhTextFileHeader.bSignature2='W';
	fhTextFileHeader.bSignature3='S';
	fhTextFileHeader.bVersion=6;
	fhTextFileHeader.ulFileLength=0;
	cswfTextFlash.SetFileHeader(&fhTextFileHeader);

	/*SETBACKGROUNDCOLOR*/
	SETBACKGROUNDCOLOR sbcTextBackground;
	sbcTextBackground.rhTagHeader.usTagCode=TAGCODE_SETBACKGROUNDCOLOR;
	sbcTextBackground.rhTagHeader.ulLength=3;
	sbcTextBackground.rgbBackgroundColor.ubRed=128;
	sbcTextBackground.rgbBackgroundColor.ubGreen=128;
	sbcTextBackground.rgbBackgroundColor.ubBlue=128;
	cswfTextFlash.AddTag(&sbcTextBackground);
	
	/*Text*/
	cswfTextFlash.AddTag(lpjtJPEGTables);
	ExportTextDependancies(lpdtDefineText,&cswfTextFlash);
	cswfTextFlash.AddTag(lpdtDefineText);

	/*SWFFRAMEHEADER*/
	SWFFRAMEHEADER fhTextFrameHeader;
	fhTextFrameHeader.sfpFrameRate=fhFrameHeader.sfpFrameRate;
	fhTextFrameHeader.usFrameCount=1;
	fhTextFrameHeader.srFrameSize.bNbits=31;
	fhTextFrameHeader.srFrameSize.slXMin=-1*slWidth;
	fhTextFrameHeader.srFrameSize.slYMin=-1*slHeight;
	fhTextFrameHeader.srFrameSize.slXMax=slWidth;
	fhTextFrameHeader.srFrameSize.slYMax=slHeight;
	cswfTextFlash.SetFrameHeader(&fhTextFrameHeader);
	
	/*PLACEOBJECT(2)*/
	MATRIX mMatrix;
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
	po2Place.usCharacterID=lpdtDefineText->usCharacterID;
	po2Place.usDepth=1;
	memcpy(&po2Place.mMatrix,&mMatrix,sizeof(MATRIX));
	cswfTextFlash.AddTag(&po2Place);

	/*SHOWFRAME and END*/
	UNKNOWNTAG utShowFrame;
	UNKNOWNTAG utEnd;
	memset(&utShowFrame,0,sizeof(UNKNOWNTAG));
	memset(&utEnd,0,sizeof(UNKNOWNTAG));
	utShowFrame.rhTagHeader.usTagCode=TAGCODE_SHOWFRAME;
	utEnd.rhTagHeader.usTagCode=TAGCODE_END;
	cswfTextFlash.AddTag(&utShowFrame);
	cswfTextFlash.AddTag(&utEnd);
	
	/*Save*/
	CreateFileName(szTextFileName,"text",lpdtDefineText->usCharacterID,"swf");
	cswfTextFlash.Save(szTextFileName);
	return 0;
}

void WriteTextCharacter(UI16 usCode, UI8 ubEncoding, FILE *pfileText)
{
	UI8 ubByte1, ubByte2;

	ubByte1=usCode>>8;
	ubByte2=usCode & 0x00FF;

	switch(ubEncoding)
	{
		case 0: /*unicode [little endian] (even if it's a short char, write both bytes)*/
			putc(ubByte2,pfileText);
			putc(ubByte1,pfileText);
			break;

		case 1: /*sjis [big endian] (if it's a short char, only write the low byte)*/
			
			if(ubByte1) putc(ubByte1,pfileText);
			putc(ubByte2,pfileText);
			break;
	}
}

void WriteTextLineBreak(UI8 ubEncoding, FILE *pfileText)
{
	switch(ubEncoding)
	{
		case 0: /*unicode line break 0D000A00*/
			putw(0x000A000D,pfileText);
			break;

		case 1: /*sjis linebreak 0D0A*/
			putc(0x0D,pfileText); 
			putc(0x0A,pfileText);
			break;
	}
}

UI32 CSWF::WriteDefineTextAsText(DEFINETEXT* lpdtDefineText, UI8 fStartUnicode, FILE *pfileText)
{
	DEFINEFONT *lpdfDefineFont;
	DEFINEFONT2 *lpdfDefineFont2;
	DEFINEFONTINFO *lpdfiFontInfo;
	GLYPHENTRY *lpgeGlyphEntry;
	UI8 ubEncoding; //0=Unicode, 1=SJIS
	//UI8 ubWide;
	UI16 usFontID=0, usCode;
	UI32 ulGlyphIndex;

	for(UI32 iTextRecord=0; iTextRecord<lpdtDefineText->ulNumTextRecords; iTextRecord++)
	{
		TEXTRECORD *lpTextRecord=lpdtDefineText->lprgtrpTextRecords[iTextRecord];
		if(lpTextRecord->usFontID) usFontID=lpTextRecord->usFontID;
		if(!usFontID) return 0;
				
		switch(((UNKNOWNTAG*)lprgvpCharacters[usFontID])->rhTagHeader.usTagCode)
		{
			case TAGCODE_DEFINEFONT:
				lpdfDefineFont=(DEFINEFONT*)lprgvpCharacters[usFontID];
				lpdfiFontInfo=lpdfDefineFont->lpdfiFontInfo;
				if(!lpdfiFontInfo) continue;
				
				if(lpdfDefineFont->lpdfiFontInfo->fFontFlagsShiftJIS)
					ubEncoding=1;
				else ubEncoding=0;
				//ubWide=lpdfDefineFont->lpdfiFontInfo->fFontFlagsWideCodes;
				break;

			case TAGCODE_DEFINEFONT2:
				lpdfDefineFont2=(DEFINEFONT2*)lprgvpCharacters[usFontID];
				lpdfiFontInfo=NULL;
				
				if(lpdfDefineFont2->fFontFlagsShiftJIS) 
					ubEncoding=1;
				else ubEncoding=0;
				//ubWide=lpdfDefineFont2->fFontFlagsWideCodes;
				break;
		}
				
		/*start unicode text*/
		if(ubEncoding==0 && fStartUnicode) 
		{
			WriteTextCharacter(0xFEFF,ubEncoding,pfileText);
			fStartUnicode=false;
		}

		for(UI32 iGlyphEntry=0; iGlyphEntry<lpTextRecord->ubGlyphCount; iGlyphEntry++)
		{
			lpgeGlyphEntry=&lpTextRecord->lprggeGlyphEntries[iGlyphEntry];
			ulGlyphIndex=lpgeGlyphEntry->ulGlyphIndex;
		
			if(lpdfiFontInfo) usCode=lpdfiFontInfo->lprgusCodeTable[ulGlyphIndex];
			else usCode=lpdfDefineFont2->lprgusCodeTable[ulGlyphIndex];

			WriteTextCharacter(usCode,ubEncoding,pfileText);
		}

		WriteTextLineBreak(ubEncoding,pfileText);
	}

	WriteTextLineBreak(ubEncoding,pfileText);

	return ubEncoding;
}

UI32 CSWF::SaveDefineTextAsTXT(DEFINETEXT* lpdtDefineText)
{
	char szTextFileName[4096];
	FILE *pfileText;

	CreateFileName(szTextFileName,"text",lpdtDefineText->usCharacterID,"txt");
	pfileText=fopen(szTextFileName,"wb");

	WriteDefineTextAsText(lpdtDefineText,true,pfileText);

	fclose(pfileText);

	return 0;
}

UI32 CSWF::SaveText()
{
	char szTextFileName[4096];
	DEFINETEXT *lpdtDefineText;
	FILE *pfileText;
	UI16 usTagCode;
	UI8 fFirst=true;

	CreateFileName(szTextFileName,"text",0,"txt");
	pfileText=fopen(szTextFileName,"wb");

	for(UI32 iTag=0; iTag<nTags; iTag++)
	{
		lpdtDefineText=(DEFINETEXT*)lprgvpTags[iTag];
		usTagCode=lpdtDefineText->rhTagHeader.usTagCode;

		if(usTagCode==TAGCODE_DEFINETEXT || usTagCode==TAGCODE_DEFINETEXT2)
		{
			WriteDefineTextAsText((DEFINETEXT*)lprgvpTags[iTag],fFirst,pfileText);
			fFirst=false;
		}
	}	

	fclose(pfileText);
	return 1;
}

/*edit text*/

UI32 CSWF::ReadDefineEditText(DEFINEEDITTEXT *lpdtDefineEditText)
{
	UI32 ulTagLength=bsInFileStream.GetByteOffset();

	memset(lpdtDefineEditText,0,sizeof(DEFINEEDITTEXT));
	memcpy(&lpdtDefineEditText->rhTagHeader,&rhTagHeader,sizeof(RECORDHEADER));
	
	lpdtDefineEditText->usCharacterID=bsInFileStream.ReadUI16();
	ReadRect(&lpdtDefineEditText->rBounds);

	lpdtDefineEditText->fWordWrap=bsInFileStream.ReadUB(1);
	lpdtDefineEditText->fMultiline=bsInFileStream.ReadUB(1);
	lpdtDefineEditText->fPassword=bsInFileStream.ReadUB(1);
	lpdtDefineEditText->fReadOnly=bsInFileStream.ReadUB(1);
	lpdtDefineEditText->fHasTextColor=bsInFileStream.ReadUB(1);
	lpdtDefineEditText->fHasMaxLength=bsInFileStream.ReadUB(1);
	lpdtDefineEditText->fHasFont=bsInFileStream.ReadUB(1);
	lpdtDefineEditText->fReserved1=bsInFileStream.ReadUB(1);
	lpdtDefineEditText->fAutoSize=bsInFileStream.ReadUB(1);
	lpdtDefineEditText->fHasLayout=bsInFileStream.ReadUB(1);
	lpdtDefineEditText->fNoSelect=bsInFileStream.ReadUB(1);
	lpdtDefineEditText->fBorder=bsInFileStream.ReadUB(1);
	lpdtDefineEditText->fReserved2=bsInFileStream.ReadUB(1);
	lpdtDefineEditText->fHTML=bsInFileStream.ReadUB(1);
	lpdtDefineEditText->fUseOutlines=bsInFileStream.ReadUB(1);

	lpdtDefineEditText->usFontID=bsInFileStream.ReadUI16();
	ReadRGB(&lpdtDefineEditText->rgbaTextColor,true);
	lpdtDefineEditText->usMaxLength=bsInFileStream.ReadUI16();
	lpdtDefineEditText->ubAlign=bsInFileStream.ReadUI8();
	lpdtDefineEditText->usLeftMargin=bsInFileStream.ReadUI16();
	lpdtDefineEditText->usRightMargin=bsInFileStream.ReadUI16();
	lpdtDefineEditText->usIndent=bsInFileStream.ReadUI16();
	lpdtDefineEditText->usLeading=bsInFileStream.ReadUI16();
	bsInFileStream.ReadString(lpdtDefineEditText->szVariableName);
	bsInFileStream.ReadString(lpdtDefineEditText->szInitialText);

	bsInFileStream.ReadUI8();
	ulTagLength=bsInFileStream.GetByteOffset()-ulTagLength;

	return ulTagLength;
}

UI32 CSWF::WriteDefineEditText(DEFINEEDITTEXT *lpdtDefineEditText, FILE *pfileSWF)
{
	bsOutFileStream.Create(sizeof(DEFINEEDITTEXT)+lpdtDefineEditText->rhTagHeader.ulLength);

	bsOutFileStream.WriteUI16(lpdtDefineEditText->usCharacterID);
	WriteRect(&lpdtDefineEditText->rBounds);
	
	bsOutFileStream.WriteUB(lpdtDefineEditText->fWordWrap,1);
	bsOutFileStream.WriteUB(lpdtDefineEditText->fMultiline,1);
	bsOutFileStream.WriteUB(lpdtDefineEditText->fPassword,1);
	bsOutFileStream.WriteUB(lpdtDefineEditText->fReadOnly,1);
	bsOutFileStream.WriteUB(lpdtDefineEditText->fHasTextColor,1);
	bsOutFileStream.WriteUB(lpdtDefineEditText->fHasMaxLength,1);
	bsOutFileStream.WriteUB(lpdtDefineEditText->fHasFont,1);
	bsOutFileStream.WriteUB(lpdtDefineEditText->fReserved1,1);
	bsOutFileStream.WriteUB(lpdtDefineEditText->fAutoSize,1);
	bsOutFileStream.WriteUB(lpdtDefineEditText->fHasLayout,1);
	bsOutFileStream.WriteUB(lpdtDefineEditText->fNoSelect,1);
	bsOutFileStream.WriteUB(lpdtDefineEditText->fBorder,1);
	bsOutFileStream.WriteUB(lpdtDefineEditText->fReserved2,1);
	bsOutFileStream.WriteUB(lpdtDefineEditText->fHTML,1);
	bsOutFileStream.WriteUB(lpdtDefineEditText->fUseOutlines,1);

	bsOutFileStream.WriteUI16(lpdtDefineEditText->usFontID);
	WriteRGB(&lpdtDefineEditText->rgbaTextColor,true);
	bsOutFileStream.WriteUI16(lpdtDefineEditText->usMaxLength);
	bsOutFileStream.WriteUI8(lpdtDefineEditText->ubAlign);
	bsOutFileStream.WriteUI16(lpdtDefineEditText->usLeftMargin);
	bsOutFileStream.WriteUI16(lpdtDefineEditText->usRightMargin);
	bsOutFileStream.WriteUI16(lpdtDefineEditText->usIndent);
	bsOutFileStream.WriteUI16(lpdtDefineEditText->usLeading);
	bsOutFileStream.WriteString(lpdtDefineEditText->szVariableName);
	bsOutFileStream.WriteString(lpdtDefineEditText->szInitialText);

	lpdtDefineEditText->rhTagHeader.ulLength=bsOutFileStream.GetByteOffset();
	WriteTagHeader(&lpdtDefineEditText->rhTagHeader,pfileSWF);
	bsOutFileStream.WriteToFile(pfileSWF);

	return lpdtDefineEditText->rhTagHeader.ulLength;
}
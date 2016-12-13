#include "swf.h"

/*Morph Shapes*/

UI32 CSWF::ReadMorphFillStyleArray(MORPHFILLSTYLEARRAY *lpmfsaFillStyleArray)
{
	UI32 ulByteLength=bsInFileStream.GetByteOffset();

	memset(lpmfsaFillStyleArray,0,sizeof(FILLSTYLEARRAY));

	lpmfsaFillStyleArray->usFillStyleCountExtended=bsInFileStream.ReadUI8();
	
	if(lpmfsaFillStyleArray->usFillStyleCountExtended==0xFF)
		lpmfsaFillStyleArray->usFillStyleCountExtended=bsInFileStream.ReadUI16();
	
	/*Allocate array of FILLSTYLE structs and read them*/
	if(lpmfsaFillStyleArray->usFillStyleCountExtended)
	{
		lpmfsaFillStyleArray->lprgmfsFillStyles=new MORPHFILLSTYLE[lpmfsaFillStyleArray->usFillStyleCountExtended];

		for(UI16 iCurFillStyle=0; iCurFillStyle<lpmfsaFillStyleArray->usFillStyleCountExtended; iCurFillStyle++)
			ReadMorphFillStyle(&lpmfsaFillStyleArray->lprgmfsFillStyles[iCurFillStyle]);
	}

	ulByteLength=bsInFileStream.GetByteOffset()-ulByteLength;

	return ulByteLength;
}

UI32 CSWF::WriteMorphFillStyleArray(MORPHFILLSTYLEARRAY *lpmfsaFillStyleArray)
{
	UI32 ulByteLength=bsOutFileStream.GetByteOffset();

	if(lpmfsaFillStyleArray->usFillStyleCountExtended<=0xFF)
		bsOutFileStream.WriteUI8((UI8)lpmfsaFillStyleArray->usFillStyleCountExtended);
	
	else /*Extended count*/
	{
		bsOutFileStream.WriteUI8(0xFF);
		bsOutFileStream.WriteUI16(lpmfsaFillStyleArray->usFillStyleCountExtended);
	}

	/*Write array of FILLSTYLE structs=*/
	if(lpmfsaFillStyleArray->usFillStyleCountExtended)
	{
		for(UI16 iCurFillStyle=0; iCurFillStyle<lpmfsaFillStyleArray->usFillStyleCountExtended; iCurFillStyle++)
			WriteMorphFillStyle(&lpmfsaFillStyleArray->lprgmfsFillStyles[iCurFillStyle]);
	}

	ulByteLength=bsOutFileStream.GetByteOffset()-ulByteLength;

	return ulByteLength;
}

UI32 CSWF::ReadMorphFillStyle(MORPHFILLSTYLE *lpmfsFillStyle)
{
	UI32 ulByteLength=bsInFileStream.GetByteOffset();

	memset(lpmfsFillStyle,0,sizeof(MORPHFILLSTYLE));

	lpmfsFillStyle->ubFillStyleType=bsInFileStream.ReadUI8();

	switch(lpmfsFillStyle->ubFillStyleType)
	{
		case FILLSTYLE_SOLID:
			ReadRGB(&lpmfsFillStyle->rgbaStartColor,true);
			ReadRGB(&lpmfsFillStyle->rgbaEndColor,true);
			break;

		case FILLSTYLE_LINEARGRADIENT:
		case FILLSTYLE_RADIALGRADIENT:
			ReadMatrix(&lpmfsFillStyle->mStartGradientMatrix);
			ReadMatrix(&lpmfsFillStyle->mEndGradientMatrix);
			ReadMorphGradient(&lpmfsFillStyle->mgGradient);
			break;

		case FILLSTYLE_REPEATINGBITMAP:
		case FILLSTYLE_CLIPPEDBITMAP:
		case FILLSTYLE_REPEATINGBITMAPNS:
		case FILLSTYLE_CLIPPEDBITMAPNS:
			lpmfsFillStyle->usBitmapID=bsInFileStream.ReadUI16();
			ReadMatrix(&lpmfsFillStyle->mStartBitmapMatrix);
			ReadMatrix(&lpmfsFillStyle->mEndBitmapMatrix);
			break;
	}

	ulByteLength=bsInFileStream.GetByteOffset()-ulByteLength;

	return ulByteLength;
}

UI32 CSWF::WriteMorphFillStyle(MORPHFILLSTYLE *lpmfsFillStyle)
{
	UI32 ulByteLength=bsOutFileStream.GetByteOffset();

	bsOutFileStream.WriteUI8(lpmfsFillStyle->ubFillStyleType);

	switch(lpmfsFillStyle->ubFillStyleType)
	{
		case FILLSTYLE_SOLID:
			WriteRGB(&lpmfsFillStyle->rgbaStartColor,true);
			WriteRGB(&lpmfsFillStyle->rgbaEndColor,true);
			break;

		case FILLSTYLE_LINEARGRADIENT:
		case FILLSTYLE_RADIALGRADIENT:
			WriteMatrix(&lpmfsFillStyle->mStartGradientMatrix);
			WriteMatrix(&lpmfsFillStyle->mEndGradientMatrix);
			WriteMorphGradient(&lpmfsFillStyle->mgGradient);
			break;

		case FILLSTYLE_REPEATINGBITMAP:
		case FILLSTYLE_CLIPPEDBITMAP:
		case FILLSTYLE_REPEATINGBITMAPNS:
		case FILLSTYLE_CLIPPEDBITMAPNS:
			bsOutFileStream.WriteUI16(lpmfsFillStyle->usBitmapID);
			WriteMatrix(&lpmfsFillStyle->mStartBitmapMatrix);
			WriteMatrix(&lpmfsFillStyle->mStartBitmapMatrix);
			break;
	}

	ulByteLength=bsOutFileStream.GetByteOffset()-ulByteLength;

	return ulByteLength;
}

UI32 CSWF::ReadMorphLineStyleArray(MORPHLINESTYLEARRAY *lpmlsaLineStyleArray)
{
	UI32 ulByteLength=bsInFileStream.GetByteOffset();

	memset(lpmlsaLineStyleArray,0,sizeof(LINESTYLEARRAY));
	
	lpmlsaLineStyleArray->usLineStyleCountExtended=bsInFileStream.ReadUI8();

	if(lpmlsaLineStyleArray->usLineStyleCountExtended==0xFF)
		lpmlsaLineStyleArray->usLineStyleCountExtended=bsInFileStream.ReadUI16();
	
	/*Allocate array of LINESTYLE structs and read them*/
	if(lpmlsaLineStyleArray->usLineStyleCountExtended)
	{
		lpmlsaLineStyleArray->lprgmlsLineStyles=new MORPHLINESTYLE[lpmlsaLineStyleArray->usLineStyleCountExtended];

		for(UI16 iCurLineStyle=0; iCurLineStyle<lpmlsaLineStyleArray->usLineStyleCountExtended; iCurLineStyle++)
			ReadMorphLineStyle(&lpmlsaLineStyleArray->lprgmlsLineStyles[iCurLineStyle]);
	}

	ulByteLength=bsInFileStream.GetByteOffset()-ulByteLength;

	return ulByteLength;
}

UI32 CSWF::WriteMorphLineStyleArray(MORPHLINESTYLEARRAY *lpmlsaLineStyleArray)
{
	UI32 ulByteLength=bsOutFileStream.GetByteOffset();
	
	if(lpmlsaLineStyleArray->usLineStyleCountExtended<=0xFF)
		bsOutFileStream.WriteUI8((UI8)lpmlsaLineStyleArray->usLineStyleCountExtended);
		
	else /*Extended count*/
	{
		bsOutFileStream.WriteUI8(0xFF);
		bsOutFileStream.WriteUI16(lpmlsaLineStyleArray->usLineStyleCountExtended);
	}

	/*Write array of LINESTYLE structs*/
	if(lpmlsaLineStyleArray->usLineStyleCountExtended)
	{
		for(UI16 iCurLineStyle=0; iCurLineStyle<lpmlsaLineStyleArray->usLineStyleCountExtended; iCurLineStyle++)
			WriteMorphLineStyle(&lpmlsaLineStyleArray->lprgmlsLineStyles[iCurLineStyle]);
	}

	ulByteLength=bsOutFileStream.GetByteOffset()-ulByteLength;

	return ulByteLength;
}

UI32 CSWF::ReadMorphLineStyle(MORPHLINESTYLE *lpmlsLineStyle)
{
	UI32 ulByteLength=bsInFileStream.GetByteOffset();

	memset(lpmlsLineStyle,0,sizeof(MORPHLINESTYLE));

	lpmlsLineStyle->usStartWidth=bsInFileStream.ReadUI16();
	lpmlsLineStyle->usEndWidth=bsInFileStream.ReadUI16();

	ReadRGB(&lpmlsLineStyle->rgbaStartColor,true);
	ReadRGB(&lpmlsLineStyle->rgbaEndColor,true);

	ulByteLength=bsInFileStream.GetByteOffset()-ulByteLength;

	return ulByteLength;
}

UI32 CSWF::WriteMorphLineStyle(MORPHLINESTYLE *lpmlsLineStyle)
{
	UI32 ulByteLength=bsOutFileStream.GetByteOffset();

	bsOutFileStream.WriteUI16(lpmlsLineStyle->usStartWidth);
	bsOutFileStream.WriteUI16(lpmlsLineStyle->usEndWidth);

	WriteRGB(&lpmlsLineStyle->rgbaStartColor,true);
	WriteRGB(&lpmlsLineStyle->rgbaEndColor,true);

	ulByteLength=bsOutFileStream.GetByteOffset()-ulByteLength;

	return ulByteLength;
}

UI32 CSWF::ReadMorphGradient(MORPHGRADIENT *lpmgGradient)
{
	UI32 ulGradientLength=bsInFileStream.GetByteOffset();

	memset(lpmgGradient,0,sizeof(MORPHGRADIENT));

	lpmgGradient->ubNumGradients=bsInFileStream.ReadUI8();
	
	/*Allocate array of MORPHGRADIENTRECORDs and read them*/
	if(lpmgGradient->ubNumGradients)
	{
		lpmgGradient->lprgmgrGradientRecords=new MORPHGRADIENTRECORD[lpmgGradient->ubNumGradients];

		for(UI8 iGradient=0; iGradient<lpmgGradient->ubNumGradients; iGradient++)
			ReadMorphGradientRecord(&lpmgGradient->lprgmgrGradientRecords[iGradient]);
	}
	
	ulGradientLength=bsInFileStream.GetByteOffset()-ulGradientLength;

	return ulGradientLength;
}

UI32 CSWF::WriteMorphGradient(MORPHGRADIENT *lpmgGradient)
{
	UI32 ulGradientLength=bsOutFileStream.GetByteOffset();

	bsOutFileStream.WriteUI8(lpmgGradient->ubNumGradients);
	
	/*Allocate array of GRADIENTRECORDs and write them*/
	if(lpmgGradient->ubNumGradients)
	{
		for(UI8 iGradient=0; iGradient<lpmgGradient->ubNumGradients; iGradient++)
			WriteMorphGradientRecord(&lpmgGradient->lprgmgrGradientRecords[iGradient]);
	}
	
	ulGradientLength=bsOutFileStream.GetByteOffset()-ulGradientLength;

	return ulGradientLength;
}

UI32 CSWF::ReadMorphGradientRecord(MORPHGRADIENTRECORD *lpmgrGradientRecord)
{
	UI32 ulGradientLength=bsInFileStream.GetByteOffset();

	memset(lpmgrGradientRecord,0,sizeof(GRADIENTRECORD));

	lpmgrGradientRecord->ubStartRatio=bsInFileStream.ReadUI8();
	ReadRGB(&lpmgrGradientRecord->rgbaStartColor,true);

	lpmgrGradientRecord->ubEndRatio=bsInFileStream.ReadUI8();
	ReadRGB(&lpmgrGradientRecord->rgbaEndColor,true);

	ulGradientLength=bsInFileStream.GetByteOffset()-ulGradientLength;

	return ulGradientLength;
}

UI32 CSWF::WriteMorphGradientRecord(MORPHGRADIENTRECORD *lpmgrGradientRecord)
{
	UI32 ulGradientLength=bsOutFileStream.GetByteOffset();

	bsOutFileStream.WriteUI8(lpmgrGradientRecord->ubStartRatio);
	WriteRGB(&lpmgrGradientRecord->rgbaStartColor,true);

	bsOutFileStream.WriteUI8(lpmgrGradientRecord->ubEndRatio);
	WriteRGB(&lpmgrGradientRecord->rgbaEndColor,true);

	ulGradientLength=bsOutFileStream.GetByteOffset()-ulGradientLength;

	return ulGradientLength;
}

UI32 CSWF::ReadDefineMorphShape(DEFINEMORPHSHAPE *lpdmsMorphShape)
{
	UI32 ulTagLength=bsInFileStream.GetByteOffset();

	memset(lpdmsMorphShape,0,sizeof(DEFINEMORPHSHAPE));
	memcpy(&lpdmsMorphShape->rhTagHeader,&rhTagHeader,sizeof(RECORDHEADER));
	
	lpdmsMorphShape->usCharacterID=bsInFileStream.ReadUI16();
	ReadRect(&lpdmsMorphShape->rStartBounds);
	ReadRect(&lpdmsMorphShape->rEndBounds);
	lpdmsMorphShape->ulOffset=bsInFileStream.ReadUI32();
	ReadMorphFillStyleArray(&lpdmsMorphShape->mfsaMorphFillStyleArray);
	ReadMorphLineStyleArray(&lpdmsMorphShape->mlsaMorphLineStyleArray);
	ReadShape(&lpdmsMorphShape->sStartEdges);
	ReadShape(&lpdmsMorphShape->sEndEdges);

	bsInFileStream.SeekNextByte();
	ulTagLength=bsInFileStream.GetByteOffset()-ulTagLength;

	return ulTagLength;
}

UI32 CSWF::WriteDefineMorphShape(DEFINEMORPHSHAPE *lpdmsMorphShape, FILE *pfileSWF)
{
	bsOutFileStream.Create(sizeof(DEFINEMORPHSHAPE)+lpdmsMorphShape->rhTagHeader.ulLength);

	bsOutFileStream.WriteUI16(lpdmsMorphShape->usCharacterID);
	WriteRect(&lpdmsMorphShape->rStartBounds);
	WriteRect(&lpdmsMorphShape->rEndBounds);
	bsOutFileStream.WriteUI32(lpdmsMorphShape->ulOffset);

	WriteMorphFillStyleArray(&lpdmsMorphShape->mfsaMorphFillStyleArray);
	WriteMorphLineStyleArray(&lpdmsMorphShape->mlsaMorphLineStyleArray);
	WriteShape(&lpdmsMorphShape->sStartEdges);
	WriteShape(&lpdmsMorphShape->sEndEdges);

	lpdmsMorphShape->rhTagHeader.ulLength=bsOutFileStream.GetByteOffset();
	WriteTagHeader(&lpdmsMorphShape->rhTagHeader,pfileSWF);
	bsOutFileStream.WriteToFile(pfileSWF);

	return bsOutFileStream.GetByteOffset();
}

UI32 CSWF::DeleteDefineMorphShape(DEFINEMORPHSHAPE *lpdmsMorphShape)
{
	void **lprgvpShapeRecords;
	UI32 ulNumShapeRecords;
	
	delete lpdmsMorphShape->mfsaMorphFillStyleArray.lprgmfsFillStyles;
	delete lpdmsMorphShape->mlsaMorphLineStyleArray.lprgmlsLineStyles;
	
	lprgvpShapeRecords=lpdmsMorphShape->sStartEdges.lprgvpShapeRecords;
	ulNumShapeRecords=lpdmsMorphShape->sStartEdges.ulNumShapeRecords;
	DeleteShapeRecords(lprgvpShapeRecords,ulNumShapeRecords);

	lprgvpShapeRecords=lpdmsMorphShape->sEndEdges.lprgvpShapeRecords;
	ulNumShapeRecords=lpdmsMorphShape->sEndEdges.ulNumShapeRecords;
	DeleteShapeRecords(lprgvpShapeRecords,ulNumShapeRecords);
	
	return 0;
}

UI32 CSWF::ExportMorphShapeDependancies(DEFINEMORPHSHAPE* lpdmsMorphShape, CSWF *lpcswfShapeFlash)
{
	/*In original array*/
	UI32 iShapeRecord;
	FILLSTYLEARRAY *lpfsaFillStyleArray;
	MORPHFILLSTYLEARRAY *lpmfsaFillStyleArray=&lpdmsMorphShape->mfsaMorphFillStyleArray;
	
	for(UI32 iFillStyle=0; iFillStyle<lpmfsaFillStyleArray->usFillStyleCountExtended; iFillStyle++)
	{
		UI16 usCharacterID=lpmfsaFillStyleArray->lprgmfsFillStyles[iFillStyle].usBitmapID;
		lpcswfShapeFlash->AddTag(lprgvpCharacters[usCharacterID]);
	}

	/*In StartEdges arrays*/
	for(iShapeRecord=0; iShapeRecord<lpdmsMorphShape->sStartEdges.ulNumShapeRecords; iShapeRecord++)
	{
		if(!((STYLECHANGERECORD*)lpdmsMorphShape->sStartEdges.lprgvpShapeRecords[iShapeRecord])->fTypeFlag &&
			((STYLECHANGERECORD*)lpdmsMorphShape->sStartEdges.lprgvpShapeRecords[iShapeRecord])->fStateNewStyles)
		{
			lpfsaFillStyleArray=&((STYLECHANGERECORD*)lpdmsMorphShape->sStartEdges.lprgvpShapeRecords[iShapeRecord])->fsaFillStyleArray;

			for(UI32 iFillStyle=0; iFillStyle<lpmfsaFillStyleArray->usFillStyleCountExtended; iFillStyle++)
			{
				UI16 usCharacterID=lpfsaFillStyleArray->lprgfsFillStyles[iFillStyle].usBitmapID;
				lpcswfShapeFlash->AddTag(lprgvpCharacters[usCharacterID]);
			}
		}
	}

	/*In EndEdges arrays*/
	for(iShapeRecord=0; iShapeRecord<lpdmsMorphShape->sEndEdges.ulNumShapeRecords; iShapeRecord++)
	{
		if(!((STYLECHANGERECORD*)lpdmsMorphShape->sEndEdges.lprgvpShapeRecords[iShapeRecord])->fTypeFlag &&
			((STYLECHANGERECORD*)lpdmsMorphShape->sEndEdges.lprgvpShapeRecords[iShapeRecord])->fStateNewStyles)
		{
			lpfsaFillStyleArray=&((STYLECHANGERECORD*)lpdmsMorphShape->sEndEdges.lprgvpShapeRecords[iShapeRecord])->fsaFillStyleArray;

			for(UI32 iFillStyle=0; iFillStyle<lpfsaFillStyleArray->usFillStyleCountExtended; iFillStyle++)
			{
				UI16 usCharacterID=lpfsaFillStyleArray->lprgfsFillStyles[iFillStyle].usBitmapID;
				lpcswfShapeFlash->AddTag(lprgvpCharacters[usCharacterID]);
			}
		}
	}

	return 1;
}

UI32 CSWF::SaveDefineMorphShape(DEFINEMORPHSHAPE* lpdmsMorphShape)
{
	UI32 ulLength, ulIncrement;
	SI32 slXMin, slXMax, slYMin, slYMax;
	UI16 usRatio, usFrames;
	char szShapeFileName[MAX_FILEPATH];
	CSWF cswfShapeFlash;
	
	usFrames=100;
	ulIncrement=0xFFFF/usFrames;
	usRatio=0;

	/*SWFFILEHEADER*/
	SWFFILEHEADER fhShapeFileHeader;
	fhShapeFileHeader.bSignature1='F';
	fhShapeFileHeader.bSignature2='W';
	fhShapeFileHeader.bSignature3='S';
	fhShapeFileHeader.bVersion=6;
	fhShapeFileHeader.ulFileLength=0;
	cswfShapeFlash.SetFileHeader(&fhShapeFileHeader);
	
	slXMin=MIN(lpdmsMorphShape->rStartBounds.slXMin,lpdmsMorphShape->rEndBounds.slXMin);
	slXMax=MAX(lpdmsMorphShape->rStartBounds.slXMax,lpdmsMorphShape->rEndBounds.slXMin);
	slYMin=MIN(lpdmsMorphShape->rStartBounds.slYMin,lpdmsMorphShape->rEndBounds.slXMin);
	slYMax=MAX(lpdmsMorphShape->rStartBounds.slYMax,lpdmsMorphShape->rEndBounds.slXMin);
	
	/*SWFFRAMEHEADER*/
	SWFFRAMEHEADER fhShapeFrameHeader;
	fhShapeFrameHeader.sfpFrameRate=2*fhFrameHeader.sfpFrameRate;
	fhShapeFrameHeader.usFrameCount=usFrames;
	fhShapeFrameHeader.srFrameSize.bNbits=31;
	fhShapeFrameHeader.srFrameSize.slXMin=0;
	fhShapeFrameHeader.srFrameSize.slYMin=0;
	fhShapeFrameHeader.srFrameSize.slXMax=slXMax-slXMin+100;
	fhShapeFrameHeader.srFrameSize.slYMax=slYMax-slYMin+100;
	cswfShapeFlash.SetFrameHeader(&fhShapeFrameHeader);

	/*SETBACKGROUNDCOLOR*/
	SETBACKGROUNDCOLOR sbcShapeBackground;
	sbcShapeBackground.rhTagHeader.usTagCode=TAGCODE_SETBACKGROUNDCOLOR;
	sbcShapeBackground.rhTagHeader.ulLength=3;
	sbcShapeBackground.rgbBackgroundColor.ubRed=128;
	sbcShapeBackground.rgbBackgroundColor.ubGreen=128;
	sbcShapeBackground.rgbBackgroundColor.ubBlue=128;
	cswfShapeFlash.AddTag(&sbcShapeBackground);
	
	/*Fill Images*/
	cswfShapeFlash.AddTag(lpjtJPEGTables);

	/*DEFINESHAPE*/
	ExportMorphShapeDependancies(lpdmsMorphShape,&cswfShapeFlash);
	cswfShapeFlash.AddTag(lpdmsMorphShape);

	/*PLACEOBJECT(2)*/
	PLACEOBJECT2 *lprgpo2Place=new PLACEOBJECT2[usFrames];

	MATRIX mMatrix;
	mMatrix.fHasScale=0;
	mMatrix.lfpScaleX=1;
	mMatrix.lfpScaleY=1;
	mMatrix.fHasRotate=0;
	mMatrix.ubNTranslateBits=31;
	mMatrix.slTranslateX=-1*slXMin+50;
	mMatrix.slTranslateY=-1*slYMin+50;
	
	memset(&lprgpo2Place[0],0,sizeof(PLACEOBJECT2));
	lprgpo2Place[0].rhTagHeader.usTagCode=TAGCODE_PLACEOBJECT2;
	lprgpo2Place[0].fPlaceFlagHasClipDepth=true;
	lprgpo2Place[0].fPlaceFlagHasRatio=true;
	lprgpo2Place[0].fPlaceFlagHasMatrix=true;
	lprgpo2Place[0].fPlaceFlagHasCharacter=true;
	lprgpo2Place[0].usDepth=1;
	lprgpo2Place[0].usRatio=usRatio;
	memcpy(&lprgpo2Place[0].mMatrix,&mMatrix,sizeof(MATRIX));
	lprgpo2Place[0].usCharacterID=lpdmsMorphShape->usCharacterID;	

	UNKNOWNTAG utShowFrame;
	memset(&utShowFrame,0,sizeof(UNKNOWNTAG));
	utShowFrame.rhTagHeader.usTagCode=TAGCODE_SHOWFRAME;

	for(UI16 usFrame=0; usFrame<usFrames; usFrame++)
	{
		if(usFrame)
		{
			memset(&lprgpo2Place[usFrame],0,sizeof(PLACEOBJECT2));
			lprgpo2Place[usFrame].rhTagHeader.usTagCode=TAGCODE_PLACEOBJECT2;
			lprgpo2Place[usFrame].fPlaceFlagHasClipDepth=true;
			lprgpo2Place[usFrame].fPlaceFlagHasRatio=true;
			lprgpo2Place[usFrame].fPlaceFlagMove=true;
			lprgpo2Place[usFrame].usDepth=1;
			lprgpo2Place[usFrame].usRatio=usRatio;
		}

		cswfShapeFlash.AddTag(&lprgpo2Place[usFrame]);
		cswfShapeFlash.AddTag(&utShowFrame);
		usRatio+=ulIncrement;
	}
	
	/*END*/
	UNKNOWNTAG utEnd;
	memset(&utEnd,0,sizeof(UNKNOWNTAG));
	utEnd.rhTagHeader.usTagCode=TAGCODE_END;
	cswfShapeFlash.AddTag(&utEnd);
	
	/*Save*/
	CreateFileName(szShapeFileName,"morph",lpdmsMorphShape->usCharacterID,"swf");
	cswfShapeFlash.Save(szShapeFileName);

	delete lprgpo2Place;

	return 0;
}
#include "swf.h"
#include "svg.h"

/*Shapes*/

UI32 CSWF::ReadDefineShape(DEFINESHAPE *lpdsShape, UI8 fRGBA)
{
	UI32 ulTagLength=bsInFileStream.GetByteOffset();

	memset(lpdsShape,0,sizeof(DEFINESHAPE));
	memcpy(&lpdsShape->rhTagHeader,&rhTagHeader,sizeof(RECORDHEADER));
	
	lpdsShape->usShapeID=bsInFileStream.ReadUI16();
	ReadRect(&lpdsShape->rShapeBounds);
	ReadShapeWithStyle(&lpdsShape->swsShapeWithStyle,fRGBA);

	bsInFileStream.SeekNextByte();
	ulTagLength=bsInFileStream.GetByteOffset()-ulTagLength;

	return ulTagLength;
}

UI32 CSWF::WriteDefineShape(DEFINESHAPE *lpdsShape, UI8 fRGBA, FILE *pfileSWF)
{
	bsOutFileStream.Create(sizeof(DEFINESHAPE)+lpdsShape->rhTagHeader.ulLength);

	bsOutFileStream.WriteUI16(lpdsShape->usShapeID);
	WriteRect(&lpdsShape->rShapeBounds);
	WriteShapeWithStyle(&lpdsShape->swsShapeWithStyle,fRGBA);

	if(fRGBA) lpdsShape->rhTagHeader.usTagCode=TAGCODE_DEFINESHAPE3;
	else lpdsShape->rhTagHeader.usTagCode=TAGCODE_DEFINESHAPE;
	lpdsShape->rhTagHeader.ulLength=bsOutFileStream.GetByteOffset();
	WriteTagHeader(&lpdsShape->rhTagHeader,pfileSWF);
	bsOutFileStream.WriteToFile(pfileSWF);

	return bsOutFileStream.GetByteOffset();
}

UI32 CSWF::ReadShape(SHAPE *lpsShape)
{
	UI32 ulByteLength=bsInFileStream.GetByteOffset();

	memset(lpsShape,0,sizeof(SHAPE));

	nFillBits=lpsShape->ubNumFillBits=(UI8)bsInFileStream.ReadUB(4);
	nLineBits=lpsShape->ubNumLineBits=(UI8)bsInFileStream.ReadUB(4);
	ReadShapeRecords((SHAPEWITHSTYLE*)lpsShape,false);
	
	bsInFileStream.SeekNextByte();
	ulByteLength=bsInFileStream.GetByteOffset()-ulByteLength;
	
	return ulByteLength;
}

UI32 CSWF::WriteShape(SHAPE *lpsShape)
{
	UI32 ulByteLength=bsOutFileStream.GetByteOffset();

	nFillBits=lpsShape->ubNumFillBits;//=BITSFORUNSIGNEDVALUE(lpswsShape->fsaFillStyleArray.usFillStyleCountExtended);
	nLineBits=lpsShape->ubNumLineBits;//=BITSFORUNSIGNEDVALUE(lpswsShape->lsaLineStyleArray.usLineStyleCountExtended);
	
	bsOutFileStream.WriteUB(lpsShape->ubNumFillBits,4);
	bsOutFileStream.WriteUB(lpsShape->ubNumLineBits,4);
	WriteShapeRecords((SHAPEWITHSTYLE*)lpsShape,false);
	
	bsOutFileStream.SeekNextByte();
	ulByteLength=bsOutFileStream.GetByteOffset()-ulByteLength;
	
	return ulByteLength;
}

UI32 CSWF::DeleteShapeRecords(void **lprgvpShapeRecords, UI32 ulNumShapeRecords)
{
	STYLECHANGERECORD *lpscrStyleChange;

	for(UI32 iShapeRecord=0; iShapeRecord<ulNumShapeRecords; iShapeRecord++)
	{
		/*STYLECHANGERECORD*/
		if(!((STRAIGHTEDGERECORD*)(lprgvpShapeRecords[iShapeRecord]))->fTypeFlag)
		{
			lpscrStyleChange=(STYLECHANGERECORD*)lprgvpShapeRecords[iShapeRecord];
			
			delete lpscrStyleChange->fsaFillStyleArray.lprgfsFillStyles;
			delete lpscrStyleChange->lsaLineStyleArray.lprglsLineStyles;
		}

		delete lprgvpShapeRecords[iShapeRecord];
	}

	delete lprgvpShapeRecords;
		
	return 0;
}

UI32 CSWF::ReadShapeWithStyle(SHAPEWITHSTYLE *lpswsShape, UI8 fRGBA)
{
	UI32 ulByteLength=bsInFileStream.GetByteOffset();

	memset(lpswsShape,0,sizeof(SHAPEWITHSTYLE));

	ReadFillStyleArray(&lpswsShape->fsaFillStyleArray,fRGBA);
	ReadLineStyleArray(&lpswsShape->lsaLineStyleArray,fRGBA);
	nFillBits=lpswsShape->ubNumFillBits=(UI8)bsInFileStream.ReadUB(4);
	nLineBits=lpswsShape->ubNumLineBits=(UI8)bsInFileStream.ReadUB(4);
	ReadShapeRecords(lpswsShape,fRGBA);
	
	bsInFileStream.SeekNextByte();
	ulByteLength=bsInFileStream.GetByteOffset()-ulByteLength;
	
	return ulByteLength;
}

UI32 CSWF::WriteShapeWithStyle(SHAPEWITHSTYLE *lpswsShape, UI8 fRGBA)
{
	UI32 ulByteLength=bsOutFileStream.GetByteOffset();

	WriteFillStyleArray(&lpswsShape->fsaFillStyleArray,fRGBA);
	WriteLineStyleArray(&lpswsShape->lsaLineStyleArray,fRGBA);
	nFillBits=lpswsShape->ubNumFillBits;//=BITSFORUNSIGNEDVALUE(lpswsShape->fsaFillStyleArray.usFillStyleCountExtended);
	nLineBits=lpswsShape->ubNumLineBits;//=BITSFORUNSIGNEDVALUE(lpswsShape->lsaLineStyleArray.usLineStyleCountExtended);
	
	bsOutFileStream.WriteUB(lpswsShape->ubNumFillBits,4);
	bsOutFileStream.WriteUB(lpswsShape->ubNumLineBits,4);
	WriteShapeRecords(lpswsShape,fRGBA);
	
	bsOutFileStream.SeekNextByte();
	ulByteLength=bsOutFileStream.GetByteOffset()-ulByteLength;
	
	return ulByteLength;
}

/*SHAPERECORDS*/

UI32 CSWF::ReadShapeRecords(SHAPEWITHSTYLE *lpswsShape, UI8 fRGBA)
{
	UI32 ulBitLength=bsInFileStream.GetByteOffset();
	void **lprgvpShapeRecords=new void*[100000];

	while(true)
	{
		/*Edge record*/
		if(bsInFileStream.GetUB(8) & 0x80)
		{
			/*STRAIGHTEDGERECORD*/
			if(bsInFileStream.GetUB(8) & 0x40)
			{
				lprgvpShapeRecords[lpswsShape->ulNumShapeRecords]=new STRAIGHTEDGERECORD;
				ReadStraightEdgeRecord((STRAIGHTEDGERECORD*)lprgvpShapeRecords[lpswsShape->ulNumShapeRecords]);
			}
			
			else /*CURVEDEDGERECORD*/
			{
				lprgvpShapeRecords[lpswsShape->ulNumShapeRecords]=new CURVEDEDGERECORD;
				ReadCurvedEdgeRecord((CURVEDEDGERECORD*)lprgvpShapeRecords[lpswsShape->ulNumShapeRecords]);
			}
		}	


		/*Non edge record*/
		else
		{
			/*STYLECHANGERECORD*/
			if(bsInFileStream.GetUB(8) & 0x7C) 
			{
				lprgvpShapeRecords[lpswsShape->ulNumShapeRecords]=new STYLECHANGERECORD;
				ReadStyleChangeRecord((STYLECHANGERECORD*)lprgvpShapeRecords[lpswsShape->ulNumShapeRecords],fRGBA);
			}

			else /*ENDSHAPERECORD*/
			{
				bsInFileStream.ReadUB(6);
				break;
			}
		}

		lpswsShape->ulNumShapeRecords++;
	}

	lpswsShape->lprgvpShapeRecords=new void*[lpswsShape->ulNumShapeRecords];
	memcpy(lpswsShape->lprgvpShapeRecords,lprgvpShapeRecords,lpswsShape->ulNumShapeRecords<<2);
	delete lprgvpShapeRecords;


	bsInFileStream.SeekNextByte();
	ulBitLength=bsInFileStream.GetByteOffset()-ulBitLength;

	return ulBitLength;
}

UI32 CSWF::WriteShapeRecords(SHAPEWITHSTYLE *lpswsShape, UI8 fRGBA)
{
	UI32 ulBitLength=bsOutFileStream.GetByteOffset();
		
	for(UI32 iShapeRecord=0; iShapeRecord<lpswsShape->ulNumShapeRecords; iShapeRecord++)
	{
		/*Edge record*/
		if(((STRAIGHTEDGERECORD*)(lpswsShape->lprgvpShapeRecords[iShapeRecord]))->fTypeFlag)
		{
			/*STRAIGHTEDGERECORD*/
			if(((STRAIGHTEDGERECORD*)(lpswsShape->lprgvpShapeRecords[iShapeRecord]))->fStraightFlag)
				WriteStraightEdgeRecord((STRAIGHTEDGERECORD*)lpswsShape->lprgvpShapeRecords[iShapeRecord]);
						
			else /*CURVEDEDGERECORD*/
				WriteCurvedEdgeRecord((CURVEDEDGERECORD*)lpswsShape->lprgvpShapeRecords[iShapeRecord]);
		}	

		/*STYLECHANGERECORD*/
		else 
			WriteStyleChangeRecord((STYLECHANGERECORD*)lpswsShape->lprgvpShapeRecords[iShapeRecord],fRGBA);	
	}
	
	/*ENDSHAPERECORD*/
	bsOutFileStream.WriteUB(0,6);

	bsOutFileStream.SeekNextByte();
	ulBitLength=bsOutFileStream.GetByteOffset()-ulBitLength;

	return ulBitLength;
}

UI32 CSWF::ReadStyleChangeRecord(STYLECHANGERECORD *lpscrStyleChange, UI8 fRGBA)
{
	UI32 ulBitLength=bsInFileStream.GetByteOffset();
	
	memset(lpscrStyleChange,0,sizeof(STYLECHANGERECORD));

	/*Read info*/
	lpscrStyleChange->fTypeFlag=(UI8)bsInFileStream.ReadUB(1);
	lpscrStyleChange->fStateNewStyles=(UI8)bsInFileStream.ReadUB(1);
	lpscrStyleChange->fStateLineStyle=(UI8)bsInFileStream.ReadUB(1);
	lpscrStyleChange->fStateFillStyle1=(UI8)bsInFileStream.ReadUB(1); 
	lpscrStyleChange->fStateFillStyle0=(UI8)bsInFileStream.ReadUB(1);
	lpscrStyleChange->fStateMoveTo=(UI8)bsInFileStream.ReadUB(1);

	/*Read move info*/
	if(lpscrStyleChange->fStateMoveTo)
	{
		lpscrStyleChange->ubMoveBits=(UI8)bsInFileStream.ReadUB(5);
		lpscrStyleChange->slMoveDeltaX=bsInFileStream.ReadSB(lpscrStyleChange->ubMoveBits);
		lpscrStyleChange->slMoveDeltaY=bsInFileStream.ReadSB(lpscrStyleChange->ubMoveBits);
	}

	/*Read fill styles*/
	if(lpscrStyleChange->fStateFillStyle0)
		lpscrStyleChange->ulFillStyle0=bsInFileStream.ReadUB(nFillBits);
	
	if(lpscrStyleChange->fStateFillStyle1)
		lpscrStyleChange->ulFillStyle1=bsInFileStream.ReadUB(nFillBits);
	
	/*Read line style*/
	if(lpscrStyleChange->fStateLineStyle)
		lpscrStyleChange->ulLineStyle=bsInFileStream.ReadUB(nLineBits);
	
	/*Read new styles*/
	if(lpscrStyleChange->fStateNewStyles)
	{
		/*Seek and end bitstream*/
		bsInFileStream.SeekNextByte();
		
		ReadFillStyleArray(&lpscrStyleChange->fsaFillStyleArray,fRGBA);
		ReadLineStyleArray(&lpscrStyleChange->lsaLineStyleArray,fRGBA);

		nFillBits=lpscrStyleChange->ubNumFillBits=(UI8)bsInFileStream.ReadUB(4);
		nLineBits=lpscrStyleChange->ubNumLineBits=(UI8)bsInFileStream.ReadUB(4);
	}

	ulBitLength=bsInFileStream.GetByteOffset()-ulBitLength;

	return ulBitLength;
}

UI32 CSWF::WriteStyleChangeRecord(STYLECHANGERECORD *lpscrStyleChange, UI8 fRGBA)
{
	UI32 ulBitLength=bsOutFileStream.GetByteOffset();
	
	/*Write info*/
	bsOutFileStream.WriteUB(lpscrStyleChange->fTypeFlag,1);
	bsOutFileStream.WriteUB(lpscrStyleChange->fStateNewStyles,1);
	bsOutFileStream.WriteUB(lpscrStyleChange->fStateLineStyle,1);
	bsOutFileStream.WriteUB(lpscrStyleChange->fStateFillStyle1,1); 
	bsOutFileStream.WriteUB(lpscrStyleChange->fStateFillStyle0,1);
	bsOutFileStream.WriteUB(lpscrStyleChange->fStateMoveTo,1);

	/*Write move info*/
	if(lpscrStyleChange->fStateMoveTo)
	{
		bsOutFileStream.WriteUB(lpscrStyleChange->ubMoveBits,5);
		bsOutFileStream.WriteSB(lpscrStyleChange->slMoveDeltaX,lpscrStyleChange->ubMoveBits);
		bsOutFileStream.WriteSB(lpscrStyleChange->slMoveDeltaY,lpscrStyleChange->ubMoveBits);
	}

	/*Write fill styles*/
	if(lpscrStyleChange->fStateFillStyle0)
		bsOutFileStream.WriteUB(lpscrStyleChange->ulFillStyle0,nFillBits);
	
	if(lpscrStyleChange->fStateFillStyle1)
		bsOutFileStream.WriteUB(lpscrStyleChange->ulFillStyle1,nFillBits);
	
	/*Write line style*/
	if(lpscrStyleChange->fStateLineStyle)
		bsOutFileStream.WriteUB(lpscrStyleChange->ulLineStyle,nLineBits);
	
	/*Write new styles*/
	if(lpscrStyleChange->fStateNewStyles)
	{
		/*Seek and end bitstream*/
		bsInFileStream.SeekNextByte();
		
		WriteFillStyleArray(&lpscrStyleChange->fsaFillStyleArray,fRGBA);
		WriteLineStyleArray(&lpscrStyleChange->lsaLineStyleArray,fRGBA);

		nFillBits=lpscrStyleChange->ubNumFillBits;//=BITSFORUNSIGNEDVALUE(lpscrStyleChange->fsaFillStyleArray.usFillStyleCountExtended);
		nLineBits=lpscrStyleChange->ubNumLineBits;//=BITSFORUNSIGNEDVALUE(lpscrStyleChange->lsaLineStyleArray.usLineStyleCountExtended);
		
		bsOutFileStream.WriteUB(lpscrStyleChange->ubNumFillBits,4);
		bsOutFileStream.WriteUB(lpscrStyleChange->ubNumLineBits,4);
	}

	ulBitLength=bsOutFileStream.GetByteOffset()-ulBitLength;

	return ulBitLength;
}

UI32 CSWF::ReadStraightEdgeRecord(STRAIGHTEDGERECORD *lpserStraightEdge)
{
	UI32 ulBitLength=bsInFileStream.GetByteOffset();
	UI8 ubBits;

	memset(lpserStraightEdge,0,sizeof(STRAIGHTEDGERECORD));

	/*Read info*/
	lpserStraightEdge->fTypeFlag=(UI8)bsInFileStream.ReadUB(1);
	lpserStraightEdge->fStraightFlag=(UI8)bsInFileStream.ReadUB(1);
	lpserStraightEdge->ubNumBits=(UI8)bsInFileStream.ReadUB(4);
	ubBits=lpserStraightEdge->ubNumBits+2;

	/*Read general line flag*/
	lpserStraightEdge->fGeneralLineFlag=(UI8)bsInFileStream.ReadUB(1);

	/*Read general line if flag is set*/
	if(lpserStraightEdge->fGeneralLineFlag)
	{
		lpserStraightEdge->slGeneralDeltaX=bsInFileStream.ReadSB(ubBits);
		lpserStraightEdge->slGeneralDeltaY=bsInFileStream.ReadSB(ubBits);
	}

	else /*If not a general line*/
	{
		lpserStraightEdge->fVertLineFlag=(UI8)bsInFileStream.ReadUB(1);
		
		if(lpserStraightEdge->fVertLineFlag)
			lpserStraightEdge->slVertDeltaY=bsInFileStream.ReadSB(ubBits);
		
		else
			lpserStraightEdge->slVertDeltaX=bsInFileStream.ReadSB(ubBits);
	}

	ulBitLength=bsInFileStream.GetByteOffset()-ulBitLength;

	return ulBitLength;
}

UI32 CSWF::WriteStraightEdgeRecord(STRAIGHTEDGERECORD *lpserStraightEdge)
{
	UI32 ulBitLength=bsOutFileStream.GetByteOffset();
	
	/*UI8 ubBits=0;
	ubBits=MAX(ubBits,BITSFORSIGNEDVALUE(lpserStraightEdge->slGeneralDeltaX));
	ubBits=MAX(ubBits,BITSFORSIGNEDVALUE(lpserStraightEdge->slGeneralDeltaY));
	ubBits=MAX(ubBits,BITSFORSIGNEDVALUE(lpserStraightEdge->slVertDeltaX));
	ubBits=MAX(ubBits,BITSFORSIGNEDVALUE(lpserStraightEdge->slVertDeltaY));
	lpserStraightEdge->ubNumBits=ubBits-2;*/

	/*Write info*/
	bsOutFileStream.WriteUB(lpserStraightEdge->fTypeFlag,1);
	bsOutFileStream.WriteUB(lpserStraightEdge->fStraightFlag,1);
	bsOutFileStream.WriteUB(lpserStraightEdge->ubNumBits,4);
	
	/*Write general line flag*/
	bsOutFileStream.WriteUB(lpserStraightEdge->fGeneralLineFlag,1);

	/*Write general line if flag is set*/
	if(lpserStraightEdge->fGeneralLineFlag)
	{
		bsOutFileStream.WriteSB(lpserStraightEdge->slGeneralDeltaX,lpserStraightEdge->ubNumBits+2);
		bsOutFileStream.WriteSB(lpserStraightEdge->slGeneralDeltaY,lpserStraightEdge->ubNumBits+2);
	}

	else /*If not a general line*/
	{
		bsOutFileStream.WriteUB(lpserStraightEdge->fVertLineFlag,1);
		
		if(lpserStraightEdge->fVertLineFlag)
			bsOutFileStream.WriteSB(lpserStraightEdge->slVertDeltaY,lpserStraightEdge->ubNumBits+2);
		
		else
			bsOutFileStream.WriteSB(lpserStraightEdge->slVertDeltaX,lpserStraightEdge->ubNumBits+2);
	}

	ulBitLength=bsOutFileStream.GetByteOffset()-ulBitLength;

	return ulBitLength;
}

UI32 CSWF::ReadCurvedEdgeRecord(CURVEDEDGERECORD *lpcerCurvedEdge)
{
	UI32 ulBitLength=bsInFileStream.GetByteOffset();
	UI8 ubBits;

	memset(lpcerCurvedEdge,0,sizeof(CURVEDEDGERECORD));

	/*Read info*/
	lpcerCurvedEdge->fTypeFlag=(UI8)bsInFileStream.ReadUB(1);
	lpcerCurvedEdge->fStraightFlag=(UI8)bsInFileStream.ReadUB(1);
	lpcerCurvedEdge->ubNumBits=(UI8)bsInFileStream.ReadUB(4);
	ubBits=lpcerCurvedEdge->ubNumBits+2;

	/*Read Deltas*/
	lpcerCurvedEdge->slControlDeltaX=bsInFileStream.ReadSB(ubBits);
	lpcerCurvedEdge->slControlDeltaY=bsInFileStream.ReadSB(ubBits);
	lpcerCurvedEdge->slAnchorDeltaX=bsInFileStream.ReadSB(ubBits);
	lpcerCurvedEdge->slAnchorDeltaY=bsInFileStream.ReadSB(ubBits);

	ulBitLength=bsInFileStream.GetByteOffset()-ulBitLength;

	return ulBitLength;
}

UI32 CSWF::WriteCurvedEdgeRecord(CURVEDEDGERECORD *lpcerCurvedEdge)
{
	UI32 ulBitLength=bsOutFileStream.GetByteOffset();
	
	/*UI8 ubBits=0;
	ubBits=MAX(ubBits,BITSFORSIGNEDVALUE(lpcerCurvedEdge->slControlDeltaX));
	ubBits=MAX(ubBits,BITSFORSIGNEDVALUE(lpcerCurvedEdge->slControlDeltaY));
	ubBits=MAX(ubBits,BITSFORSIGNEDVALUE(lpcerCurvedEdge->slAnchorDeltaX));
	ubBits=MAX(ubBits,BITSFORSIGNEDVALUE(lpcerCurvedEdge->slAnchorDeltaY));
	lpcerCurvedEdge->ubNumBits=ubBits-2;*/
	
	/*Write info*/
	bsOutFileStream.WriteUB(lpcerCurvedEdge->fTypeFlag,1);
	bsOutFileStream.WriteUB(lpcerCurvedEdge->fStraightFlag,1);
	bsOutFileStream.WriteUB(lpcerCurvedEdge->ubNumBits,4);
	
	/*Read Deltas*/
	bsOutFileStream.WriteSB(lpcerCurvedEdge->slControlDeltaX,lpcerCurvedEdge->ubNumBits+2);
	bsOutFileStream.WriteSB(lpcerCurvedEdge->slControlDeltaY,lpcerCurvedEdge->ubNumBits+2);
	bsOutFileStream.WriteSB(lpcerCurvedEdge->slAnchorDeltaX,lpcerCurvedEdge->ubNumBits+2);
	bsOutFileStream.WriteSB(lpcerCurvedEdge->slAnchorDeltaY,lpcerCurvedEdge->ubNumBits+2);

	ulBitLength=bsOutFileStream.GetByteOffset()-ulBitLength;

	return ulBitLength;
}

UI32 CSWF::ReadFillStyleArray(FILLSTYLEARRAY *lpfsaFillStyleArray, UI8 fRGBA)
{
	UI32 ulByteLength=bsInFileStream.GetByteOffset();

	memset(lpfsaFillStyleArray,0,sizeof(FILLSTYLEARRAY));

	lpfsaFillStyleArray->usFillStyleCountExtended=bsInFileStream.ReadUI8();
	
	if(lpfsaFillStyleArray->usFillStyleCountExtended==0xFF)
		lpfsaFillStyleArray->usFillStyleCountExtended=bsInFileStream.ReadUI16();
	
	/*Allocate array of FILLSTYLE structs and read them*/
	if(lpfsaFillStyleArray->usFillStyleCountExtended)
	{
		lpfsaFillStyleArray->lprgfsFillStyles=new FILLSTYLE[lpfsaFillStyleArray->usFillStyleCountExtended];

		for(UI16 iCurFillStyle=0; iCurFillStyle<lpfsaFillStyleArray->usFillStyleCountExtended; iCurFillStyle++)
			ReadFillStyle(&lpfsaFillStyleArray->lprgfsFillStyles[iCurFillStyle],fRGBA);
	}

	ulByteLength=bsInFileStream.GetByteOffset()-ulByteLength;

	return ulByteLength;
}

UI32 CSWF::WriteFillStyleArray(FILLSTYLEARRAY *lpfsaFillStyleArray, UI8 fRGBA)
{
	UI32 ulByteLength=bsOutFileStream.GetByteOffset();

	if(lpfsaFillStyleArray->usFillStyleCountExtended<=0xFF)
		bsOutFileStream.WriteUI8((UI8)lpfsaFillStyleArray->usFillStyleCountExtended);
	
	else /*Extended count*/
	{
		bsOutFileStream.WriteUI8(0xFF);
		bsOutFileStream.WriteUI16(lpfsaFillStyleArray->usFillStyleCountExtended);
	}

	/*Write array of FILLSTYLE structs=*/
	if(lpfsaFillStyleArray->usFillStyleCountExtended)
	{
		for(UI16 iCurFillStyle=0; iCurFillStyle<lpfsaFillStyleArray->usFillStyleCountExtended; iCurFillStyle++)
			WriteFillStyle(&lpfsaFillStyleArray->lprgfsFillStyles[iCurFillStyle],fRGBA);
	}

	ulByteLength=bsOutFileStream.GetByteOffset()-ulByteLength;

	return ulByteLength;
}

UI32 CSWF::ReadFillStyle(FILLSTYLE *lpfsFillStyle, UI8 fRGBA)
{
	UI32 ulByteLength=bsInFileStream.GetByteOffset();

	memset(lpfsFillStyle,0,sizeof(FILLSTYLE));

	lpfsFillStyle->ubFillStyleType=bsInFileStream.ReadUI8();

	switch(lpfsFillStyle->ubFillStyleType)
	{
		case FILLSTYLE_SOLID:
			ReadRGB(&lpfsFillStyle->rgbaColor,fRGBA);
			break;

		case FILLSTYLE_LINEARGRADIENT:
		case FILLSTYLE_RADIALGRADIENT:
			ReadMatrix(&lpfsFillStyle->mGradientMatrix);
			ReadGradient(&lpfsFillStyle->gGradient,fRGBA);
			break;

		case FILLSTYLE_REPEATINGBITMAP:
		case FILLSTYLE_CLIPPEDBITMAP:
		case FILLSTYLE_REPEATINGBITMAPNS:
		case FILLSTYLE_CLIPPEDBITMAPNS:
			lpfsFillStyle->usBitmapID=bsInFileStream.ReadUI16();
			ReadMatrix(&lpfsFillStyle->mBitmapMatrix);
			break;
	}

	ulByteLength=bsInFileStream.GetByteOffset()-ulByteLength;

	return ulByteLength;
}

UI32 CSWF::WriteFillStyle(FILLSTYLE *lpfsFillStyle, UI8 fRGBA)
{
	UI32 ulByteLength=bsOutFileStream.GetByteOffset();

	bsOutFileStream.WriteUI8(lpfsFillStyle->ubFillStyleType);

	switch(lpfsFillStyle->ubFillStyleType)
	{
		case FILLSTYLE_SOLID:
			WriteRGB(&lpfsFillStyle->rgbaColor,fRGBA);
			break;

		case FILLSTYLE_LINEARGRADIENT:
		case FILLSTYLE_RADIALGRADIENT:
			WriteMatrix(&lpfsFillStyle->mGradientMatrix);
			WriteGradient(&lpfsFillStyle->gGradient,fRGBA);
			break;

		case FILLSTYLE_REPEATINGBITMAP:
		case FILLSTYLE_CLIPPEDBITMAP:
		case FILLSTYLE_REPEATINGBITMAPNS:
		case FILLSTYLE_CLIPPEDBITMAPNS:
			bsOutFileStream.WriteUI16(lpfsFillStyle->usBitmapID);
			WriteMatrix(&lpfsFillStyle->mBitmapMatrix);
			break;
	}

	ulByteLength=bsOutFileStream.GetByteOffset()-ulByteLength;

	return ulByteLength;
}

UI32 CSWF::ReadLineStyleArray(LINESTYLEARRAY *lplsaLineStyleArray, UI8 fRGBA)
{
	UI32 ulByteLength=bsInFileStream.GetByteOffset();

	memset(lplsaLineStyleArray,0,sizeof(LINESTYLEARRAY));
	
	lplsaLineStyleArray->usLineStyleCountExtended=bsInFileStream.ReadUI8();

	if(lplsaLineStyleArray->usLineStyleCountExtended==0xFF)
		lplsaLineStyleArray->usLineStyleCountExtended=bsInFileStream.ReadUI16();
	
	/*Allocate array of LINESTYLE structs and read them*/
	if(lplsaLineStyleArray->usLineStyleCountExtended)
	{
		lplsaLineStyleArray->lprglsLineStyles=new LINESTYLE[lplsaLineStyleArray->usLineStyleCountExtended];

		for(UI16 iCurLineStyle=0; iCurLineStyle<lplsaLineStyleArray->usLineStyleCountExtended; iCurLineStyle++)
			ReadLineStyle(&lplsaLineStyleArray->lprglsLineStyles[iCurLineStyle],fRGBA);
	}

	ulByteLength=bsInFileStream.GetByteOffset()-ulByteLength;

	return ulByteLength;
}

UI32 CSWF::WriteLineStyleArray(LINESTYLEARRAY *lplsaLineStyleArray, UI8 fRGBA)
{
	UI32 ulByteLength=bsOutFileStream.GetByteOffset();
	
	if(lplsaLineStyleArray->usLineStyleCountExtended<=0xFF)
		bsOutFileStream.WriteUI8((UI8)lplsaLineStyleArray->usLineStyleCountExtended);
		
	else /*Extended count*/
	{
		bsOutFileStream.WriteUI8(0xFF);
		bsOutFileStream.WriteUI16(lplsaLineStyleArray->usLineStyleCountExtended);
	}

	/*Write array of LINESTYLE structs*/
	if(lplsaLineStyleArray->usLineStyleCountExtended)
	{
		for(UI16 iCurLineStyle=0; iCurLineStyle<lplsaLineStyleArray->usLineStyleCountExtended; iCurLineStyle++)
			WriteLineStyle(&lplsaLineStyleArray->lprglsLineStyles[iCurLineStyle],fRGBA);
	}

	ulByteLength=bsOutFileStream.GetByteOffset()-ulByteLength;

	return ulByteLength;
}

UI32 CSWF::ReadLineStyle(LINESTYLE *lplsLineStyle, UI8 fRGBA)
{
	UI32 ulByteLength=bsInFileStream.GetByteOffset();

	memset(lplsLineStyle,0,sizeof(LINESTYLE));

	lplsLineStyle->usWidth=bsInFileStream.ReadUI16();

	ReadRGB(&lplsLineStyle->rgbaColor,fRGBA);

	ulByteLength=bsInFileStream.GetByteOffset()-ulByteLength;

	return ulByteLength;
}

UI32 CSWF::WriteLineStyle(LINESTYLE *lplsLineStyle, UI8 fRGBA)
{
	UI32 ulByteLength=bsOutFileStream.GetByteOffset();

	bsOutFileStream.WriteUI16(lplsLineStyle->usWidth);

	WriteRGB(&lplsLineStyle->rgbaColor,fRGBA);

	ulByteLength=bsOutFileStream.GetByteOffset()-ulByteLength;

	return ulByteLength;
}

UI32 CSWF::ReadGradient(GRADIENT *lpgGradient, UI8 fRGBA)
{
	UI32 ulGradientLength=bsInFileStream.GetByteOffset();

	memset(lpgGradient,0,sizeof(GRADIENT));

	lpgGradient->ubNumGradients=bsInFileStream.ReadUI8();
	
	/*Allocate array of GRADIENTRECORDs and read them*/
	if(lpgGradient->ubNumGradients)
	{
		lpgGradient->lprggrGradientRecords=new GRADIENTRECORD[lpgGradient->ubNumGradients];

		for(UI8 iGradient=0; iGradient<lpgGradient->ubNumGradients; iGradient++)
			ReadGradientRecord(&lpgGradient->lprggrGradientRecords[iGradient],fRGBA);
	}
	
	ulGradientLength=bsInFileStream.GetByteOffset()-ulGradientLength;

	return ulGradientLength;
}

UI32 CSWF::WriteGradient(GRADIENT *lpgGradient, UI8 fRGBA)
{
	UI32 ulGradientLength=bsOutFileStream.GetByteOffset();

	bsOutFileStream.WriteUI8(lpgGradient->ubNumGradients);
	
	/*Allocate array of GRADIENTRECORDs and read them*/
	if(lpgGradient->ubNumGradients)
	{
		for(UI8 iGradient=0; iGradient<lpgGradient->ubNumGradients; iGradient++)
			WriteGradientRecord(&lpgGradient->lprggrGradientRecords[iGradient],fRGBA);
	}
	
	ulGradientLength=bsOutFileStream.GetByteOffset()-ulGradientLength;

	return ulGradientLength;
}

UI32 CSWF::ReadGradientRecord(GRADIENTRECORD *lpgrGradientRecord, UI8 fRGBA)
{
	UI32 ulGradientLength=bsInFileStream.GetByteOffset();

	memset(lpgrGradientRecord,0,sizeof(GRADIENTRECORD));

	lpgrGradientRecord->ubRatio=bsInFileStream.ReadUI8();

	ReadRGB(&lpgrGradientRecord->rgbaColor,fRGBA);

	ulGradientLength=bsInFileStream.GetByteOffset()-ulGradientLength;

	return ulGradientLength;
}

UI32 CSWF::WriteGradientRecord(GRADIENTRECORD *lpgrGradientRecord, UI8 fRGBA)
{
	UI32 ulGradientLength=bsOutFileStream.GetByteOffset();

	bsOutFileStream.WriteUI8(lpgrGradientRecord->ubRatio);

	WriteRGB(&lpgrGradientRecord->rgbaColor,fRGBA);

	ulGradientLength=bsOutFileStream.GetByteOffset()-ulGradientLength;

	return ulGradientLength;
}

UI32 CSWF::ExportShapeDependancies(DEFINESHAPE* lpdsShape, CSWF *lpcswfShapeFlash)
{
	/*In original array*/
	FILLSTYLEARRAY *lpfsaFillStyleArray=&lpdsShape->swsShapeWithStyle.fsaFillStyleArray;
	
	for(UI32 iFillStyle=0; iFillStyle<lpfsaFillStyleArray->usFillStyleCountExtended; iFillStyle++)
	{
		UI16 usCharacterID=lpfsaFillStyleArray->lprgfsFillStyles[iFillStyle].usBitmapID;
		lpcswfShapeFlash->AddTag(lprgvpCharacters[usCharacterID]);
	}

	/*In STYLECHANGERECORD arrays*/
	for(UI32 iShapeRecord=0; iShapeRecord<lpdsShape->swsShapeWithStyle.ulNumShapeRecords; iShapeRecord++)
	{
		if(!((STYLECHANGERECORD*)lpdsShape->swsShapeWithStyle.lprgvpShapeRecords[iShapeRecord])->fTypeFlag &&
			((STYLECHANGERECORD*)lpdsShape->swsShapeWithStyle.lprgvpShapeRecords[iShapeRecord])->fStateNewStyles)
		{
			lpfsaFillStyleArray=&((STYLECHANGERECORD*)lpdsShape->swsShapeWithStyle.lprgvpShapeRecords[iShapeRecord])->fsaFillStyleArray;

			for(UI32 iFillStyle=0; iFillStyle<lpfsaFillStyleArray->usFillStyleCountExtended; iFillStyle++)
			{
				UI16 usCharacterID=lpfsaFillStyleArray->lprgfsFillStyles[iFillStyle].usBitmapID;
				lpcswfShapeFlash->AddTag(lprgvpCharacters[usCharacterID]);
			}
		}
	}

	return 1;
}

UI32 CSWF::SaveDefineShape(DEFINESHAPE* lpdsShape)
{
	UI32 ulLength;
	char szShapeFileName[4096];
	CSWF cswfShapeFlash;
	
	/*SWFFILEHEADER*/
	SWFFILEHEADER fhShapeFileHeader;
	fhShapeFileHeader.bSignature1='F';
	fhShapeFileHeader.bSignature2='W';
	fhShapeFileHeader.bSignature3='S';
	fhShapeFileHeader.bVersion=6;
	fhShapeFileHeader.ulFileLength=0;
	cswfShapeFlash.SetFileHeader(&fhShapeFileHeader);
	
	/*SWFFRAMEHEADER*/
	SWFFRAMEHEADER fhShapeFrameHeader;
	fhShapeFrameHeader.sfpFrameRate=fhFrameHeader.sfpFrameRate;
	fhShapeFrameHeader.usFrameCount=1;
	fhShapeFrameHeader.srFrameSize.bNbits=31;
	fhShapeFrameHeader.srFrameSize.slXMin=0;
	fhShapeFrameHeader.srFrameSize.slYMin=0;
	fhShapeFrameHeader.srFrameSize.slXMax=lpdsShape->rShapeBounds.slXMax-lpdsShape->rShapeBounds.slXMin+100;
	fhShapeFrameHeader.srFrameSize.slYMax=lpdsShape->rShapeBounds.slYMax-lpdsShape->rShapeBounds.slYMin+100;
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
	ExportShapeDependancies(lpdsShape,&cswfShapeFlash);
	cswfShapeFlash.AddTag(lpdsShape);

	/*PLACEOBJECT(2)*/
	MATRIX mMatrix;
	mMatrix.fHasScale=0;
	mMatrix.lfpScaleX=1;
	mMatrix.lfpScaleY=1;
	mMatrix.fHasRotate=0;
	mMatrix.ubNTranslateBits=31;
	mMatrix.slTranslateX=-1*lpdsShape->rShapeBounds.slXMin+50;
	mMatrix.slTranslateY=-1*lpdsShape->rShapeBounds.slYMin+50;

	PLACEOBJECT2 po2Place;
	memset(&po2Place,0,sizeof(PLACEOBJECT2));
	po2Place.rhTagHeader.usTagCode=TAGCODE_PLACEOBJECT2;
	po2Place.fPlaceFlagHasMatrix=true;
	po2Place.fPlaceFlagHasCharacter=true;
	po2Place.fPlaceFlagHasClipDepth=true;
	po2Place.usCharacterID=lpdsShape->usShapeID;
	po2Place.usDepth=1;
	memcpy(&po2Place.mMatrix,&mMatrix,sizeof(MATRIX));
	cswfShapeFlash.AddTag(&po2Place);
	
	/*SHOWFRAME and END*/
	UNKNOWNTAG utShowFrame;
	UNKNOWNTAG utEnd;
	memset(&utShowFrame,0,sizeof(UNKNOWNTAG));
	memset(&utEnd,0,sizeof(UNKNOWNTAG));
	utShowFrame.rhTagHeader.usTagCode=TAGCODE_SHOWFRAME;
	utEnd.rhTagHeader.usTagCode=TAGCODE_END;
	cswfShapeFlash.AddTag(&utShowFrame);
	cswfShapeFlash.AddTag(&utEnd);
	
	/*Save*/
	CreateFileName(szShapeFileName,"shape",lpdsShape->usShapeID,"swf");
	cswfShapeFlash.Save(szShapeFileName);

	return 0;
}

UI32 CSWF::WriteDefineShapeAsSVG(DEFINESHAPE *lpdsShape, UI8 fDisplay, FILE *pfileSVG, PLACEOBJECT2 *lppo2Place)
{
	FILLSTYLE *lprgfsFillStyles;
	LINESTYLE *lprglsLineStyles;
	CEdgeList elEdgeList, *lprgelPaths;
	UI32 ulNumPaths=0;

	lprgfsFillStyles=lpdsShape->swsShapeWithStyle.fsaFillStyleArray.lprgfsFillStyles;
	lprglsLineStyles=lpdsShape->swsShapeWithStyle.lsaLineStyleArray.lprglsLineStyles;
 
	if(!fDisplay) fprintf(pfileSVG,"<defs>\n\n");

	/*shape group*/
	fprintf(pfileSVG,"<!-- Shape %i -->\n\n",lpdsShape->usShapeID);
	fprintf(pfileSVG,"<g id=\"shape_%i\"",lpdsShape->usShapeID);
	if(lppo2Place) WriteTransform(&lppo2Place->mMatrix,pfileSVG);
	fprintf(pfileSVG," style=\"stroke-linecap: round; stroke-linejoin: round;\">\n\n");
	
	/*create edge list and convert it to SVG compatible format*/
	elEdgeList.DefineShapeToEdges(lpdsShape);
	elEdgeList.ConvertToSingleFill();
	ulNumPaths=elEdgeList.Enumerate(&lprgelPaths);

	/*draw each path's (fill & lines)*/
	for(UI32 iPath=0; iPath<ulNumPaths; iPath++)
	{
		WriteSVGPaths(lpdsShape,&lprgelPaths[iPath],false,pfileSVG,lppo2Place);
		WriteSVGPaths(lpdsShape,&lprgelPaths[iPath],true,pfileSVG,lppo2Place);
	}

	delete[] lprgelPaths;

	fprintf(pfileSVG,"</g>\n\n");

	if(!fDisplay) fprintf(pfileSVG,"</defs>\n\n");

	return 0;
}

UI32 CSWF::SaveDefineShapeAsSVG(DEFINESHAPE *lpdsShape)
{
	FILE *pfileSVG;
	char szShapeFileName[4096];
	double sdXMin, sdXMax, sdYMin, sdYMax, sdWidth, sdHeight;

	/*open file*/
	CreateFileName(szShapeFileName,"shape",lpdsShape->usShapeID,"svg");
	pfileSVG=fopen(szShapeFileName,"w");

	/*dimension calculations*/
	sdXMin=double(lpdsShape->rShapeBounds.slXMin)/20;
	sdXMax=double(lpdsShape->rShapeBounds.slXMax)/20;
	sdYMin=double(lpdsShape->rShapeBounds.slYMin)/20;
	sdYMax=double(lpdsShape->rShapeBounds.slYMax)/20;
	sdWidth=sdXMax-sdXMin;
	sdHeight=sdYMax-sdYMin;
	
	/*info*/
	fprintf(pfileSVG,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	fprintf(pfileSVG,"<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\n");
	fprintf(pfileSVG,"\"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg11.dtd\">\n\n");
	fprintf(pfileSVG,"<!-- Generated by SWFRIP -->\n\n");
	
	/*svg*/
	fprintf(pfileSVG,"<svg width=\"%0.2f\" height=\"%0.2f\" viewBox=\"%0.2f %0.2f %0.2f %0.2f\">\n\n",
			sdWidth,sdHeight,sdXMin,sdYMin,sdWidth,sdHeight);

	WriteDefineShapeAsSVG(lpdsShape,true,pfileSVG);

	/*end file*/
	fprintf(pfileSVG,"</svg>");
	fclose(pfileSVG);

	/*so szLastRes is this shape*/
	CreateFileName(szShapeFileName,"shape",lpdsShape->usShapeID,"svg");

	return 0;
}
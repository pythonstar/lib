#include "swf.h"

UI32 CSWF::ReadRect(SRECT *lpfrRect)
{	
	UI32 ulRectLength=bsInFileStream.GetByteOffset();
		
	lpfrRect->bNbits=(UI8)bsInFileStream.ReadUB(5);
		
	lpfrRect->slXMin=bsInFileStream.ReadSB(lpfrRect->bNbits);
	lpfrRect->slXMax=bsInFileStream.ReadSB(lpfrRect->bNbits);
	lpfrRect->slYMin=bsInFileStream.ReadSB(lpfrRect->bNbits);
	lpfrRect->slYMax=bsInFileStream.ReadSB(lpfrRect->bNbits);
	
	bsInFileStream.SeekNextByte();
	ulRectLength=bsInFileStream.GetByteOffset()-ulRectLength;
	
	return ulRectLength;
}

UI32 CSWF::WriteRect(SRECT *lpfrRect)
{
	UI8 ubBits;
	UI32 ulRectSize=bsOutFileStream.GetByteOffset();

	/*Calculate required bits*/
	ubBits=0;
	ubBits=MAX(ubBits,BITSFORSIGNEDVALUE(lpfrRect->slXMin));
	ubBits=MAX(ubBits,BITSFORSIGNEDVALUE(lpfrRect->slXMax));
	ubBits=MAX(ubBits,BITSFORSIGNEDVALUE(lpfrRect->slYMin));
	ubBits=MAX(ubBits,BITSFORSIGNEDVALUE(lpfrRect->slYMax));
	lpfrRect->bNbits=ubBits;
	
	bsOutFileStream.WriteUB(lpfrRect->bNbits,5);
	bsOutFileStream.WriteSB(lpfrRect->slXMin,lpfrRect->bNbits);
	bsOutFileStream.WriteSB(lpfrRect->slXMax,lpfrRect->bNbits);
	bsOutFileStream.WriteSB(lpfrRect->slYMin,lpfrRect->bNbits);
	bsOutFileStream.WriteSB(lpfrRect->slYMax,lpfrRect->bNbits);

	bsOutFileStream.SeekNextByte();
	ulRectSize=bsOutFileStream.GetByteOffset()-ulRectSize;

	return ulRectSize;
}

UI32 CSWF::ReadRGB(RGBA *lprgbaColor, UI8 fAlpha)
{
	lprgbaColor->ubRed=bsInFileStream.ReadUI8();
	lprgbaColor->ubGreen=bsInFileStream.ReadUI8();
	lprgbaColor->ubBlue=bsInFileStream.ReadUI8();

	if(fAlpha) 
	{
		lprgbaColor->ubAlpha=bsInFileStream.ReadUI8();
		return 4*sizeof(UI8);
	}

	else lprgbaColor->ubAlpha=255;

	return 3*sizeof(UI8);
}

UI32 CSWF::WriteRGB(RGBA *lprgbaColor, UI8 fAlpha)
{
	bsOutFileStream.WriteUI8(lprgbaColor->ubRed);
	bsOutFileStream.WriteUI8(lprgbaColor->ubGreen);
	bsOutFileStream.WriteUI8(lprgbaColor->ubBlue);
	if(fAlpha) 
	{
		bsOutFileStream.WriteUI8(lprgbaColor->ubAlpha);
		return 4*sizeof(UI8);
	}

	return 3*sizeof(UI8);
}

UI32 CSWF::ReadMatrix(MATRIX *lpmMatrix)
{
	UI32 ulMatrixLength=bsInFileStream.GetByteOffset();

	memset(lpmMatrix,0,sizeof(MATRIX));

	lpmMatrix->fHasScale=(UI8)bsInFileStream.ReadUB(1);
	
	/*Read scale info*/
	if(lpmMatrix->fHasScale)
	{
		lpmMatrix->ubNScaleBits=(UI8)bsInFileStream.ReadUB(5);
		lpmMatrix->lfpScaleX=bsInFileStream.ReadFB(lpmMatrix->ubNScaleBits);
		lpmMatrix->lfpScaleY=bsInFileStream.ReadFB(lpmMatrix->ubNScaleBits);
	}

	else lpmMatrix->lfpScaleX=lpmMatrix->lfpScaleY=FLOATTOFP32(1.0);

	lpmMatrix->fHasRotate=(UI8)bsInFileStream.ReadUB(1);

	/*Read rotate info*/
	if(lpmMatrix->fHasRotate)
	{
		lpmMatrix->ubNRotateBits=(UI8)bsInFileStream.ReadUB(5);
		lpmMatrix->lfpRotateSkew0=bsInFileStream.ReadFB(lpmMatrix->ubNRotateBits);
		lpmMatrix->lfpRotateSkew1=bsInFileStream.ReadFB(lpmMatrix->ubNRotateBits);
	}

	lpmMatrix->ubNTranslateBits=(UI8)bsInFileStream.ReadUB(5);
	lpmMatrix->slTranslateX=bsInFileStream.ReadSB(lpmMatrix->ubNTranslateBits);
	lpmMatrix->slTranslateY=bsInFileStream.ReadSB(lpmMatrix->ubNTranslateBits);

	bsInFileStream.SeekNextByte();
	ulMatrixLength=bsInFileStream.GetByteOffset()-ulMatrixLength;

	return ulMatrixLength;
}

UI32 CSWF::WriteMatrix(MATRIX *lpmMatrix)
{
	UI8 ubBits;
	UI32 ulMatrixLength=bsOutFileStream.GetByteOffset();
	
	/*Write scale info*/
	bsOutFileStream.WriteUB(lpmMatrix->fHasScale,1);
	if(lpmMatrix->fHasScale)
	{
		/*Calculate scale bits required*/
		ubBits=0;
		ubBits=MAX(ubBits,BITSFORSIGNEDVALUE(lpmMatrix->lfpScaleX));
		ubBits=MAX(ubBits,BITSFORSIGNEDVALUE(lpmMatrix->lfpScaleY));
		//lpmMatrix->ubNScaleBits=ubBits;
		
		bsOutFileStream.WriteUB(lpmMatrix->ubNScaleBits,5);
		bsOutFileStream.WriteFB(lpmMatrix->lfpScaleX,lpmMatrix->ubNScaleBits);
		bsOutFileStream.WriteFB(lpmMatrix->lfpScaleY,lpmMatrix->ubNScaleBits);
	}

	/*Write rotate info*/
	bsOutFileStream.WriteUB(lpmMatrix->fHasRotate,1);
	if(lpmMatrix->fHasRotate)
	{
		/*Calculate rotate bits required*/
		ubBits=0;
		ubBits=MAX(ubBits,BITSFORSIGNEDVALUE(lpmMatrix->lfpRotateSkew0));
		ubBits=MAX(ubBits,BITSFORSIGNEDVALUE(lpmMatrix->lfpRotateSkew1));
		lpmMatrix->ubNRotateBits=ubBits;

		bsOutFileStream.WriteUB(lpmMatrix->ubNRotateBits,5);
		bsOutFileStream.WriteFB(lpmMatrix->lfpRotateSkew0,lpmMatrix->ubNRotateBits);
		bsOutFileStream.WriteFB(lpmMatrix->lfpRotateSkew1,lpmMatrix->ubNRotateBits);
	}

	/*Write translate info*/
	ubBits=0;
	ubBits=MAX(ubBits,BITSFORSIGNEDVALUE(lpmMatrix->slTranslateX));
	ubBits=MAX(ubBits,BITSFORSIGNEDVALUE(lpmMatrix->slTranslateY));
	lpmMatrix->ubNTranslateBits=ubBits;

	bsOutFileStream.WriteSB(lpmMatrix->ubNTranslateBits,5);
	bsOutFileStream.WriteSB(lpmMatrix->slTranslateX,lpmMatrix->ubNTranslateBits);
	bsOutFileStream.WriteSB(lpmMatrix->slTranslateY,lpmMatrix->ubNTranslateBits);

	bsOutFileStream.SeekNextByte();
	ulMatrixLength=bsOutFileStream.GetByteOffset()-ulMatrixLength;

	return ulMatrixLength;
}

UI32 CSWF::ReadCXForm(CXFORM *lpcxfwaColorTransform, UI8 fAlpha)
{
	UI32 ulLength=bsInFileStream.GetByteOffset();

	memset(lpcxfwaColorTransform,0,sizeof(CXFORM));

	lpcxfwaColorTransform->fHasAddTerms=(UI8)bsInFileStream.ReadUB(1);
	lpcxfwaColorTransform->fHasMultTerms=(UI8)bsInFileStream.ReadUB(1);
	lpcxfwaColorTransform->ubNBits=(UI8)bsInFileStream.ReadUB(4);

	/*Read multiplier terms*/
	if(lpcxfwaColorTransform->fHasMultTerms)
	{
		lpcxfwaColorTransform->sfpRedMultTerm=bsInFileStream.ReadSB(lpcxfwaColorTransform->ubNBits);
		lpcxfwaColorTransform->sfpGreenMultTerm=bsInFileStream.ReadSB(lpcxfwaColorTransform->ubNBits);
		lpcxfwaColorTransform->sfpBlueMultTerm=bsInFileStream.ReadSB(lpcxfwaColorTransform->ubNBits);
		if(fAlpha) lpcxfwaColorTransform->sfpAlphaMultTerm=bsInFileStream.ReadSB(lpcxfwaColorTransform->ubNBits);
	}

	else
	{
		lpcxfwaColorTransform->sfpRedMultTerm=FLOATTOFP16(1.0);
		lpcxfwaColorTransform->sfpGreenMultTerm=FLOATTOFP16(1.0);
		lpcxfwaColorTransform->sfpBlueMultTerm=FLOATTOFP16(1.0);
		lpcxfwaColorTransform->sfpAlphaMultTerm=FLOATTOFP16(1.0);
	}
		
	/*Read add terms*/
	if(lpcxfwaColorTransform->fHasAddTerms)
	{
		lpcxfwaColorTransform->sbRedAddTerm=bsInFileStream.ReadSB(lpcxfwaColorTransform->ubNBits);
		lpcxfwaColorTransform->sbGreenAddTerm=bsInFileStream.ReadSB(lpcxfwaColorTransform->ubNBits);
		lpcxfwaColorTransform->sbBlueAddTerm=bsInFileStream.ReadSB(lpcxfwaColorTransform->ubNBits);
		if(fAlpha) lpcxfwaColorTransform->sbAlphaAddTerm=bsInFileStream.ReadSB(lpcxfwaColorTransform->ubNBits);
	}

	bsInFileStream.SeekNextByte();
	ulLength=bsInFileStream.GetByteOffset()-ulLength;
	
	return ulLength;
}

UI32 CSWF::WriteCXForm(CXFORM *lpcxfwaColorTransform, UI8 fAlpha)
{
	UI8 ubBits;
	UI32 ulLength=bsOutFileStream.GetByteOffset();

	/*Calculate required bits*/
	ubBits=0;
	ubBits=MAX(ubBits,BITSFORSIGNEDVALUE(lpcxfwaColorTransform->sfpRedMultTerm));
	ubBits=MAX(ubBits,BITSFORSIGNEDVALUE(lpcxfwaColorTransform->sfpGreenMultTerm));
	ubBits=MAX(ubBits,BITSFORSIGNEDVALUE(lpcxfwaColorTransform->sfpBlueMultTerm));
	ubBits=MAX(ubBits,BITSFORSIGNEDVALUE(lpcxfwaColorTransform->sfpAlphaMultTerm));
	ubBits=MAX(ubBits,BITSFORUNSIGNEDVALUE(lpcxfwaColorTransform->sbRedAddTerm));
	ubBits=MAX(ubBits,BITSFORUNSIGNEDVALUE(lpcxfwaColorTransform->sbGreenAddTerm));
	ubBits=MAX(ubBits,BITSFORUNSIGNEDVALUE(lpcxfwaColorTransform->sbBlueAddTerm));
	ubBits=MAX(ubBits,BITSFORUNSIGNEDVALUE(lpcxfwaColorTransform->sbAlphaAddTerm));
	lpcxfwaColorTransform->ubNBits=ubBits;

	/*Write flags and bits*/
	bsOutFileStream.WriteUB(lpcxfwaColorTransform->fHasAddTerms,1);
	bsOutFileStream.WriteUB(lpcxfwaColorTransform->fHasMultTerms,1);
	bsOutFileStream.WriteUB(lpcxfwaColorTransform->ubNBits,4);

	/*Write multiplier terms*/
	if(lpcxfwaColorTransform->fHasMultTerms)
	{
		bsOutFileStream.WriteSB(lpcxfwaColorTransform->sfpRedMultTerm,lpcxfwaColorTransform->ubNBits);
		bsOutFileStream.WriteSB(lpcxfwaColorTransform->sfpGreenMultTerm,lpcxfwaColorTransform->ubNBits);
		bsOutFileStream.WriteSB(lpcxfwaColorTransform->sfpBlueMultTerm,lpcxfwaColorTransform->ubNBits);
		if(fAlpha) bsOutFileStream.WriteSB(lpcxfwaColorTransform->sfpAlphaMultTerm,lpcxfwaColorTransform->ubNBits);
	}
	
	/*Write add terms*/
	if(lpcxfwaColorTransform->fHasAddTerms)
	{
		bsOutFileStream.WriteSB(lpcxfwaColorTransform->sbRedAddTerm,lpcxfwaColorTransform->ubNBits);
		bsOutFileStream.WriteSB(lpcxfwaColorTransform->sbGreenAddTerm,lpcxfwaColorTransform->ubNBits);
		bsOutFileStream.WriteSB(lpcxfwaColorTransform->sbBlueAddTerm,lpcxfwaColorTransform->ubNBits);
		if(fAlpha) bsOutFileStream.WriteSB(lpcxfwaColorTransform->sbAlphaAddTerm,lpcxfwaColorTransform->ubNBits);
	}

	bsOutFileStream.SeekNextByte();
	ulLength=bsOutFileStream.GetByteOffset()-ulLength;
	
	return ulLength;
}
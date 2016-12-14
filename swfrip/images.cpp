#include "swf.h"
#include <png.h>

UI32 CSWF::FixJPEGData(UI8 *prgbData, UI32 ulSize)
{
	for(UI32 iByte=0; iByte<ulSize; iByte++)
	{	
		if(prgbData[iByte]==0xFF)
		{
			if(prgbData[iByte+1]==0xD8 && iByte>1) prgbData[iByte+1]=0x00;
		
			if(prgbData[iByte+1]==0xD9 && iByte<ulSize-2) prgbData[iByte+1]=0x00;
		}
	}

	return 1;
}

UI32 CSWF::ReadJPEGTables(JPEGTABLES *lpjtJpegTables)
{
	UI32 ulTagLength=bsInFileStream.GetByteOffset();

	memset(lpjtJpegTables,0,sizeof(JPEGTABLES));
	memcpy(&lpjtJpegTables->rhTagHeader,&rhTagHeader,sizeof(RECORDHEADER));

	lpjtJpegTables->ulJPEGDataSize=lpjtJpegTables->rhTagHeader.ulLength;
	lpjtJpegTables->prgbJPEGData=new UI8[lpjtJpegTables->ulJPEGDataSize];
	bsInFileStream.ReadBytes(lpjtJpegTables->prgbJPEGData,lpjtJpegTables->ulJPEGDataSize);

	ulTagLength=bsInFileStream.GetByteOffset()-ulTagLength;
	return ulTagLength;
}

UI32 CSWF::WriteJPEGTables(JPEGTABLES *lpjtJpegTables, FILE *pfileSWF)
{
	if(!lpjtJpegTables) return 0;
	
	bsOutFileStream.Create(sizeof(JPEGTABLES)+lpjtJpegTables->ulJPEGDataSize);
	bsOutFileStream.WriteBytes(lpjtJpegTables->prgbJPEGData,lpjtJpegTables->ulJPEGDataSize);

	lpjtJpegTables->rhTagHeader.usTagCode=TAGCODE_JPEGTABLES;
	lpjtJpegTables->rhTagHeader.ulLength=lpjtJpegTables->ulJPEGDataSize;
	WriteTagHeader(&lpjtJpegTables->rhTagHeader,pfileSWF);
	bsOutFileStream.WriteToFile(pfileSWF);

	return lpjtJpegTables->rhTagHeader.ulLength;
}

UI32 CSWF::ReadDefineBits(DEFINEBITS *lpdbDefineBits)
{
	UI32 ulTagLength=bsInFileStream.GetByteOffset();

	memset(lpdbDefineBits,0,sizeof(DEFINEBITS));
	memcpy(&lpdbDefineBits->rhTagHeader,&rhTagHeader,sizeof(RECORDHEADER));
	lpdbDefineBits->usCharacterID=bsInFileStream.ReadUI16();
	lpdbDefineBits->ulJPEGDataSize=lpdbDefineBits->rhTagHeader.ulLength-sizeof(UI16);
	lpdbDefineBits->prgbJPEGData=new UI8[lpdbDefineBits->ulJPEGDataSize];
	bsInFileStream.ReadBytes(lpdbDefineBits->prgbJPEGData,lpdbDefineBits->ulJPEGDataSize);
	
	ulTagLength=bsInFileStream.GetByteOffset()-ulTagLength;
	return ulTagLength;
}

UI32 CSWF::WriteDefineBits(DEFINEBITS *lpdbDefineBits, FILE *pfileSWF)
{
	bsOutFileStream.Create(sizeof(DEFINEBITS)+lpdbDefineBits->ulJPEGDataSize);

	bsOutFileStream.WriteUI16(lpdbDefineBits->usCharacterID);
	bsOutFileStream.WriteBytes(lpdbDefineBits->prgbJPEGData,lpdbDefineBits->ulJPEGDataSize);

	lpdbDefineBits->rhTagHeader.usTagCode=TAGCODE_DEFINEBITS;
	lpdbDefineBits->rhTagHeader.ulLength=bsOutFileStream.GetByteOffset();
	WriteTagHeader(&lpdbDefineBits->rhTagHeader,pfileSWF);
	bsOutFileStream.WriteToFile(pfileSWF);
	
	return lpdbDefineBits->rhTagHeader.ulLength;
}

UI32 CSWF::SaveDefineBitsAsJPEG(DEFINEBITS *lpdbDefineBits)
{
	FILE *pfileJPEG;
	char szJPEGFileName[MAX_FILEPATH];
	
	/*Create file*/
	CreateFileName(szJPEGFileName,"image",lpdbDefineBits->usCharacterID,"jpg");
	pfileJPEG=fopen(szJPEGFileName,"wb");
	fwrite(lpjtJPEGTables->prgbJPEGData,1,lpjtJPEGTables->ulJPEGDataSize-2,pfileJPEG);
	fwrite(lpdbDefineBits->prgbJPEGData+2,1,lpdbDefineBits->ulJPEGDataSize-2,pfileJPEG);
	fclose(pfileJPEG);
	return 1;
}

UI32 CSWF::ReadDefineBitsJPEG2(DEFINEBITSJPEG2 *lpdbj2DefineBits)
{
	UI32 ulTagLength=bsInFileStream.GetByteOffset();

	memset(lpdbj2DefineBits,0,sizeof(DEFINEBITSJPEG2));
	memcpy(&lpdbj2DefineBits->rhTagHeader,&rhTagHeader,sizeof(RECORDHEADER));
	
	lpdbj2DefineBits->usCharacterID=bsInFileStream.ReadUI16();
	lpdbj2DefineBits->ulJPEGDataSize=lpdbj2DefineBits->rhTagHeader.ulLength-sizeof(UI16);
	lpdbj2DefineBits->prgbJPEGData=new UI8[lpdbj2DefineBits->ulJPEGDataSize];
	bsInFileStream.ReadBytes(lpdbj2DefineBits->prgbJPEGData,lpdbj2DefineBits->ulJPEGDataSize);
	
	ulTagLength=bsInFileStream.GetByteOffset()-ulTagLength;
	return ulTagLength;
}

UI32 CSWF::WriteDefineBitsJPEG2(DEFINEBITSJPEG2 *lpdbj2DefineBits, FILE *pfileSWF)
{
	bsOutFileStream.Create(sizeof(DEFINEBITSJPEG2)+lpdbj2DefineBits->ulJPEGDataSize);

	bsOutFileStream.WriteUI16(lpdbj2DefineBits->usCharacterID);
	bsOutFileStream.WriteBytes(lpdbj2DefineBits->prgbJPEGData,lpdbj2DefineBits->ulJPEGDataSize);

	lpdbj2DefineBits->rhTagHeader.usTagCode=TAGCODE_DEFINEBITSJPEG2;
	lpdbj2DefineBits->rhTagHeader.ulLength=bsOutFileStream.GetByteOffset();
	WriteTagHeader(&lpdbj2DefineBits->rhTagHeader,pfileSWF);
	bsOutFileStream.WriteToFile(pfileSWF);
	
	return lpdbj2DefineBits->rhTagHeader.ulLength;
}

UI32 CSWF::SaveDefineBitsJPEG2AsJPEG(DEFINEBITSJPEG2 *lpdbj2DefineBits)
{
	FILE *pfileJPEG;
	UI8 *prgbJPEGData, *prgbStart;
	UI32 ulJPEGDataSize;
	char szJPEGFileName[MAX_FILEPATH];
	
	prgbStart=lpdbj2DefineBits->prgbJPEGData;
	ulJPEGDataSize=lpdbj2DefineBits->ulJPEGDataSize;

	while(prgbStart[3]==0xD8 || prgbStart[3]==0xD9) 
	{
		prgbStart+=2;
		ulJPEGDataSize-=2;
	}

	prgbJPEGData=new UI8[ulJPEGDataSize];
	memcpy(prgbJPEGData,prgbStart,ulJPEGDataSize);
	FixJPEGData(prgbJPEGData,ulJPEGDataSize);

	/*write file*/
	CreateFileName(szJPEGFileName,"image",lpdbj2DefineBits->usCharacterID,"jpg");
	pfileJPEG=fopen(szJPEGFileName,"wb");
	fwrite(prgbJPEGData,1,ulJPEGDataSize,pfileJPEG);
	fclose(pfileJPEG);

	return 1;
}

UI32 CSWF::ReadDefineBitsJPEG3(DEFINEBITSJPEG3 *lpdbj3DefineBits)
{
	UI32 ulTagLength=bsInFileStream.GetByteOffset();

	memset(lpdbj3DefineBits,0,sizeof(DEFINEBITSJPEG2));
	memcpy(&lpdbj3DefineBits->rhTagHeader,&rhTagHeader,sizeof(RECORDHEADER));

	lpdbj3DefineBits->usCharacterID=bsInFileStream.ReadUI16();
	lpdbj3DefineBits->ulAlphaDataOffset=bsInFileStream.ReadUI32();
	lpdbj3DefineBits->prgbJPEGData=new UI8[lpdbj3DefineBits->ulAlphaDataOffset];
	bsInFileStream.ReadBytes(lpdbj3DefineBits->prgbJPEGData,lpdbj3DefineBits->ulAlphaDataOffset);
	
	/*uncompress alpha data*/
	lpdbj3DefineBits->ulAlphaDataSize=lpdbj3DefineBits->rhTagHeader.ulLength-lpdbj3DefineBits->ulAlphaDataOffset-sizeof(UI16)-sizeof(UI32);
	lpdbj3DefineBits->prgbBitmapAlphaData=new UI8[lpdbj3DefineBits->ulAlphaDataSize];
	bsInFileStream.ReadBytes(lpdbj3DefineBits->prgbBitmapAlphaData,lpdbj3DefineBits->ulAlphaDataSize);

	ulTagLength=bsInFileStream.GetByteOffset()-ulTagLength;
	return ulTagLength;
}

UI32 CSWF::WriteDefineBitsJPEG3(DEFINEBITSJPEG3 *lpdbj3DefineBits, FILE *pfileSWF)
{
	bsOutFileStream.Create(sizeof(DEFINEBITSJPEG3)+lpdbj3DefineBits->ulAlphaDataOffset+lpdbj3DefineBits->ulAlphaDataSize);

	bsOutFileStream.WriteUI16(lpdbj3DefineBits->usCharacterID);
	bsOutFileStream.WriteUI32(lpdbj3DefineBits->ulAlphaDataOffset);
	bsOutFileStream.WriteBytes(lpdbj3DefineBits->prgbJPEGData,lpdbj3DefineBits->ulAlphaDataOffset);
	bsOutFileStream.WriteBytes(lpdbj3DefineBits->prgbBitmapAlphaData,lpdbj3DefineBits->ulAlphaDataSize);

	lpdbj3DefineBits->rhTagHeader.usTagCode=TAGCODE_DEFINEBITSJPEG3;
	lpdbj3DefineBits->rhTagHeader.ulLength=bsOutFileStream.GetByteOffset();
	WriteTagHeader(&lpdbj3DefineBits->rhTagHeader,pfileSWF);
	bsOutFileStream.WriteToFile(pfileSWF);
	
	return lpdbj3DefineBits->rhTagHeader.ulLength;
}

UI32 CSWF::SaveDefineBitsJPEG3AsJPEG(DEFINEBITSJPEG3 *lpdbj3DefineBits)
{
	FILE *pfileJPEG;
	UI8 *prgbJPEGData, *prgbStart;
	UI32 ulJPEGDataSize;
	char szJPEGFileName[MAX_FILEPATH];

	/*Create file*/
	prgbStart=lpdbj3DefineBits->prgbJPEGData;
	ulJPEGDataSize=lpdbj3DefineBits->ulAlphaDataOffset;

	while(prgbStart[3]==0xD8 || prgbStart[3]==0xD9) 
	{
		prgbStart+=2;
		ulJPEGDataSize-=2;
	}

	prgbJPEGData=new UI8[ulJPEGDataSize];
	memcpy(prgbJPEGData,prgbStart,ulJPEGDataSize);
	FixJPEGData(prgbJPEGData,ulJPEGDataSize);

	CreateFileName(szJPEGFileName,"image",lpdbj3DefineBits->usCharacterID,"jpg");
	pfileJPEG=fopen(szJPEGFileName,"wb");
	fwrite(prgbJPEGData,1,ulJPEGDataSize,pfileJPEG);
	fclose(pfileJPEG);

	return 1;
}

UI32 CSWF::ReadDefineBitsLossless(DEFINEBITSLOSSLESS *lpdblDefineBits)
{
	UI32 ulTagLength=bsInFileStream.GetByteOffset();
	UI32 ulPixSize, ulScanlineSize;
	SI32 siZError;

	memset(lpdblDefineBits,0,sizeof(DEFINEBITSLOSSLESS));
	memcpy(&lpdblDefineBits->rhTagHeader,&rhTagHeader,sizeof(RECORDHEADER));
	
	lpdblDefineBits->usCharacterID=bsInFileStream.ReadUI16();
	lpdblDefineBits->ubBitmapFormat=bsInFileStream.ReadUI8();
	lpdblDefineBits->usBitmapWidth=bsInFileStream.ReadUI16();
	lpdblDefineBits->usBitmapHeight=bsInFileStream.ReadUI16();
	if(lpdblDefineBits->ubBitmapFormat==BITMAP_COLORMAP) 
		lpdblDefineBits->ubBitmapColorTableSize=bsInFileStream.ReadUI8();

	/*Calculate bitmap data size*/
	if(lpdblDefineBits->rhTagHeader.usTagCode==TAGCODE_DEFINEBITSLOSSLESS)
	{
		if(lpdblDefineBits->ubBitmapFormat==BITMAP_RGB15) ulPixSize=2;
		else ulPixSize=3;
	}

	else ulPixSize=4;

	ulScanlineSize=lpdblDefineBits->usBitmapWidth;
	if(lpdblDefineBits->ubBitmapFormat!=BITMAP_COLORMAP) ulScanlineSize*=ulPixSize;
	ulScanlineSize+=(ulScanlineSize%4? 4-ulScanlineSize%4:0);
	lpdblDefineBits->ulDataSize=ulScanlineSize*lpdblDefineBits->usBitmapHeight;

	if(lpdblDefineBits->ubBitmapFormat==BITMAP_COLORMAP) 
		lpdblDefineBits->ulDataSize+=(lpdblDefineBits->ubBitmapColorTableSize+1)*ulPixSize;

	/*Read and Uncompress bitmap data*/
	lpdblDefineBits->ulZlibDataSize=lpdblDefineBits->rhTagHeader.ulLength-(bsInFileStream.GetByteOffset()-ulTagLength);
	lpdblDefineBits->lprgbZlibBitmapData=new UI8[lpdblDefineBits->ulZlibDataSize];
	bsInFileStream.ReadBytes(lpdblDefineBits->lprgbZlibBitmapData,lpdblDefineBits->ulZlibDataSize);
	
	lpdblDefineBits->lprgbBitmapData=new UI8[lpdblDefineBits->ulDataSize];
	siZError=uncompress(lpdblDefineBits->lprgbBitmapData,&lpdblDefineBits->ulDataSize,
						lpdblDefineBits->lprgbZlibBitmapData,lpdblDefineBits->ulZlibDataSize);
	
	if(siZError)
	{
		ulPixSize=4;
		ulScanlineSize=lpdblDefineBits->usBitmapWidth;
		ulScanlineSize*=ulPixSize;
		ulScanlineSize+=(ulScanlineSize%4? 4-ulScanlineSize%4:0);
		lpdblDefineBits->ulDataSize=ulScanlineSize*lpdblDefineBits->usBitmapHeight;

		delete lpdblDefineBits->lprgbBitmapData;
		lpdblDefineBits->lprgbBitmapData=new UI8[lpdblDefineBits->ulDataSize];

		siZError=uncompress(lpdblDefineBits->lprgbBitmapData,&lpdblDefineBits->ulDataSize,
							lpdblDefineBits->lprgbZlibBitmapData,lpdblDefineBits->ulZlibDataSize);

		lpdblDefineBits->rhTagHeader.usTagCode=TAGCODE_DEFINEBITSLOSSLESS2;
	}
		
	
	delete lpdblDefineBits->lprgbZlibBitmapData;
	lpdblDefineBits->lprgbZlibBitmapData=NULL;

	ulTagLength=bsInFileStream.GetByteOffset()-ulTagLength;
	return ulTagLength;
}

UI32 CSWF::WriteDefineBitsLossless(DEFINEBITSLOSSLESS *lpdblDefineBits, FILE *pfileSWF)
{
	bsOutFileStream.Create(sizeof(DEFINEBITSLOSSLESS)+1.2*lpdblDefineBits->ulDataSize);

	bsOutFileStream.WriteUI16(lpdblDefineBits->usCharacterID);
	bsOutFileStream.WriteUI8(lpdblDefineBits->ubBitmapFormat);
	bsOutFileStream.WriteUI16(lpdblDefineBits->usBitmapWidth);
	bsOutFileStream.WriteUI16(lpdblDefineBits->usBitmapHeight);
	
	if(lpdblDefineBits->ubBitmapFormat==BITMAP_COLORMAP) 
		bsOutFileStream.WriteUI8(lpdblDefineBits->ubBitmapColorTableSize);
	
	lpdblDefineBits->ulZlibDataSize=1.2*lpdblDefineBits->ulDataSize;
	lpdblDefineBits->lprgbZlibBitmapData=new UI8[lpdblDefineBits->ulZlibDataSize];
	
	compress(lpdblDefineBits->lprgbZlibBitmapData,&lpdblDefineBits->ulZlibDataSize,lpdblDefineBits->lprgbBitmapData,lpdblDefineBits->ulDataSize);
	bsOutFileStream.WriteBytes(lpdblDefineBits->lprgbZlibBitmapData,lpdblDefineBits->ulZlibDataSize);
	
	delete lpdblDefineBits->lprgbZlibBitmapData;
	lpdblDefineBits->lprgbZlibBitmapData=NULL;

	lpdblDefineBits->rhTagHeader.ulLength=bsOutFileStream.GetByteOffset();
	WriteTagHeader(&lpdblDefineBits->rhTagHeader,pfileSWF);
	bsOutFileStream.WriteToFile(pfileSWF);
	
	return lpdblDefineBits->rhTagHeader.ulLength;
}

UI32 CSWF::SaveDefineBitsLosslessAsFile(DEFINEBITSLOSSLESS *lpdblDefineBits, char *szExt)
{
	char szBitmapFileName[MAX_FILEPATH];

	/*Create file*/
	CreateFileName(szBitmapFileName,"image",lpdblDefineBits->usCharacterID,szExt);
	
	UI8 ubColorDepth;
	
	if(lpdblDefineBits->ubBitmapFormat==BITMAP_COLORMAP)
		ubColorDepth=8;
	else if(lpdblDefineBits->rhTagHeader.usTagCode==TAGCODE_DEFINEBITSLOSSLESS2)
		ubColorDepth=32;
	else ubColorDepth=24;

	Bitmap bmBitmap(lpdblDefineBits->usBitmapWidth,lpdblDefineBits->usBitmapHeight,ubColorDepth);
	
	CBitstream bsBitmapStream(lpdblDefineBits->ulDataSize,lpdblDefineBits->lprgbBitmapData);

	RGBA rgbaColor;
	COLORREF crColor;

	/*Read colormap*/
	if(lpdblDefineBits->ubBitmapFormat==BITMAP_COLORMAP)
		for(UI16 iPaletteEntry=0; iPaletteEntry<lpdblDefineBits->ubBitmapColorTableSize+1; iPaletteEntry++)
		{
			rgbaColor.ubRed=bsBitmapStream.ReadUI8();
			rgbaColor.ubGreen=bsBitmapStream.ReadUI8();
			rgbaColor.ubBlue=bsBitmapStream.ReadUI8();

			if(lpdblDefineBits->rhTagHeader.usTagCode==TAGCODE_DEFINEBITSLOSSLESS2) 
				rgbaColor.ubAlpha=bsBitmapStream.ReadUI8();
				
			
			crColor=RGBALPHA(rgbaColor.ubRed,rgbaColor.ubGreen,rgbaColor.ubBlue,rgbaColor.ubAlpha);

			bmBitmap.SetPaletteEntry(iPaletteEntry,crColor);
		}
	
	/*PixelData*/
	for(UI16 usY=0; usY<lpdblDefineBits->usBitmapHeight; usY++)
	{
		UI32 ulBytesRead=bsBitmapStream.GetByteOffset();

		for(UI16 usX=0; usX<lpdblDefineBits->usBitmapWidth; usX++)
		{
			if(lpdblDefineBits->ubBitmapFormat==BITMAP_COLORMAP)
				bmBitmap.SetPixelIndex(usX,usY,bsBitmapStream.ReadUI8());
			
			if(lpdblDefineBits->ubBitmapFormat==BITMAP_RGB15)
			{
				bsBitmapStream.SeekBits(1);
				rgbaColor.ubRed=(UI8)bsBitmapStream.ReadUB(5);
				rgbaColor.ubGreen=(UI8)bsBitmapStream.ReadUB(5);
				rgbaColor.ubBlue=(UI8)bsBitmapStream.ReadUB(5);

				crColor=RGB(rgbaColor.ubRed,rgbaColor.ubGreen,rgbaColor.ubBlue);
				bmBitmap.SetPixelColor(usX,usY,crColor);
			}

			if(lpdblDefineBits->ubBitmapFormat==BITMAP_RGB24)
			{
				if(lpdblDefineBits->rhTagHeader.usTagCode==TAGCODE_DEFINEBITSLOSSLESS2) 
					rgbaColor.ubAlpha=bsBitmapStream.ReadUI8();

				rgbaColor.ubRed=bsBitmapStream.ReadUI8();
				rgbaColor.ubGreen=bsBitmapStream.ReadUI8();
				rgbaColor.ubBlue=bsBitmapStream.ReadUI8();
				
				crColor=RGBALPHA(rgbaColor.ubRed,rgbaColor.ubGreen,rgbaColor.ubBlue,rgbaColor.ubAlpha);
				bmBitmap.SetPixelColor(usX,usY,crColor);
			}
		}

		ulBytesRead=bsBitmapStream.GetByteOffset()-ulBytesRead;
		bsBitmapStream.SeekBytes((ulBytesRead%4? 4-ulBytesRead%4:0));
	}
	
	if(!stricmp(szExt,"bmp")) bmBitmap.Save(szBitmapFileName);
	if(!stricmp(szExt,"pcx")) bmBitmap.SavePCX(szBitmapFileName);
	if(!stricmp(szExt,"png")) bmBitmap.SavePNG(szBitmapFileName);

	return 1;
}
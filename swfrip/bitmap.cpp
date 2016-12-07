/********************************************************************
		
				Windows Bitmap File Class [Bitmap.cpp]
	
				Wesley Hopper (HopperW2000@Yahoo.com)

********************************************************************/
 
#include "Bitmap.h"
#include "PCX.h"
 

/********************************************************************
*																	*
*	Bitmap::Bitmap(int W, int H, int CDepth,						*
*				   BYTE *Data=NULL,	int Format=BM_NORMAL)			*
*																	*
*	Constructor to create a new bitmap of given values:				* 
*	Width, Height, Color Depth; Optional: Pointer to image data		*
*	to copy, format to load that data into the Bitmap Buffer.		*	
*																	*
********************************************************************/

Bitmap::Bitmap(int W, int H, int CDepth=24, BYTE *Data, int Format)
{
	Width=W; Height=H; Bpp=CDepth;
	
	//Calculate new data size
	DataSize=RoundToInt(Width*((float)Bpp/8));
	if(DataSize%4)	DataSize+=4-DataSize%4;
	DataSize*=Height;

	//Set Header Info
	SetBIHeader();
	InitInfo(BM_INFOHEADER);
	BMPInfo=(BITMAPINFO*)malloc(sizeof(BITMAPINFOHEADER)+(Colors<<2)); 
	BMPInfo->bmiHeader=BMPInfoHeader;

	//Allocate Buffer
	Buffer=(BYTE*)malloc(DataSize);
	memset(Buffer,255,DataSize);
	memset(Palette,255,sizeof(Palette));
	if(Data) 
	{
		if(Format==BM_FLIP)
			for(int Line=0; Line<Height; Line++)
				memcpy(Buffer+(Height-1-Line)*ScanLineSize, Data+Line*ScanLineSize, ScanLineSize);
	
		else memcpy(Buffer,Data,DataSize);
	}
}


/********************************************************************
*																	*
*	int Bitmap::Load(const char *Filename, int BufferFormat)		*
*																	*
*	Loads a bitmap from file, storing its pixel data in Buffer		*
*	in the specified format:										*
*		NORMAL - Keeps the data in bottom up format	(Standard)		*
*		FLIP - Flip the data to right side up (e.g for Direct Draw)	*
*																	*
*	Returns:														*
*		NULL on success, Bitmap Error Code on failure.				*
*																	*
********************************************************************/

int Bitmap::Load(const char *FileName, int BufferFormat=BM_NORMAL)				
{
	//Open file
	FILE *bmp;
	if(!(bmp=fopen(FileName, "rb"))) return BM_OPENERR;
	int HeaderType=BM_INFOHEADER;

	//Read Windows bitmap headers
	fread(&BMPHeader,1,sizeof(BITMAPFILEHEADER),bmp); 
	if(BMPHeader.bfType!=0x4D42)
	{
		fclose(bmp);
		return BM_NOTBITMAP;}
	
	fread(&BMPInfoHeader,1,sizeof(BITMAPINFOHEADER),bmp);
	if(BMPInfoHeader.biPlanes!=1) //Go back & get BITMAPCOREHEADER
	{	
		fseek(bmp,14,SEEK_SET);
		fread(&BMPCoreHeader,1,sizeof(BITMAPCOREHEADER),bmp);
		HeaderType=BM_COREHEADER;}

		
	InitInfo(HeaderType);
	
	//Allocate buffer for BITMAPINFO*
	if(BMPInfo) free(BMPInfo);
	BMPInfo=(BITMAPINFO*)malloc(sizeof(BITMAPINFOHEADER)+(Colors<<2)); 
	BMPInfo->bmiHeader=BMPInfoHeader;
	
	//Load palette
	if(BMPInfoHeader.biBitCount<16)
	{
		fread(&Palette,4,Colors,bmp);
		memcpy(BMPInfo->bmiColors, Palette, Colors<<2);}
	
	//Allocate data buffer
	if(Buffer) free(Buffer);
	if(!(Buffer=(BYTE*)malloc(BMPInfoHeader.biSizeImage)))
    {
		fclose(bmp);
		return BM_ALLOCERR;}

	//Just in case OffBits is wrong
	long CurOff=ftell(bmp);
	fseek(bmp,0,SEEK_END);
	if(Bpp<16 && (ftell(bmp)-BMPHeader.bfOffBits)!=DataSize)
	{
		fseek(bmp,-1*DataSize,SEEK_END);
		BMPHeader.bfOffBits=ftell(bmp);}
	
	else if(Bpp>8)
	{
		fseek(bmp,CurOff,SEEK_SET);
		if(BMPHeader.bfOffBits!=ftell(bmp))
			BMPHeader.bfOffBits=ftell(bmp);}
	
	else fseek(bmp,BMPHeader.bfOffBits,SEEK_SET);

	//Correction for differnece in Header sizes
	if(HeaderType==BM_COREHEADER) BMPHeader.bfOffBits+=28;

	//Read pixel data
	if(BufferFormat==BM_NORMAL) 
		if(!fread(Buffer, 1, BMPInfoHeader.biSizeImage, bmp))
		{
			fclose(bmp);
			return BM_READERR;}
	
	if(BufferFormat==BM_FLIP) 
	for(int line=BMPInfoHeader.biHeight-1; line>=0; line--)
		fread(Buffer+line*(BMPInfoHeader.biSizeImage/BMPInfoHeader.biHeight), 1, BMPInfoHeader.biSizeImage/BMPInfoHeader.biHeight, bmp);
	
	fclose(bmp);
	return 0;
}


/********************************************************************
*																	*
*	int Bitmap::LoadPCX(const char *Filename)						*
*																	*
*	Loads a PC Paintbrush PCX from file into the Bitmap.			*
*																	*
*	Returns: 1 on success, 0 on failure.							*	
*																	*
********************************************************************/

int Bitmap::LoadPCX(const char* FileName)
{
	PCX *Pcx=new PCX;
	if(!Pcx->Load(FileName)) return 0;

	Bitmap *PcxBmp=new Bitmap(Pcx->Width,Pcx->Height,
							  Pcx->Bpp*Pcx->Planes,
							  Pcx->Buffer,BM_FLIP);

	PcxBmp->SetPalette(Pcx->Palette);
	*this=*PcxBmp;
	delete Pcx;
	delete PcxBmp;
	return 1;
}


/********************************************************************
*																	*
*	int Bitmap::Save(const char *FileName)							*									*
*																	*
*	Saves the Bitmap to file:										*
*		Bitmap File Header											*
*		Bitmap Info Header											*
*		Palette (if less than 16-Bit)								*
*		Pixel Data (from Buffer)									*
*																	*
*	Returns:														*
*		0 (NULL) on success, Bitmap Error Code on failure.			*
*																	*
********************************************************************/		

int Bitmap::Save(const char* FileName)
{
	FILE *BMPFile;
	if(!(BMPFile=fopen(FileName, "wb"))) return BM_CREATEERR;

	fwrite(&BMPHeader, 1, sizeof(BITMAPFILEHEADER), BMPFile);
	fwrite(&BMPInfoHeader, 1, sizeof(BITMAPINFOHEADER), BMPFile);
	
	if(Bpp<16)
		if(!fwrite(Palette, 1, BMPHeader.bfOffBits-54, BMPFile)) 
			return BM_WRITEERR;
		
	if(!fwrite(Buffer, 1, DataSize, BMPFile)) return BM_WRITEERR;
	
	fclose(BMPFile);
	return 0;
}


/********************************************************************
*																	*
*	int Bitmap::SavePCX(const char *Filename)						*
*																	*
*	Saves the Bitmap as a PC Paintbrush PCX file.					*
*																	*
*	Returns: 1 on success, 0 on failure.							*
*																	*
********************************************************************/

int Bitmap::SavePCX(const char* FileName)
{
	PCX *Pcx=new PCX(Width,Height,Bpp,Buffer);
	Pcx->SetPalette(Palette);
	if(!Pcx->Save(FileName)) return 0;
	delete Pcx;
	return 1;
}

/*use libpng*/

int Bitmap::SavePNG(const char *FileName)
{
	FILE *pfilePNG=NULL;
	png_structp png_ptr;
	png_infop info_ptr;
	png_colorp palette;
	png_bytep *row_pointers;
	int ColorType, ColorDepth;

	/*write struct*/
	png_ptr=png_create_write_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
	if(!png_ptr) return 0;

	/*info struct*/
	info_ptr=png_create_info_struct(png_ptr);
	if(!info_ptr)
	{
		png_destroy_write_struct(&png_ptr,(png_infopp)NULL);
		return 0;
	}

	/*error jump*/
	if(setjmp(png_jmpbuf(png_ptr)))
	{
		if(pfilePNG) fclose(pfilePNG);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return 0;
	}

	/*pass it the FILE*/
	pfilePNG=fopen(FileName,"wb");
	png_init_io(png_ptr,pfilePNG);

	/*setup info*/
	if(Bpp==32) ColorType=PNG_COLOR_TYPE_RGB_ALPHA;
	if(Bpp==24) ColorType=PNG_COLOR_TYPE_RGB;
	else		ColorType=PNG_COLOR_TYPE_PALETTE;

	ColorDepth=8;

	png_set_IHDR(png_ptr,info_ptr,Width,Height,ColorDepth,ColorType,
				 PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_BASE,PNG_FILTER_TYPE_BASE);

	/*palette*/
	palette=(png_colorp)png_malloc(png_ptr,PNG_MAX_PALETTE_LENGTH*sizeof(png_color));

	if(Bpp<16)
	{
		for(int i=0; i<PNG_MAX_PALETTE_LENGTH; ++i) 
		{
			palette[i].red=Palette[i].rgbRed;
			palette[i].green=Palette[i].rgbGreen;
			palette[i].blue=Palette[i].rgbBlue;
			//palette[i].alpha=Palette[i].rgbReserved;
		}
	
		png_set_PLTE(png_ptr,info_ptr,palette,PNG_MAX_PALETTE_LENGTH);
	}

	/*write info*/
	png_write_info(png_ptr,info_ptr);

	/*pixel data*/
	row_pointers=new png_byte*[Height];

	for(int y=0; y<Height; y++)
	{
		row_pointers[y]=Buffer+Offset(0,y);
	}
	
	/*write data*/
	png_write_image(png_ptr,row_pointers);
	png_write_end(png_ptr,NULL);
	png_destroy_write_struct(&png_ptr,(png_infopp)NULL);
	fclose(pfilePNG);

	delete row_pointers;
	delete palette;

	return 1;
}


/********************************************************************
*																	*
*	void Bitmap::operator = (const Bitmap &CopyBitmap)				*
*																	*
*	Sets a Bitmap object with the data of another.					*
*																	*
********************************************************************/

void Bitmap::operator = (const Bitmap &CopyBitmap)
{
	//Copy Header Info and Palette
	memcpy(&BMPHeader, &CopyBitmap.BMPHeader, sizeof(BITMAPFILEHEADER));
	memcpy(&BMPInfoHeader, &CopyBitmap.BMPInfoHeader, sizeof(BITMAPINFOHEADER));
	InitInfo(BM_INFOHEADER);
	if(Bpp<16) memcpy(Palette, CopyBitmap.Palette, sizeof(Palette));
	
	//Allocate new BITMAPINFO and copy
	if(BMPInfo) free(BMPInfo);
	BMPInfo=(BITMAPINFO*)malloc(sizeof(BITMAPINFOHEADER)+(Colors<<2)); 
	memcpy(BMPInfo, CopyBitmap.BMPInfo, sizeof(BITMAPINFOHEADER)+(Colors<<2));
		
	//Allocate and copy Buffer
	if(Buffer) free(Buffer);
	Buffer=(BYTE*)malloc(CopyBitmap.DataSize);
	memcpy(Buffer, CopyBitmap.Buffer, CopyBitmap.DataSize);
}


/********************************************************************
*																	*
*	void Bitmap::SetBIHeader(void)									*
*																	*
*	Sets the data in the BITMAPINFOHEADER structure.				*
*																	*
********************************************************************/

void Bitmap::SetBIHeader()
{
	memset(&BMPHeader,0,sizeof(BITMAPFILEHEADER));
	BMPHeader.bfType=0x4D42;
    BMPHeader.bfOffBits=54;
	if(Bpp<16)	BMPHeader.bfOffBits+=int(pow(2.0,Bpp))<<2;
	BMPHeader.bfSize=DataSize+BMPHeader.bfOffBits;
	memset(&BMPInfoHeader,0,sizeof(BITMAPINFOHEADER));
	BMPInfoHeader.biSize=40;
	BMPInfoHeader.biWidth=Width;
	BMPInfoHeader.biHeight=Height;
	BMPInfoHeader.biPlanes=1;
	BMPInfoHeader.biBitCount=Bpp;
	BMPInfoHeader.biSizeImage=DataSize;
}


/********************************************************************
*																	*
*	void Bitmap::InitInfo(int HeaderType)							*
*																	*
*	Initializes the Bitmap info data. 0=BITMAPINFO,1=BITMAPCORE.	*
*																	*
********************************************************************/

void Bitmap::InitInfo(int HeaderType)
{
	if(HeaderType==BM_COREHEADER)
	{
		DataSize=BMPCoreHeader.bcSize;
		Width=BMPCoreHeader.bcWidth;
		Height=BMPCoreHeader.bcHeight;
		Bpp=BMPCoreHeader.bcBitCount;
		SetBIHeader();}

	if(HeaderType==BM_INFOHEADER)
	{
		if(!BMPInfoHeader.biSizeImage) 
		{
			BMPInfoHeader.biSizeImage=BMPHeader.bfSize-54;
			if(BMPInfoHeader.biBitCount<16)
				BMPInfoHeader.biSizeImage-=int(pow(2.0,Bpp))<<2;}
	
		DataSize=BMPInfoHeader.biSizeImage;
		Width=BMPInfoHeader.biWidth;
		Height=BMPInfoHeader.biHeight;
		Bpp=BMPInfoHeader.biBitCount;}

	//Check for correct data size
	int DSBeku=RoundToInt(Width*((float)Bpp/8));
	if(DSBeku%4)	DSBeku+=4-DSBeku%4;
	DSBeku*=Height;
	if(DataSize!=DSBeku)
	{
		BMPInfoHeader.biSizeImage=DSBeku;
		BMPHeader.bfSize=DSBeku+54;
		if(Bpp==8) BMPHeader.bfSize+=1024;
		DataSize=DSBeku;}

	ScanLineSize=DataSize/Height;

	if(Bpp==1)	Colors=2;
	if(Bpp==4)	Colors=16;
	if(Bpp==8)	Colors=256;
	if(Bpp==16 || Bpp==24 || Bpp==32) Colors=0; 
}


/********************************************************************
*																	*
*	COLORREF Bitmap::GetPixelFiltered(float X, float Y)				*
*																	*
*	Creates a color for the given X & Y floats by means of			*
*	bilinear filtering. X & Y must be calculated such as to			*
*	reflect	(WholeX.XPan,WholeY.YPan), where (WholeX,WholeY)		*
*	is the coord for the top left pixel in the filtering,			*
*	and XPan & YPan are values to control the level of				*
*	color mergeance for AverageColors().							*
*																	*
*	Returns:														*
*		The new pixel color, -1 (0xFFFFFFFF) on failure.			*
*																	*
********************************************************************/

COLORREF Bitmap::GetPixelFiltered(double X, double Y)
{
	if(!CheckXY(X,Y) || Bpp<16) return 0xFFFFFFFF;

	float XPan=X-(int)X, YPan=Y-(int)Y;
	COLORREF CurColor, NextColor, C1, C2;
	CurColor=GetPixelColor(X,Y);
	
	//Calculate new pixel color 
	if(XPan && YPan)
	{   
		if(X<Width-1)
		{
			C2=GetPixelColor(X+1,Y);
			CurColor=AverageColors(CurColor,C2,XPan);}

		if(Y<Height-1 && X<Width-1)
		{
			C1=GetPixelColor(X,Y+1);
			C2=GetPixelColor(X+1,Y+1);
			NextColor=AverageColors(C1,C2,XPan);
			CurColor=AverageColors(CurColor,NextColor,YPan);}

		else if(Y<Height-1)
		{
			C2=GetPixelColor(X,Y+1);
			CurColor=AverageColors(CurColor,C2,YPan);}}

	else if(XPan && X<Width-1)
	{	
		NextColor=GetPixelColor(X+1,Y);
		CurColor=AverageColors(CurColor,NextColor,XPan);}

	else if(YPan && Y<Height-1)
	{	
		NextColor=GetPixelColor(X,Y+1);
		CurColor=AverageColors(CurColor,NextColor,YPan);}

	return CurColor;
}



/********************************************************************
*																	*
*	COLORREF Bitmap::SetPixelFiltered(float X, float Y)				*
*																	*
*	Creates a color for the given X & Y floats by means of			*
*	bilinear filtering. X & Y must be calculated such as to			*
*	reflect	(WholeX.XPan,WholeY.YPan), where (WholeX,WholeY)		*
*	is the coord for the top left pixel in the filtering,			*
*	and XPan & YPan are values to control the level of				*
*	color mergeance for AverageColors().							*
*																	*
*	Returns:														*
*		The new pixel color, -1 (0xFFFFFFFF) on failure.			*
*																	*
********************************************************************/

int Bitmap::SetPixelFiltered(double dX, double dY, BYTE ubRadius, COLORREF crBaseColor)
{
	if(!CheckXY(dX,dY) || Bpp<16) return 0xFFFFFFFF;

	double dXPan=dX-(int)dX, dYPan=dY-(int)dY, dPan;
	COLORREF crBGColor;
		
	
	/*Set exact pixel color*/
	//SetPixelColor(dX,dY,crBaseColor);

	for(BYTE ubCurRadius=0; ubCurRadius<=ubRadius; ubCurRadius++)
	{
		crBGColor=RGB(255,255,255);

		/*U,R,D,L*/
		dPan=dYPan+double(ubCurRadius)/4;
		crBGColor=GetPixelColor(dX,dY+ubCurRadius);
		SetPixelColor(dX,dY+ubCurRadius,AverageColors(crBaseColor,crBGColor,dPan));

		dPan=dXPan+double(ubCurRadius)/4;
		crBGColor=GetPixelColor(dX+ubCurRadius,dY);
		SetPixelColor(dX+ubCurRadius,dY,AverageColors(crBaseColor,crBGColor,dPan));

		dPan=dYPan+double(ubCurRadius)/4;
		crBGColor=GetPixelColor(dX,dY-ubCurRadius);
		SetPixelColor(dX,dY-ubCurRadius,AverageColors(crBaseColor,crBGColor,dPan));
		
		dPan=dXPan+double(ubCurRadius)/4;
		crBGColor=GetPixelColor(dX-ubCurRadius,dY);
		SetPixelColor(dX-ubCurRadius,dY,AverageColors(crBaseColor,crBGColor,dPan));
		

		/*UR,DR,DL,UL*/
		dPan=(dXPan+dYPan)+double(ubCurRadius)/4;
		crBGColor=GetPixelColor((dX+ubCurRadius)/2,(dY+ubCurRadius)/2);
		SetPixelColor(dX+ubCurRadius/2,dY+ubCurRadius/2,AverageColors(crBaseColor,crBGColor,dPan));
		
		dPan=(dXPan+dYPan)+double(ubCurRadius)/4;
		crBGColor=GetPixelColor((dX+ubCurRadius)/2,(dY-ubCurRadius)/2);
		SetPixelColor(dX+ubCurRadius/2,dY-ubCurRadius/2,AverageColors(crBaseColor,crBGColor,dPan));
		
		dPan=(dXPan+dYPan)+double(ubCurRadius)/4;
		crBGColor=GetPixelColor(dX-ubCurRadius/2,(dY-ubCurRadius)/2);
		SetPixelColor(dX-ubCurRadius/2,dY-ubCurRadius/2,AverageColors(crBaseColor,crBGColor,dPan));
		
		dPan=(dXPan+dYPan)+double(ubCurRadius)/4;
		crBGColor=GetPixelColor((dX-ubCurRadius)/2,(dY+ubCurRadius)/2);
		SetPixelColor(dX-ubCurRadius/2,dY+ubCurRadius/2,AverageColors(crBaseColor,crBGColor,dPan));
	}

	return 0;
}


/********************************************************************
*																	*
*	COLORREF Bitmap::GetPaletteEntry(int PIndex)					*
*																	*
*	Returns:														*
*		The palette color for the given palette index (0-255),		*
*		-1 (0xFFFFFFFF) on failure.									*
*																	*
********************************************************************/

COLORREF Bitmap::GetPaletteEntry(int PIndex)
{
	if(PIndex<0 || PIndex>Colors-1 || Bpp>8) return 0xFFFFFFFF;

	return RGBALPHA(Palette[PIndex].rgbRed, 
					Palette[PIndex].rgbGreen, 
					Palette[PIndex].rgbBlue,
					Palette[PIndex].rgbReserved);
}


/********************************************************************
*																	*
*	COLORREF Bitmap::SetPaletteEntry(int PIndex, COLORREF NPE)		*
*																	*
*	Sets the palette color for the given palette index (0-255).		*
*																	*
*	Returns:														*
*		Old index color, -1 (0xFFFFFFFF) on failure.				*
*																	*
********************************************************************/

COLORREF Bitmap::SetPaletteEntry(int PIndex, COLORREF NPE)
{
	if(PIndex<0 || PIndex>Colors-1 || Bpp>8) return 0xFFFFFFFF;
	
	COLORREF OPE=RGBALPHA(Palette[PIndex].rgbRed, 
						  Palette[PIndex].rgbGreen, 
						  Palette[PIndex].rgbBlue,
						  Palette[PIndex].rgbReserved);

	CRtoRGBQ(Palette[PIndex],NPE);
	CRtoRGBQ(BMPInfo->bmiColors[PIndex],NPE);

	return OPE;
}

/********************************************************************
*																	*
*	int Bitmap::SetPalette(const RGBQUAD*)							*
*																	*
*	Sets the palette to a given RGBQUAD array.						*
*																	*
********************************************************************/

int Bitmap::SetPalette(const RGBQUAD *RGBQPal)
{
	if(Bpp>8) return 0;

	memcpy(Palette, RGBQPal, Colors<<2);
	memcpy(BMPInfo->bmiColors, RGBQPal, Colors<<2);
	return 1;
}


/********************************************************************
*																	*
*	int Bitmap::SetPalette(const *BYTE BytePal)						*
*																	*
*	Sets the palette to a given BYTE array.							*
*																	*
********************************************************************/

int Bitmap::SetPalette(const BYTE *BytePal)
{
	if(Bpp>8) return 0;

	for(int X=0; X<Colors; X++)
		SetPaletteEntry(X,RGB(BytePal[X*3],BytePal[X*3+1],BytePal[X*3+2]));
	return 1;
}

/********************************************************************
*																	*
*	int Bitmap::SetPaletteToGrayScale()								*
*																	*
*	Sets the bitmaps palette to gray scale of the Bpp.				*
*																	*
********************************************************************/

int Bitmap::SetPaletteToGrayScale()
{
	if(Bpp>8) return 0;
	
	if(Bpp==8)
		for(int I=0; I<256; I++) SetPaletteEntry(I,RGB(I,I,I));
	
	if(Bpp==4)
		for(int I=0; I<16; I++) SetPaletteEntry(I,RGB(I<<4,I<<4,I<<4));
	
	if(Bpp==1)
	{
		SetPaletteEntry(0,RGB(0,0,0));
		SetPaletteEntry(1,RGB(255,255,255));}

	return 1;
}


/********************************************************************
*																	*
*	int Bitmap::LoadPalette(const char* PalFileName)				*
*																	*
*	Load a palette from file into the bitmap.						*
*																	*
*	Returns: 0 on success, Bitmap Error Code on failure.			*
*																	*
********************************************************************/

int Bitmap::LoadPalette(const char* PalFileName)
{
	if(Bpp>8) return 0;
	
	FILE *PalFile;
	if(!(PalFile=fopen(PalFileName, "rb"))) return BM_OPENERR;
	
	//This loads 0BGR RGBQUAD Palette (1KB for 8-bit)
	//if(!fread(Palette, 4, Colors, PalFile)) return BM_READERR;
	
	//This loads RGB Palette (768 bytes for 8-bit)
	for(int C=0; C<Colors; C++)
	{
		Palette[C].rgbRed=getc(PalFile);
		Palette[C].rgbGreen=getc(PalFile);
		Palette[C].rgbBlue=getc(PalFile);}

	memcpy(BMPInfo->bmiColors, Palette, Colors<<2);
	fclose(PalFile);
	return 0;
}


/********************************************************************
*																	*
*	int Bitmap::SavePalette(const char* PalFileName)				*
*																	*
*	Saves the Bitmap palette to a file on the specified name.		*
*																	*
*	Returns: 0 on success, Bitmap Error Code on failure.			*
*																	*
********************************************************************/

int Bitmap::SavePalette(const char * PalFileName)
{
	if(Bpp>8) return 0;
	
	FILE *PalFile=fopen(PalFileName, "wb");
	
	//This writes 0BGR RGBQuad Palette
	//if(!fwrite(Palette, 4, Colors, PalFile)) return BM_WRITEERR;
	
	//This writes RGB palette
	for(int C=0; C<Colors; C++)
	{
		putc(Palette[C].rgbRed,PalFile);
		putc(Palette[C].rgbGreen,PalFile);
		putc(Palette[C].rgbBlue,PalFile);}
	
	fclose(PalFile);
	return 0;
}


/********************************************************************
*																	*
*	void Bitmap::Resize(int NewWidth, int NewHeight,				*
*						int Filter=0, int Scanlines=0)				*								*
*																	*
*	Resizes the bitmap to the given dimensions, with				*
*	or without bi-linear filtering and scanlines.					*
*																	*
********************************************************************/

void Bitmap::Resize(int NewWidth, int NewHeight, 
					int Filter, int Scanlines)
{
	Bitmap *ResizeBmp=new Bitmap(NewWidth,NewHeight,Bpp);
	ResizeBmp->SetPalette(Palette);
	
	float WRatio=(float)(NewWidth-1)/(Width-1);
	float HRatio=(float)(NewHeight-1)/(Height-1);
	
	COLORREF CurColor;
	int CurIndex;

	for(int Y=0; Y<NewHeight; Y++)		 
		for(int X=0; X<NewWidth; X++)	
		{				
			float OrgX=X/WRatio, OrgY=Y/HRatio; 
			
			if(Scanlines && Bpp>8)
			{
				if(Y%2)
				{
					ResizeBmp->SetPixelColor(X,Y,RGB(0,0,0));
					continue;}
				OrgY=(Y>>1);}

			if(Bpp>8)
			{
				if(Filter && Bpp>8)	CurColor=GetPixelFiltered(OrgX,OrgY);			
				else CurColor=GetPixelColor(OrgX,OrgY);
				ResizeBmp->SetPixelColor(X,Y,CurColor);}

			else //Indexed Color
			{
				CurIndex=GetPixelIndex(OrgX,OrgY);
				ResizeBmp->SetPixelIndex(X,Y,CurIndex);}}			
		
	*this=*ResizeBmp;
	delete ResizeBmp;
}


/********************************************************************
*																	*
*	void Bitmap::Mirror(int Direction)								*
*																	*
*	Mirrors the Bitmap in the given direction:						*
*	(BM_HORIZONTAL or BM_VERTICAL)									*
*																	*
********************************************************************/

void Bitmap::Mirror(int Direction)
{
	Bitmap *MirrorBmp=new Bitmap(Width,Height,Bpp);
	MirrorBmp->SetPalette(Palette);

	for(int Y=0; Y<Height; Y++)
		for(int X=0; X<Width; X++)
		{
			int NX,NY;
			if(Direction==BM_HORIZONTAL) {NX=Width-1-X; NY=Y;}
			if(Direction==BM_VERTICAL)	 {NX=X;NY=Height-1-Y;}

			if(Bpp>8) 
			{
				COLORREF CurColor=GetPixelColor(NX,NY);
				MirrorBmp->SetPixelColor(X,Y,CurColor);}

			else //Indexed Color	
			{	
				int CurIndex=GetPixelIndex(NX,NY);
				MirrorBmp->SetPixelIndex(X,Y,CurIndex);}}

	*this=*MirrorBmp;
	delete MirrorBmp;
}


/********************************************************************
*																	*
*	void Bitmap::Rotate90()											*
*																	*
*	Rotates the Bitmap clockwise 90 degrees.						*
*																	*
********************************************************************/

void Bitmap::Rotate90()
{
	Bitmap *RotateBmp=new Bitmap(Height,Width,Bpp);
	RotateBmp->SetPalette(Palette);

	for(int Y=0; Y<Width; Y++)
		for(int X=0; X<Height; X++)
		{
			if(Bpp>8) 
			{
				COLORREF CurColor=GetPixelColor(Y,Height-1-X);
				RotateBmp->SetPixelColor(X,Y,CurColor);}

			else //Indexed Color
			{	
				int CurIndex=GetPixelIndex(Y,Height-1-X);
				RotateBmp->SetPixelIndex(X,Y,CurIndex);}}

	*this=*RotateBmp;
	delete RotateBmp;
}


/********************************************************************
*																	*
*	void Bitmap::Rotate(float Deg, int Filter=0)					*
*																	*
*	Rotates the Bitmap clockwise X degrees, with the				*
*	option to use or not to use bi-linear filtering.				*
*																	*
********************************************************************/

void Bitmap::Rotate(float Deg, int Filter)
{
	while(Deg>360) Deg-=360;
	if(!Deg) return;

	//Calculate new dimensions
	float Rad=Deg*PI/180, SIN=sin(Rad), COS=cos(Rad), TAN=SIN/COS;
	float H1=RoundToInt(SIN*Width);
	float W1=RoundToInt(COS*Width);		
	float W2=RoundToInt(SIN*Height);
	float H2=RoundToInt(COS*Height);
	int NewWidth=RoundToInt(W1+W2);
	int NewHeight=RoundToInt(H1+H2);
	
	Bitmap *RotateBmp=new Bitmap(NewWidth,NewHeight,Bpp);
	RotateBmp->SetPalette(Palette);
	memset(RotateBmp->Buffer,255,RotateBmp->DataSize);
		
	//X1=((X0+1)*COS+W2-(Y0+1)*SIN)-1
	//Y1=((X0+1)*SIN+(Y0+1)*COS)-1;
	COLORREF CurColor;
	for(int Y=0; Y<NewHeight; Y++)
		for(int X=0; X<NewWidth; X++)
		{
			double X0=(((X+1)+(Y+1)*TAN-W2-.5)/(COS+TAN*SIN));
			double Y0=(((Y+1)-(X0)*SIN)/COS)-1;
			if(X0<0 || X0>=Width || Y0<0 || Y0>=Height) continue;

			if(Filter && Bpp>8) 
			{
				CurColor=GetPixelFiltered(X0,Y0);
				RotateBmp->SetPixelColor((int)X,(int)Y,CurColor);}

			else if(Bpp>8)
			{
				if(!CheckXY(X0,Y0)) continue;
				CurColor=GetPixelColor((int)X0,(int)Y0);
				RotateBmp->SetPixelColor((int)X,(int)Y,CurColor);}

			else //Indexed Color
			{
				if(!CheckXY(X0,Y0)) continue;
				int Index=GetPixelIndex((int)X0,(int)Y0);
				RotateBmp->SetPixelIndex(X,Y,Index);}}
	
	*this=*RotateBmp;
	delete RotateBmp;
}


/********************************************************************
*																	*
*	void Bitmap::InvertColors(int IR=1, int IG=1, int IB=1)			*
*																	*
*	Inverts the color channels of the image. You can pass true or	*
*	false for the inversion of individual channels.					*
*	The default is all channels.									*
*																	*
********************************************************************/

void Bitmap::InvertColors(int IR, int IG, int IB)
{
	if(Bpp>16)
	{
		for(int Y=0; Y<Height; Y++)
			for(int X=0; X<Width; X++)
			{	
				int pixel=Offset(X,Y);
				if(IR) Buffer[pixel]=255-Buffer[pixel];
				if(IG) Buffer[pixel+1]=255-Buffer[pixel+1];
				if(IB) Buffer[pixel+2]=255-Buffer[pixel+2];}}

	else if(Bpp<16)
	{
		for(int P=0; P<Colors; P++)
		{
			if(IR) Palette[P].rgbRed=255-Palette[P].rgbRed;
			if(IG) Palette[P].rgbGreen=255-Palette[P].rgbGreen;
			if(IB) Palette[P].rgbBlue=255-Palette[P].rgbBlue;
			BMPInfo->bmiColors[P]=Palette[P];}}

	else //16-Bit (Slow, but who uses 16-bit anyway?)
	{
		for(int Y=0; Y<Height; Y++)
			for(int X=0; X<Width; X++)
			{
				COLORREF ICol=GetPixelColor(X,Y);
				int NR,NG,NB;
				if(IR) NR=248-CRR(ICol);
				else   NR=CRR(ICol);
				if(IG) NG=248-CRG(ICol);
				else   NG=CRG(ICol);
				if(IB) NB=248-CRB(ICol);
				else   NB=CRB(ICol);
				SetPixelColor(X,Y,RGB(NR,NG,NB));}}
}				


/********************************************************************
*																	*
*	void Bitmap::GrayScale()										*
*																	*
*	Converts the Bitmap to gray scale of its current color depth,	*
*	or 24-bit if the original was 16-bit or 32-bit.					*
*																	*
********************************************************************/

void Bitmap::GrayScale()
{
	if(Bpp<8)
	{
		for(int I=0; I<Colors; I++)
		{
			int Int=Intensity(GetPaletteEntry(I));
			SetPaletteEntry(I,RGB(Int,Int,Int));}
		
		return;}
	
	Bitmap *GrayBmp=new Bitmap(Width,Height,Bpp);
	GrayBmp->SetPaletteToGrayScale();

	for(int Y=0; Y<Height; Y++)
		for(int X=0; X<Width; X++)
		{
			int Int=GetPixelIntensity(X,Y);
			
			if(Bpp>16) memset(GrayBmp->Buffer+GrayBmp->Offset(X,Y),Int,3);
			else if(Bpp==16) GrayBmp->SetPixelColor(X,Y,RGB(Int,Int,Int));
			else if(Bpp==8) GrayBmp->SetPixelIndex(X,Y,Int);}
			
	*this=*GrayBmp;
	delete GrayBmp;
}


/********************************************************************
*																	*
*	void Bitmap::ConvertTo8BitGrayScale()							*
*																	*
*	Converts a true color image to 8-bit grayscale.					*
*																	*
********************************************************************/

void Bitmap::ConvertTo8BitGrayScale()
{
	if(Bpp<16) return;
	
	GrayScale();
	
	Bitmap *GrayBmp=new Bitmap(Width,Height,8);
	GrayBmp->SetPaletteToGrayScale();
	
	for(int Y=0; Y<Height; Y++)
		for(int X=0; X<Width; X++)
			GrayBmp->SetPixelIndex(X,Y,Buffer[Offset(X,Y)]);
		
	*this=*GrayBmp;
}


/********************************************************************
*																	*
*	void Bitmap::ConvertTo24Bit()									*
*																	*
*	Converts the Bitmap to 24-bit.									*
*																	*
********************************************************************/

void Bitmap::ConvertTo24Bit()
{
	if(Bpp==24) return;

	Bitmap *Bmp24=new Bitmap(Width,Height,24);

	for(int Y=0; Y<Height; Y++)
		for(int X=0; X<Width; X++)
			Bmp24->SetPixelColor(X,Y,GetPixelColor(X,Y));

	*this=*Bmp24;
	delete Bmp24;
}


/********************************************************************
*																	*
*	int Bitmap::ConvertToXBit(int NewBpp)							*
*																	*
*	Converts the Bitmap to a bit depth of 1-8 bits.					*
*																	*
*	Returns: 1 on success, 0 on failure, or the ConvertTo24Bit()	*
*			 return if the new color depth given was 24.			*
*																	*
********************************************************************/

int Bitmap::ConvertToXBit(int NewBpp)
{
	if(Bpp==NewBpp) return 1;
	
	if(NewBpp==24) 
	{
		ConvertTo24Bit();
		return 1;}

	else if(NewBpp>8) return 0;
	
	COLORREF Palette[257], CurColor;
	int MaxCols=(int)pow(2.0,NewBpp), X, Y;
	long NumCols=0;

	//Insert all distinct colors into the temp. Palette
	for(Y=0; Y<Height; Y++)
		for(X=0; X<Width; X++)
		{
			CurColor=GetPixelColor(X,Y);
			Insert(CurColor,NumCols,Palette);
			//Exit if number of colors exceedes max
			if(NumCols>MaxCols) return 0;}

	//Copy assembled Palette to the Temp Bitmap's Palette  
	Bitmap *BmpX=new Bitmap(Width,Height,NewBpp);
	for(int P=0; P<NumCols; P++)
		BmpX->SetPaletteEntry(P,Palette[P]);

	//Set the Temp Bitmap's Pixels to the correct palette index
	for(Y=0; Y<Height; Y++)
		for(X=0; X<Width; X++)
		{
			CurColor=GetPixelColor(X,Y);
			
			for(int P=0; P<NumCols; P++)
				if(CurColor==Palette[P])
					BmpX->SetPixelIndex(X,Y,P);}

	*this=*BmpX;
	delete BmpX;
	return 1;
}


/********************************************************************
*																	*
*	int Bitmap::Paste(const Bitmap &SrcBmp, int SrcX, int SrcY,		*	
*					  int SrcW, int SrcH, int DestX, int DestY)		*									*
*																	*
*	Pastes the image from SrcBmp (SrcX,SrcY) of Dimensions			*
*	SrcWxSrcH, to the calling Bitmap (DestX,DestY).					*  											*	
*	Both Bitmaps must be of the same color depth.					*	
*																	*
*	Returns: 1 on success, 0 on failure.							*
*																	*
********************************************************************/

int Bitmap::Paste(const Bitmap &SrcBmp, int SrcX, int SrcY, 
				  int SrcW, int SrcH, int DestX, int DestY)
{
	if(Bpp!=SrcBmp.Bpp) return 0;
	
	if(SrcX<0 || SrcY<0 || SrcW<0 || 
	   SrcH<0 || DestX<0 || DestY<0) return 0;

	/*Clipping*/
	if(SrcW>SrcBmp.Width-SrcX) SrcW=SrcBmp.Width-SrcX;
	if(SrcW>Width-DestX) SrcW=Width-DestX;
	if(SrcH>SrcBmp.Height-SrcY) SrcH=SrcBmp.Height-SrcY;
	if(SrcH>Height-DestY) SrcH=Height-DestY;

	int PixelSize=int(Bpp/8.0);
	int HPD=Height-1-DestY, HPS=SrcBmp.Height-1-SrcY;

	for(int Y=0; Y<SrcH; Y++)
		memcpy(Buffer+(HPD-Y)*ScanLineSize+(DestX*PixelSize),
			SrcBmp.Buffer+(HPS-Y)*SrcBmp.ScanLineSize+(SrcX*PixelSize),
			SrcW*PixelSize);

	return 1;
}
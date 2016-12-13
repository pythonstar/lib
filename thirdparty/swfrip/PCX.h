//PCX File Class [PCX.h]
//Wesley Hopper (HopperW2000@Yahoo.com)

//Supports all color depths of PCX, but
//currenly has no support for grayscale PCX.


#ifndef _PCX_H
#define _PCX_H

#include <stdio.h>
#include <malloc.h>
#include <math.h>


struct PCXFILEHEADER {	//pcx
	
	BYTE	pcxManufacturer;	//Manufacturer (10=ZSoft)
	BYTE	pcxVersion;			//PCX Version Info
	BYTE	pcxEncoding;		//Encoding Info (1=PCX RLE)
	BYTE	pcxBitsPerPixel;	//Bits Per Pixel
	WORD	pcxXMin;			//Image Dimensions
	WORD	pcxYMin;
	WORD	pcxXMax;
	WORD	pcxYMax;
	WORD	pcxHDpi;			//Horizontal Resolution DPI
	WORD	pcxVDpi;			//Vertical Resolution DPI
	
	BYTE	pcxColorMap[48];	//16 Color Palette (0x10)
	BYTE	pcxReserved;		//Should be 0 (0x40)
	BYTE	pcxPlanes;			//Number of Color Planes
	WORD	pcxBytesPerLine;	//Scan Line Size (Even)
	WORD	pcxPaletteInfo;		//1=Color/BW 2=Gray Scale
	WORD	pcxHScreenSize;		//Horizontal Screen Size
	WORD	pcxVScreenSize;		//Vertical Screen Size
	BYTE	pcxFiller[54];		//Blank Filler
};


class PCX
{
	public:
		PCX()						{Buffer=NULL;}
		PCX(const char* InitName)	{Buffer=NULL; Load(InitName);}
		PCX(int,int,int,BYTE*);
		int Load(const char*);
		int Save(const char*);
		int SetPalette(const RGBQUAD*);

		BYTE *Buffer, Palette[768];
		int	Width, Height, Bpp, Planes, DataSize, SLSize;

	private:
		void EncRun(BYTE,BYTE,FILE*);
		PCXFILEHEADER PCXHeader;
};


PCX::PCX(int W, int H, int CDepth=8, BYTE *Data=NULL)
{
	Width=W;
	Height=H;
	if(CDepth==24)	{Bpp=8; Planes=3;}
	else			{Bpp=CDepth; Planes=1;}
	
	DataSize=RoundToInt(Width*(float)(Bpp*Planes)/8);
	if(DataSize%4) DataSize+=4-DataSize%4;
	SLSize=DataSize;
	DataSize*=Height;
		
	memset(&PCXHeader,0,sizeof(PCXFILEHEADER));
	PCXHeader.pcxManufacturer=10;
	PCXHeader.pcxVersion=5;		
	PCXHeader.pcxEncoding=1;		
	PCXHeader.pcxBitsPerPixel=Bpp;	
	PCXHeader.pcxXMin=0;			
	PCXHeader.pcxYMin=0;
	PCXHeader.pcxXMax=Width-1;
	PCXHeader.pcxYMax=Height-1;
	//PCXHeader.pcxHDpi;			//Horizontal Resolution DPI
	//PCXHeader.pcxVDpi;			//Vertical Resolution DPI
	PCXHeader.pcxPlanes=Planes;		
	PCXHeader.pcxBytesPerLine=SLSize/Planes;	
	PCXHeader.pcxPaletteInfo=1;		
	//PCXHeader.pcxHScreenSize;		//Horizontal Screen Size
	//PCXHeader.pcxVScreenSize;		//Vertical Screen Size

	//Allocate Buffer
	Buffer=(BYTE*)malloc(DataSize);
	if(Data)
		for(int Line=0; Line<Height; Line++)
			memcpy(Buffer+(Height-1-Line)*SLSize, 
				   Data+Line*SLSize, SLSize);
}


int PCX::Load(const char *FileName)
{
	//Open file
	FILE *PCXFile;
	if(!(PCXFile=fopen(FileName,"rb"))) return 0;
	fread(&PCXHeader, 1, sizeof(PCXFILEHEADER), PCXFile);

	//Calculate
	Width=PCXHeader.pcxXMax-PCXHeader.pcxXMin+1;
	Height=PCXHeader.pcxYMax-PCXHeader.pcxYMin+1;
	Bpp=PCXHeader.pcxBitsPerPixel;
	Planes=PCXHeader.pcxPlanes;
	
	DataSize=RoundToInt(Width*(float)(Bpp*Planes)/8);
	if(DataSize%4) DataSize+=4-DataSize%4;
	SLSize=DataSize;
	DataSize*=Height;

	//Read Encoded Data
	Buffer=(BYTE*)malloc(DataSize);
	int ByteCnt=0, DupCnt;
	BYTE CurByte, *BufTemp=Buffer;
	for(int L=0; L<Height; L++)
	{
		DupCnt=0; //Restart run at new scanline
		for(int P=Planes-1; P>=0; P--) //Decode each scanline
		{	
			ByteCnt=0;	
			Buffer=BufTemp+(L*SLSize+P);
			while(ByteCnt<PCXHeader.pcxBytesPerLine) //Decode each plane
			{	
				if(!DupCnt)
				{
					DupCnt=1;
					CurByte=getc(PCXFile);
						
					if(0xC0==(0xC0 & CurByte))		
					{
						DupCnt=0x3F & CurByte;
						CurByte=getc(PCXFile);}}
					
				while(DupCnt>0)
				{
					*Buffer=CurByte;
					Buffer+=Planes;
					ByteCnt++;
					DupCnt--;
					if(ByteCnt>=PCXHeader.pcxBytesPerLine) break;}}}}

	Buffer=BufTemp;

	//Get Palette
	if(Bpp<8) memcpy(Palette, PCXHeader.pcxColorMap, 48);
	if(Bpp==8)
	{
		fseek(PCXFile, -769, SEEK_END);
		if(getc(PCXFile)==12) fread(Palette, 3, 256, PCXFile);}
		
	fclose(PCXFile);
	return 1;
}


void PCX::EncRun(BYTE LastByte, BYTE RunCount, FILE *File)
{
	if(RunCount)
	{
		//If not encoded, write normal byte
		if(RunCount==1 && 0xC0!=(0xC0 & LastByte))
			putc(LastByte, File);

        else //Write encoded run
		{
			putc(0xC0 | RunCount, File);
			putc(LastByte, File);}}
}


int PCX::Save(const char *SaveName)
{
	//Open File
	FILE *PCXFile=fopen(SaveName,"wb");
	fwrite(&PCXHeader,1,sizeof(PCXFILEHEADER),PCXFile);
	
	//Write Encoded Data
	BYTE CurByte, LastByte, RunCount=1, *BufTemp=Buffer;
	for(int L=0; L<Height; L++)
	{
		RunCount=1;	//Restart run at new scanline
		Buffer=BufTemp+(L*SLSize+Planes-1);
		LastByte=*Buffer;
		for(int P=Planes-1; P>=0; P--) //Encode each plane
		{
			Buffer=BufTemp+(L*SLSize+P);
			for(int Index=0; Index<PCXHeader.pcxBytesPerLine; Index++)
			{	
				if(P==Planes-1 && Index==0)
				{Buffer+=Planes; Index++;}

				CurByte=*Buffer; //Continue run
			    if(CurByte==LastByte)     
				{
					RunCount++;
			        if(RunCount==63) //Max run length, end & write
					{
						EncRun(LastByte, RunCount, PCXFile);
						RunCount=0;}}
			    else //End run & write data
				{
					if(RunCount) EncRun(LastByte, RunCount, PCXFile);
						
				    LastByte=CurByte;
					RunCount=1;}
		
				Buffer+=Planes;}} 

		if(RunCount) EncRun(LastByte, RunCount, PCXFile);}

	Buffer=BufTemp;

	//Write palette 256 color
	if(Bpp==8 && Planes==1)
	{
		putc(12,PCXFile);
		fwrite(Palette,3,256,PCXFile);}

	fclose(PCXFile);
	return 0;
}


int PCX::SetPalette(const RGBQUAD *RGBQPal)
{
	if(Bpp>8) return 0;
	
	if(Bpp==8)
		for(int X=0; X<256; X++)
		{
			Palette[X*3]=RGBQPal[X].rgbRed;
			Palette[X*3+1]=RGBQPal[X].rgbGreen;
			Palette[X*3+2]=RGBQPal[X].rgbBlue;}

	if(Bpp<8)
	{
		for(int X=0; X<16; X++)
		{
			PCXHeader.pcxColorMap[X*3]=RGBQPal[X].rgbRed;
			PCXHeader.pcxColorMap[X*3+1]=RGBQPal[X].rgbGreen;
			PCXHeader.pcxColorMap[X*3+2]=RGBQPal[X].rgbBlue;}
		//Copy from header to the Palette
		memcpy(Palette,PCXHeader.pcxColorMap,48);}

	return 1;
}

#endif //PCX_H
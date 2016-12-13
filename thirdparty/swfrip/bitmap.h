/********************************************************************
		
				Windows Bitmap File Class [Bitmap.h]
	
				Wesley Hopper (HopperW2000@Yahoo.com)


		This is a class for loading, creating, editing, and saving 
	Windows bitmaps. It can be used under systems other than Windows 
	since all the necessary definitions are included in this header. 
	I think the only thing not supported is RLE compressed Bitmaps.
	
		All functions treat the bitmap as Bottom-Up format, even if 
	the data was fliped during loading.

********************************************************************/

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <windows.h>
#include <png.h>


#ifndef	BITMAP_H
#define	BITMAP_H


/*******************Windows Definitions********************/
/*
#pragma pack(2) //Align structs on WORD boundry

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef long LONG;
typedef unsigned long ULONG;
typedef unsigned long DWORD;

typedef unsigned long COLORREF;

#define RGB(r,g,b) ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

typedef struct tagRGBQUAD {
        BYTE    rgbBlue;
        BYTE    rgbGreen;
        BYTE    rgbRed;
        BYTE    rgbReserved;
} RGBQUAD;
//typedef RGBQUAD FAR* LPRGBQUAD;

typedef struct tagBITMAPFILEHEADER {
        WORD    bfType;
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
} BITMAPFILEHEADER;//, FAR *LPBITMAPFILEHEADER, *PBITMAPFILEHEADER;


typedef struct tagBITMAPCOREHEADER {
        DWORD   bcSize;                 
        WORD    bcWidth;
        WORD    bcHeight;
        WORD    bcPlanes;
        WORD    bcBitCount;
} BITMAPCOREHEADER;//, FAR *LPBITMAPCOREHEADER, *PBITMAPCOREHEADER;

typedef struct tagBITMAPINFOHEADER{
        DWORD      biSize;
        LONG       biWidth;
        LONG       biHeight;
        WORD       biPlanes;
        WORD       biBitCount;
        DWORD      biCompression;
        DWORD      biSizeImage;
        LONG       biXPelsPerMeter;
        LONG       biYPelsPerMeter;
        DWORD      biClrUsed;
        DWORD      biClrImportant;
} BITMAPINFOHEADER;//, FAR *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef struct tagBITMAPINFO {
    BITMAPINFOHEADER    bmiHeader;
    RGBQUAD             bmiColors[1];
} BITMAPINFO;//, FAR *LPBITMAPINFO, *PBITMAPINFO;

*/

/*****************Bitmap Class Definitions*****************/

#ifndef PI
#define PI 3.1415926535897932384626433832795
#endif


//Data loading formats
#define BM_NORMAL		0x0001 //Bottom-Up (Default)
#define BM_FLIP			0x0002 //Right Side Up

//Color Formats
#define BM_RGBCOLOR		0x0003	
#define BM_PALCOLOR		0x0004

//Bitmap Error Codes
#define BM_NOTBITMAP	0x0005
#define BM_OPENERR		0x0006
#define BM_CREATEERR	0x0007
#define BM_ALLOCERR		0x0008
#define BM_READERR		0x0009
#define BM_WRITEERR		0x000A

//Header Type
#define BM_INFOHEADER	0x000B	//54 Bytes
#define BM_COREHEADER	0x000C	//26 Bytes

//Mirror Directions
#define BM_HORIZONTAL	0x000D
#define BM_VERTICAL		0x000E

/*make a COLORREF with Alpha*/
#define RGBALPHA(r,g,b,a) ((COLORREF)((DWORD)r) | ((DWORD)g)<<8 | ((DWORD)b)<<16 | ((DWORD)a)<<24)

/*Sets the given RGBQUAD struct with the data of the given COLORREF*/
#define CRtoRGBQ(RGBQ, CR)	RGBQ.rgbReserved=(BYTE)(CR>>24);\
							RGBQ.rgbBlue=(BYTE)(CR>>16);\
							RGBQ.rgbGreen=(BYTE)(CR>>8);\
							RGBQ.rgbRed=(BYTE)CR;

/*Gives the Red, Green, or Blue value of the given COLORREF*/
#define CRR(CR)	((BYTE)(CR))
#define CRG(CR)	((BYTE)(CR>>8))
#define CRB(CR)	((BYTE)(CR>>16))
#define CRA(CR)	((BYTE)(CR>>24))

/*Gives the intensity of the given COLORREF*/
#define Intensity(Color) ((3*CRR(Color)+6*CRG(Color)+CRB(Color))/10)

/*Rounding functions*/
#define Round(X)		((X-int(X))>=.5? X+1:X) 
#define RoundToInt(X)	(int(Round(X)))
#define RoundUp(X)		(X>(int)X? ((int)(X+1)):X)
#define RoundToByte(X)  (RoundToInt(X)>255? 255:RoundToInt(X))


class Bitmap
{
	public:
		/*Default Constructor*/
		Bitmap() {Buffer=NULL; BMPInfo=NULL;} 

		/*Create New Bitmap*/
		Bitmap(int,int,int,BYTE *Data=NULL,int Format=BM_NORMAL); 
		
		/*Init and Load Bitmap from File*/
		Bitmap(const char *FileName,int BufferFormat=BM_NORMAL) 
			{Buffer=NULL; BMPInfo=NULL; Load(FileName,BufferFormat);}

		/*Destructor*/
		~Bitmap() 
		{
			if(Buffer) free(Buffer);
			if(BMPInfo) free(BMPInfo);
		}

		/*File Functions*/
		int	Load(const char*,int);
		int	Save(const char*);
		int	LoadPCX(const char*);
		int	SavePCX(const char*);
		int	SavePNG(const char*);
		
		
		/*Bitmap Assignment*/
		//Note: DO NOT define & assign in one 
		//statement like this (Bitmap BMP1=BMP2;)
		void		operator=(const Bitmap&);
		
		/*Pixel Functions*/
		COLORREF	GetPixelColor(int,int);
		COLORREF	GetPixelColor2432(int,int);
		COLORREF	GetPixelColor16(int,int);
		COLORREF	GetPixelColor8(int,int);
		COLORREF	GetPixelColor4(int,int);
		COLORREF	GetPixelColor1(int,int);
		
		void		SetPixelColor(int,int,COLORREF);
		void		SetPixelColor2432(int,int,COLORREF);
		void		SetPixelColor16(int,int,COLORREF);
		
		int			GetPixelIndex(int,int);
		int			GetPixelIndex8(int,int);
		int			GetPixelIndex4(int,int);
		int			GetPixelIndex1(int,int);
		
		void		SetPixelIndex(int,int,int);
		void		SetPixelIndex8(int,int,int);
		void		SetPixelIndex4(int,int,int);
		void		SetPixelIndex1(int,int,int);

		COLORREF	GetPixelFiltered(double,double);
		int			SetPixelFiltered(double,double,BYTE,COLORREF);
		int			GetPixelIntensity(int,int);
		
		/*Palette Functions*/
		COLORREF	GetPaletteEntry(int);
		COLORREF	SetPaletteEntry(int,COLORREF);
		int			SetPalette(const RGBQUAD*);
		int			SetPalette(const BYTE*);
		int			SetPaletteToGrayScale();
		int			LoadPalette(const char*);
		int			SavePalette(const char*);
		
		/*Transform Functions*/
		void		Resize(int,int,int=0,int=0);
		void		Mirror(int);
		void		Rotate90();
		void		Rotate(float,int=0);
		void		InvertColors(int=1,int=1,int=1);
		void		GrayScale();
		void		ConvertTo8BitGrayScale();
		void		ConvertTo24Bit();
		int			ConvertToXBit(int);
		int			Paste(const Bitmap&,int,int,int,int,int=0,int=0);

		/*Drawing Functions*/
		int DrawLine(ULONG ulStartX, ULONG ulStartY, LONG slDeltaX, LONG slDeltaY, BYTE ubWidth, COLORREF crColor)
		{
			double dSlope, dCurX=0, dCurY=0;
			ULONG ulEndX, ulEndY, ulPixels;
			LONG slDirection;

			if(!CheckXY(ulStartX,ulStartY)) return 0;

			if(slDeltaX==0) return DrawVerticalLine(ulStartX,ulStartY,slDeltaY,ubWidth,crColor);
	

			dSlope=float(slDeltaY)/slDeltaX;
			ulEndX=ulStartX+slDeltaX;
			ulEndY=ulStartY+slDeltaY;
			slDirection=(slDeltaX>0? 1:-1);
			ulPixels=(slDeltaX>0? slDeltaX:slDeltaX*-1);
						
			//for(DWORD ulPixel=0; ulPixel<ulPixels; ulPixel++)
			for(ULONG ulCurPixel=0; ulCurPixel<=ulPixels; ulCurPixel++) 
			{
				/*y=mx+b*/
				dCurX=ulStartX+ulCurPixel*slDirection;
				dCurY=ulStartY+dSlope*ulCurPixel*slDirection;
				SetPixelColor((int)dCurX,(int)dCurY,crColor);
			}
			
			return 1;
		}

		int DrawVerticalLine(ULONG ulStartX, ULONG ulStartY, LONG slDeltaY, BYTE ubWidth, COLORREF crColor)
		{
			if(!CheckXY(ulStartX,ulStartY)) return 0;

			ULONG ulEndY=ulStartY+slDeltaY;
			LONG  slDirection=(slDeltaY>0? 1:-1);
			ULONG ulPixels=(slDeltaY>0? slDeltaY:slDeltaY*-1);

			for(ULONG ulCurPixel=0; ulCurPixel<=ulPixels; ulCurPixel++) 
			{
				double dCurY=ulStartY+ulCurPixel*slDirection;
				SetPixelColor((int)ulStartX,(int)dCurY,crColor);
			}

			return 1;
		}
		
		/*Debug*/
		void		DumpHeaderData(const char*);

		/*Buffer offset to the data for pixel(X,Y)*/
		inline int	Offset(int X, int Y) 
		{return int((Height-1-Y)*ScanLineSize+X*(Bpp/8.0));}

		template <class TYPE> /*Checks for dimension range*/
		inline int	FlipY(TYPE Y) {return (TYPE(Height)-1-Y);}
		
		template <class TYPE> /*Checks for dimension range*/
		inline int	CheckXY(TYPE X, TYPE Y) 
					{if(X<0 || X>=Width || Y<0 || Y>=Height) return 0;
					 else return 1;}
		
		/*Image Info*/
		int Width, Height;	//Image Dimensions
		int Bpp, Colors;	//Color depth and max # of colors 
		ULONG DataSize;		//Size of image data buffer
		int ScanLineSize;	//Size of a scanline (including padding)
		BYTE			*Buffer;		//Pointer to data
		RGBQUAD		    Palette[256];	//RGBQUAD (BGR0) Palette
		BITMAPINFO		*BMPInfo;		//Info for display
		
	private:
		void		SetBIHeader();
		void		InitInfo(int);
				
		BITMAPFILEHEADER BMPHeader;		//Bitmap file info
		BITMAPINFOHEADER BMPInfoHeader; //Info header
		BITMAPCOREHEADER BMPCoreHeader; //Core header (secondary)
}; 


/********************************************************************
*																	*
*	HBITMAP Bitmap::CreateGDIBitmap(BITMAP *BmpStruct=NULL)			*
*																	*
*	Creates a WinGDI Bitmap from this Bitmap class					*
*	using CreateBitmapIndirect().									*
*																	*
*	Returns:														*
*		The HBITMAP for the Bitmap created.							*
*																	*
********************************************************************/
/*
HBITMAP Bitmap::CreateGDIBitmap(BITMAP *BmpStruct=NULL)
{
	if(!BmpStruct) BmpStruct=new BITMAP;
	BmpStruct->bmWidth=Width;
	BmpStruct->bmHeight=Height;
	BmpStruct->bmBitsPixel=Bpp;
	BmpStruct->bmPlanes=1;
	BmpStruct->bmType=0;
	BmpStruct->bmWidthBytes=ScanLineSize;
	BmpStruct->bmBits=Buffer;
	return(CreateBitmapIndirect(BmpStruct));
}
*/

/********************************************************************
*																	*
*	inline COLORREF Bitmap::GetPixelColor(int X, int Y)				*
*																	*
*	Returns:														*
*		The color of the pixel (X,Y), -1 (0xFFFFFFFF) on failure.	*
*																	*
********************************************************************/

inline COLORREF Bitmap::GetPixelColor(int X, int Y)
{
	if(!CheckXY(X,Y)) return 0xFFFFFFFF;
	if(Bpp>16)	return(GetPixelColor2432(X,Y));
	if(Bpp==16) return(GetPixelColor16(X,Y));
	if(Bpp==8)	return(GetPixelColor8(X,Y));
	if(Bpp==4)	return(GetPixelColor4(X,Y));
	if(Bpp==1)	return(GetPixelColor1(X,Y));
	return 0xFFFFFFFF;
}

inline COLORREF Bitmap::GetPixelColor2432(int X, int Y)
{
	int Index=Offset(X,Y);
	if(Bpp==24) return(RGB(Buffer[Index+2],Buffer[Index+1],Buffer[Index]));
	if(Bpp==32) return(RGBALPHA(Buffer[Index+3],Buffer[Index+2],Buffer[Index+1],Buffer[Index]));
	return 0xFFFFFFFF;
}

inline COLORREF Bitmap::GetPixelColor16(int X, int Y)
{
	int Index=Offset(X,Y);
	
	int R=(Buffer[Index+1]>>2)<<3;
	int G=(Buffer[Index+1]<<6)>>3;
	G=(G | (Buffer[Index]>>5))<<3;
	int B=Buffer[Index]<<3;
	
	return(RGB(R,G,B));
}

inline COLORREF Bitmap::GetPixelColor8(int X, int Y)
{
	int Index=Offset(X,Y);
	Index=Buffer[Index];

	return(RGB(Palette[Index].rgbRed,
			   Palette[Index].rgbGreen,
			   Palette[Index].rgbBlue));
}

inline COLORREF Bitmap::GetPixelColor4(int X, int Y)
{
	int Index=Offset(X,Y);

	if(X%2) Index=Buffer[Index] & 0x0F;
	else	Index=Buffer[Index]>>4;

	return(RGB(Palette[Index].rgbRed,
			   Palette[Index].rgbGreen,
			   Palette[Index].rgbBlue));
}

inline COLORREF Bitmap::GetPixelColor1(int X, int Y)		
{
	int Index=Offset(X,Y);
	int Sh=8-(X+1)%8;
	if(Sh==8) Sh=0;
	Index=(Buffer[Index]>>Sh) & 0x01;

	return(RGB(Palette[Index].rgbRed,
			   Palette[Index].rgbGreen,
			   Palette[Index].rgbBlue));
}


/********************************************************************
*																	*
*	COLORREF Bitmap::SetPixelColor(int X,int Y,COLORREF NewColor)	*
*																	*
*	Sets the pixel color for the given (X,Y),						*
*	(X & Y are visual top to bottom (0,0)=Top Left Corner).			*
*																	*
********************************************************************/

inline void Bitmap::SetPixelColor(int X, int Y, COLORREF NewColor)
{
	if(!CheckXY(X,Y) || Bpp<16) return;

	if(Bpp>16) SetPixelColor2432(X,Y,NewColor);
	else SetPixelColor16(X,Y,NewColor);	
}

inline void Bitmap::SetPixelColor2432(int X, int Y, COLORREF NewColor)
{
	int Index=Offset(X,Y);
	Buffer[Index]=CRB(NewColor);
	Buffer[Index+1]=CRG(NewColor);
	Buffer[Index+2]=CRR(NewColor);
	if(Bpp=32) Buffer[Index+3]=CRA(NewColor);
}

inline void Bitmap::SetPixelColor16(int X, int Y, COLORREF NewColor)
{
	int Index=Offset(X,Y);
	Buffer[Index]=(CRG(NewColor)>>3)<<5 | (CRB(NewColor)>>3);
	Buffer[Index+1]=(CRR(NewColor)>>3)<<2 | (CRG(NewColor)>>6);
}


/********************************************************************
*																	*
*	inline int GetPixelIndex(int X, int Y)							*
*																	*
*	Gets the palette index, (0-255), for the pixel (X,Y).			*
*																	*
*	Returns:														*
*		Palette index value, -1 (0xFFFFFFFF) on failure.			*
*																	*
********************************************************************/

inline int Bitmap::GetPixelIndex(int X, int Y)	
{
	if(!CheckXY(X,Y) || Bpp>8) return 0xFFFFFFFF;
	
	if(Bpp==8)	return(GetPixelIndex8(X,Y));
	if(Bpp==4)	return(GetPixelIndex4(X,Y));
	if(Bpp==1)	return(GetPixelIndex1(X,Y));

	return 0xFFFFFFFF;
}	
		
inline int Bitmap::GetPixelIndex8(int X, int Y)
{
	return(Buffer[Offset(X,Y)]);
} 

inline int Bitmap::GetPixelIndex4(int X, int Y)
{
	if(X%2) return(Buffer[Offset(X,Y)] & 0x0F);
	else	return(Buffer[Offset(X,Y)]>>4);
}

inline int Bitmap::GetPixelIndex1(int X, int Y)
{
	int Sh=8-(X+1)%8;
	if(Sh==8) Sh=0;
	return((Buffer[Offset(X,Y)]>>Sh) & 0x01);
}


/********************************************************************
*																	*
*	inline void SetPixelIndex(int X, int Y, int Index)				*
*																	*
*	Sets the palette index, (0-(2^BPP-1)), for the pixel (X,Y).		*
*																	*
********************************************************************/

inline void Bitmap::SetPixelIndex(int X, int Y, int NPI)
{
	if(!CheckXY(X,Y) || Bpp>8 || NPI>=Colors) return;
	
	if(Bpp==8)	SetPixelIndex8(X,Y,NPI);
	if(Bpp==4)	SetPixelIndex4(X,Y,NPI);
	if(Bpp==1)	SetPixelIndex1(X,Y,NPI);
}

inline void Bitmap::SetPixelIndex8(int X, int Y, int NPI)
{
	Buffer[Offset(X,Y)]=NPI;
}

inline void Bitmap::SetPixelIndex4(int X, int Y, int NPI)
{
	int Index=Offset(X,Y);
	
	if(X%2)	Buffer[Index]=(Buffer[Index] & 0xF0) | NPI;
	else	Buffer[Index]=(Buffer[Index] & 0x0F) | (NPI<<4);
}

inline void Bitmap::SetPixelIndex1(int X, int Y, int NPI)
{
	int Index=Offset(X,Y);

	int Sh=8-(X+1)%8;
	if(Sh==8) Sh=0;
	Buffer[Index]=(Buffer[Index] & (0xFF ^ 1<<Sh)) | (NPI<<Sh);
}


/************************************************************************
*																		*
*	inline int Bitmap::GetPixelIntensity(int X,int Y)					*
*																		*
*	Returns the pixel color's intensity for the given (X,Y),			*
*	(X & Y are visual top to bottom (0,0)=Top Left Corner).				*
*																		*							
************************************************************************/

inline int Bitmap::GetPixelIntensity(int X, int Y)
{
	return(Intensity(GetPixelColor(X,Y)));
}


/************************************************************************
*																		*
*	inline COLORREF AverageColors(COLORREF, COLORREF, float)			*	
*																		*
*	Creates a new color based on two original colors and a pan.			*
*	The color channels from both colors are combined via the			*
*	pan (a float from 0-1) determining the presence of each color.		*
*																		*
*	0.0=1st color completely dominant									*
*	0.5=Even average of both colors										*
*	1.0=2nd color completely cominant									*
*																		*
*	Precision is based on the multiplier PPM (always a power of 10)		*
*																		*
*	Returns:															*
*		The COLORREF of the averaged color.								*
*																		*
************************************************************************/

inline COLORREF AverageColors(COLORREF Color1, COLORREF Color2, float Pan)
{
	int R,G,B, PPM=10000, C2=RoundToInt(Pan*PPM), C1=PPM-C2;

	R=(CRR(Color1)*C1+CRR(Color2)*C2)/PPM;
	G=(CRG(Color1)*C1+CRG(Color2)*C2)/PPM;
	B=(CRB(Color1)*C1+CRB(Color2)*C2)/PPM;
	
	return(RGB(R,G,B));
}


//Binary Search/Insertion Sort for COLORREF Array
inline void Insert(COLORREF NewColor, long &NumCols, COLORREF *ColorList)
{
	long CurPos=NumCols>>1, High=NumCols-1, Low=0;

	while(High>=Low)
	{
		if(NewColor==ColorList[CurPos]) return;

		if(High==Low)
		{
			if(NewColor>ColorList[CurPos]) CurPos++;
			break;}

		if(NewColor<ColorList[CurPos]) High=CurPos;
		if(NewColor>=ColorList[CurPos]) Low=CurPos+1;
		CurPos=(High+Low)>>1;}

	if(Low>High) CurPos=Low;
	if(CurPos<NumCols) 
		memmove(&ColorList[CurPos+1],&ColorList[CurPos],(NumCols-CurPos)<<2);

	ColorList[CurPos]=NewColor;
	NumCols++;
}


// Write Bitmap's Header data to a text file
inline void Bitmap::DumpHeaderData(const char *DataFN)
{
	FILE *DataFile=fopen(DataFN, "w");
	fprintf(DataFile, "BITMAPFILEHEADER\n\nbfType: %i\nbfSize: %i\nbfReserved1: %i\nbfReserved1: %i\nbfOffBits: %i\n\n",		
			BMPHeader.bfType, BMPHeader.bfSize, BMPHeader.bfReserved1, 
			BMPHeader.bfReserved2, BMPHeader.bfOffBits);

	fprintf(DataFile, "BITMAPINFOHEADER\n\nbiSize: %i\nbiWidth: %i\nbiHeight: %i\nbiPlanes: %i\nbiBitCount: %i\nbiCompression: %i\nbiSizeImage: %i\nbiXPelsPerMeter: %i\nbiYPelsPerMeter: %i\nbiColorUsed: %i\nbiColorImportant: %i",
			BMPInfoHeader.biSize, BMPInfoHeader.biWidth,
			BMPInfoHeader.biHeight, BMPInfoHeader.biPlanes,
			BMPInfoHeader.biBitCount, BMPInfoHeader.biCompression,
			BMPInfoHeader.biSizeImage, BMPInfoHeader.biXPelsPerMeter,
			BMPInfoHeader.biYPelsPerMeter, BMPInfoHeader.biClrUsed,
			BMPInfoHeader.biClrImportant);

	fclose(DataFile);
}

#endif //BITMAP_H 
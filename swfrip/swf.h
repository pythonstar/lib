#ifndef SWF_H
#define SWF_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "bitstream.h"
#include "bitmap.h"
#include "structs.h"
#include "actions.h"

#define BaseName(S) strrchr(S,'\\')+1

/*ZLIB functions*/
/*
typedef int (__stdcall *compress_type)
(
	unsigned char *dest,
	unsigned long *destLen,
	const unsigned char *source,
	unsigned long sourceLen
);

typedef int (__stdcall *uncompress_type)
(	
	unsigned char *dest, 
	unsigned long *destLen, 
	const unsigned char *source, 
	unsigned long sourceLen
);
*/
#define MAX_TAGS			175000
#define MAX_ACTIONS			100000
#define MAX_FILEPATH		4096

#define SWF_SHAPE		0x00
#define SWF_IMAGE		0x01
#define SWF_MORPH		0x02
#define SWF_FONT		0x03
#define SWF_TEXT		0x04
#define SWF_SOUND		0x05
#define SWF_BUTTON		0x06
#define SWF_SPRITE		0x07
#define SWF_VIDEO		0x08
#define SWF_FRAME		0x09
#define SWF_SCRIPT		0x0A
#define SWF_SPRITEFRAME	0x10
#define SWF_ALL			0xFF


/*Type definitions*/

#define UI8		unsigned char
#define UI16	unsigned short
#define UI32	unsigned long
#define SI8		signed char
#define SI16	signed short
#define SI32	signed long
#define FP16	unsigned short
#define FP32	unsigned long


/*Tag Codes*/

#define TAGCODE_END					0
#define TAGCODE_SHOWFRAME			1
#define TAGCODE_DEFINESHAPE			2
#define TAGCODE_PLACEOBJECT			4
#define TAGCODE_REMOVEOBJECT		5
#define TAGCODE_DEFINEBITS			6
#define TAGCODE_DEFINEBUTTON		7
#define TAGCODE_JPEGTABLES			8
#define TAGCODE_SETBACKGROUNDCOLOR	9
#define TAGCODE_DEFINEFONT			10
#define TAGCODE_DEFINETEXT			11
#define TAGCODE_DOACTION			12
#define TAGCODE_DEFINEFONTINFO		13
#define TAGCODE_DEFINESOUND			14
#define TAGCODE_STARTSOUND			15
#define TAGCODE_DEFINEBUTTONSOUND	17
#define TAGCODE_SOUNDSTREAMHEAD		18
#define TAGCODE_SOUNDSTREAMBLOCK	19
#define TAGCODE_DEFINEBITSLOSSLESS	20
#define TAGCODE_DEFINEBITSJPEG2		21
#define TAGCODE_DEFINESHAPE2		22
#define TAGCODE_DEFINEBUTTONCXFORM	23
#define TAGCODE_PROTECT				24
#define TAGCODE_PLACEOBJECT2		26
#define TAGCODE_REMOVEOBJECT2		28
#define TAGCODE_DEFINESHAPE3		32
#define TAGCODE_DEFINETEXT2			33
#define TAGCODE_DEFINEBUTTON2		34
#define TAGCODE_DEFINEBITSJPEG3		35
#define TAGCODE_DEFINEBITSLOSSLESS2	36
#define TAGCODE_DEFINEEDITTEXT		37
#define TAGCODE_DEFINESPRITE		39
#define TAGCODE_FRAMELABEL			43
#define TAGCODE_SOUNDSTREAMHEAD2	45
#define TAGCODE_DEFINEMORPHSHAPE	46
#define TAGCODE_DEFINEFONT2			48
#define TAGCODE_EXPORTASSETS		56
#define TAGCODE_IMPORTASSETS		57
#define TAGCODE_ENABLEDEBUGGER		58
#define TAGCODE_DOINITACTION		59
#define TAGCODE_DEFINEVIDEOSTREAM	60
#define TAGCODE_VIDEOFRAME			61
#define TAGCODE_DEFINEFONTINFO2		62
#define TAGCODE_ENABLEDEBUGGER2		64
#define TAGCODE_SCRIPTLIMITS		65
#define TAGCODE_SETTABINDEX			66


const char TagNames[][80]=
{
	"END",			
	"SHOWFRAME",	
	"DEFINESHAPE",
	"3",
	"PLACEOBJECT",
	"REMOVEOBJECT",
	"DEFINEBITS",
	"DEFINEBUTTON",
	"JPEGTABLES",
	"SETBACKGROUNDCOLOR",
	"DEFINEFONT",
	"DEFINETEXT",
	"DOACTION",
	"DEFINEFONTINFO",
	"DEFINESOUND",
	"STARTSOUND",
	"16",
	"DEFINEBUTTONSOUND",
	"SOUNDSTREAMHEAD",
	"SOUNDSTREAMBLOCK",
	"DEFINEBITSLOSSLESS",
	"DEFINEBITJPEG2",
	"DEFINESHAPE2",
	"DEFINEBUTTONCXFORM",
	"PROTECT",
	"25",
	"PLACEOBJECT2",
	"27",
	"REMOVEOBJECT2",
	"29",
	"30",
	"31",
	"DEFINESHAPE3",
	"DEFINETEXT2",
	"DEFINEBUTTON2",
	"DEFINEBITJPEG3",
	"DEFINEBITSLOSSLESS2",
	"DEFINEEDITTEXT",
	"38",
	"DEFINESPRITE",
	"40",
	"41",
	"42",
	"FRAMELABEL",
	"44",
	"SOUNDSTREAMHEAD2",
	"DEFINEMORPHSHAPE",
	"47",
	"DEFINEFONT2",
	"49",
	"50",
	"51",
	"52",
	"53",
	"54",
	"55",
	"EXPORTASSETS",
	"IMPORTASSETS",
	"ENABLEDEBUGGER",
	"DOINITACTION",
	"DEFINEVIDEOSTREAM",
	"VIDEOFRAME",
	"DEFINEFONTINFO2",
	"63",
	"ENABLEDEBUGGER2",
	"SCRIPTLIMITS",
	"SETTABINDEX"
};
	

/*Fill Styles*/
 
#define FILLSTYLE_SOLID				0x00
#define FILLSTYLE_LINEARGRADIENT	0x10
#define FILLSTYLE_RADIALGRADIENT	0x12
#define FILLSTYLE_REPEATINGBITMAP	0x40
#define FILLSTYLE_CLIPPEDBITMAP		0x41
#define FILLSTYLE_REPEATINGBITMAPNS	0x42
#define FILLSTYLE_CLIPPEDBITMAPNS	0x43


#define BITMAP_COLORMAP	3
#define BITMAP_RGB15	4
#define BITMAP_RGB24	5


class CSWF
{
public:
	CSWF() 
	{
		Init();
		Clear(); 
	}

	CSWF(const char *szFileName) 
	{
		Load(szFileName);
	}

	void Init()
	{
		szSWFName=NULL;
		lprgflpLabels=NULL;
		UI16 usNumLabels=0;
		fDeleteTags=0;
		ubShapeFormat=0;
		ubImageFormat=0;
		ubTextFormat=0;
		ubFrameFormat=0;
		nTags=0;
		//fZLIB=LoadZLIB();
	}

	UI32 Load(const char*);
	UI8 Loaded() {return fLoaded;}
	void Clear();

	UI32 GetNumTags(UI16);
	UI16 GetNumFrames(UI16);	
	FRAMELABEL *GetFrameLabel(UI16,UI16);
	SI32 FindFrameStart(UI16,UI16);
	int  GetCharacterType(UI16);
	UI32 HasSoundStream(UI16);
	UI32 HasActions(UI16);
	UI32 Save(char*);
	UI32 SaveSSWF(char*);
	UI8 GetExportFormat(UI8);
	UI8 SetExportFormat(UI8,UI8);
	UI32 SaveCharacter(UI16);
	UI32 SaveText();
	UI32 SaveSound(UI16);
	UI32 SaveFrame(UI16,UI16);
	UI32 SaveFrameAsSVG(UI16,UI16);
	UI32 SaveScript(UI16);
	UI32 SaveAll(UI8);
	UI32 CreateFileName(char*,char*,UI32,char*);
	UI32 MakeDir(char*);
	void GetLastRes(char* szFileName) {strcpy(szFileName,szLastRes);}
	UI8 SetDeleteTags(UI8 fDelete) {return fDeleteTags=fDelete;}

	UI32 WriteDefineShapeAsSVG(DEFINESHAPE*,UI8,FILE*,PLACEOBJECT2 *lppo2Place=NULL);

	UI32 GetInfo(SWFINFO*);
	UI32 SetInfo(SWFINFO*);
	UI32 GetTagInfo(SI32,UI32,TAGINFO*);
	
	UI32 SetFileHeader(SWFFILEHEADER *lpfhFileHeader)
	{
		memcpy(&fhFileHeader,lpfhFileHeader,sizeof(SWFFILEHEADER));
		return 1;
	}

	UI32 SetFrameHeader(SWFFRAMEHEADER *lpfhFrameHeader)
	{
		memcpy(&fhFrameHeader,lpfhFrameHeader,sizeof(SWFFRAMEHEADER));
		return 1;
	}

	UI32 AddTag(void*);
/*
	void *NyNew (size_t stSize) 
	{
		void *lpvAlloc=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,stSize);

		FILE *pfileAlloc=fopen("D:\\alloc.txt","a");
		fprintf(pfileAlloc,"%p\t%lu\n",lpvAlloc,stSize);
		fclose(pfileAlloc); MessageBox(NULL,"blah","blah",MB_OK);

		return lpvAlloc;
	}
	
	void MyDelete (void *lpvData) 
	{
		if(!lpvData) return;

		HeapFree(GetProcessHeap(),NULL,lpvData); 
		lpvData=NULL;

		FILE *pfileAlloc=fopen("D:\\dealloc.txt","a");
		fprintf(pfileAlloc,"%p\n",lpvData);
		fclose(pfileAlloc);		
	}
*/
private:
	UI32 ReadUnknownTag(UNKNOWNTAG*);
	UI32 WriteUnknownTag(UNKNOWNTAG*,FILE*);
	UI16 IsCharacter(void*);
	UI32 GetTagArray(UI16,void **&,UI32&);
	UI32 ReadFrameHeader(SWFFRAMEHEADER*);
	UI32 WriteFrameHeader(SWFFRAMEHEADER*,FILE*);
	UI32 ReadTagHeader(RECORDHEADER*);
	UI32 ReadTags(void**,UI16*,FRAMELABEL**&,UI16*);
	UI32 WriteTags(void**,UI32, FILE*);
	UI32 DeleteTags(void**,UI32);
	UI32 WriteTagHeader(RECORDHEADER*,FILE*);
	UI32 WriteCharacter(UI16,FILE*);
	UI32 ExportCharacterDependancies(UNKNOWNTAG*,CSWF*);
	
	/*Basic Types*/
	UI32 ReadRect(SRECT*);
	UI32 WriteRect(SRECT*);
	UI32 ReadMatrix(MATRIX*);
	UI32 WriteMatrix(MATRIX*);
	UI32 ReadRGB(RGBA*,UI8);
	UI32 WriteRGB(RGBA*,UI8);
	UI32 ReadCXForm(CXFORM*,UI8);
	UI32 WriteCXForm(CXFORM*,UI8);
	
	/*Control Tags*/
	UI32 ReadSetBackgroundColor(SETBACKGROUNDCOLOR*);
	UI32 WriteSetBackgroundColor(SETBACKGROUNDCOLOR*,FILE*);
	UI32 ReadProtect(PROTECT*);
	UI32 WriteProtect(PROTECT*,FILE*);
	UI32 ReadFrameLabel(FRAMELABEL*);
	UI32 WriteFrameLabel(FRAMELABEL*,FILE*);
	UI32 WriteEnd(FILE*);
	UI32 ReadExportAssets(EXPORTASSETS*);
	UI32 WriteExportAssets(EXPORTASSETS*,FILE*);
	UI32 DeleteAssets(ASSET*,UI16);
	
	/*Display*/
	UI32 ReadPlaceObject(PLACEOBJECT*);
	UI32 WritePlaceObject(PLACEOBJECT*,FILE*);
	UI32 ReadPlaceObject2(PLACEOBJECT2*);
	UI32 WritePlaceObject2(PLACEOBJECT2*,FILE*);
	UI32 DeletePlaceObject2(PLACEOBJECT2*);
	UI32 ReadClipActions(CLIPACTIONS*);
	UI32 WriteClipActions(CLIPACTIONS*);
	UI32 ClipActionEndFlag(CLIPEVENTFLAGS*);
	UI32 ReadClipEventFlags(CLIPEVENTFLAGS*);
	UI32 WriteClipEventFlags(CLIPEVENTFLAGS*);
	UI32 ReadClipActionRecords(CLIPACTIONS*);
	UI32 WriteClipActionRecords(CLIPACTIONS*);
	UI32 ReadRemoveObject(REMOVEOBJECT*);
	UI32 WriteRemoveObject(REMOVEOBJECT*,FILE*);
	UI32 ReadRemoveObject2(REMOVEOBJECT2*);
	UI32 WriteRemoveObject2(REMOVEOBJECT2*,FILE*);
	UI32 WriteShowFrame(FILE*);

	/*Shapes*/
	UI32 ReadDefineShape(DEFINESHAPE*,UI8);
	UI32 WriteDefineShape(DEFINESHAPE*,UI8,FILE*);
	UI32 ReadShape(SHAPE*);
	UI32 WriteShape(SHAPE*lpsShape);
	UI32 DeleteShapeRecords(void**,UI32);
	UI32 ReadShapeWithStyle(SHAPEWITHSTYLE*,UI8);
	UI32 WriteShapeWithStyle(SHAPEWITHSTYLE*,UI8);
	UI32 ReadShapeRecords(SHAPEWITHSTYLE*,UI8);
	UI32 WriteShapeRecords(SHAPEWITHSTYLE*,UI8);
	UI32 ReadStraightEdgeRecord(STRAIGHTEDGERECORD*);
	UI32 WriteStraightEdgeRecord(STRAIGHTEDGERECORD*);
	UI32 ReadCurvedEdgeRecord(CURVEDEDGERECORD*);
	UI32 WriteCurvedEdgeRecord(CURVEDEDGERECORD*);
	UI32 ReadStyleChangeRecord(STYLECHANGERECORD*,UI8);
	UI32 WriteStyleChangeRecord(STYLECHANGERECORD*,UI8);
	UI32 ReadFillStyleArray(FILLSTYLEARRAY*,UI8);
	UI32 WriteFillStyleArray(FILLSTYLEARRAY*,UI8);
	UI32 ReadLineStyleArray(LINESTYLEARRAY*,UI8);
	UI32 WriteLineStyleArray(LINESTYLEARRAY*,UI8);
	UI32 ReadFillStyle(FILLSTYLE*,UI8);
	UI32 WriteFillStyle(FILLSTYLE*,UI8);
	UI32 ReadLineStyle(LINESTYLE*,UI8);
	UI32 WriteLineStyle(LINESTYLE*,UI8);
	UI32 ReadGradient(GRADIENT*,UI8);
	UI32 WriteGradient(GRADIENT*,UI8);
	UI32 ReadGradientRecord(GRADIENTRECORD*,UI8);
	UI32 WriteGradientRecord(GRADIENTRECORD*,UI8);
	UI32 SaveDefineShape(DEFINESHAPE*);
	UI32 ExportShapeDependancies(DEFINESHAPE*,CSWF*);
	UI32 SaveDefineShapeAsSVG(DEFINESHAPE*);

	/*morph shapes*/
	UI32 ReadDefineMorphShape(DEFINEMORPHSHAPE*);
	UI32 WriteDefineMorphShape(DEFINEMORPHSHAPE*,FILE*);
	UI32 DeleteDefineMorphShape(DEFINEMORPHSHAPE*);
	UI32 ReadMorphFillStyleArray(MORPHFILLSTYLEARRAY*);
	UI32 WriteMorphFillStyleArray(MORPHFILLSTYLEARRAY*);
	UI32 ReadMorphLineStyleArray(MORPHLINESTYLEARRAY*);
	UI32 WriteMorphLineStyleArray(MORPHLINESTYLEARRAY*);
	UI32 ReadMorphFillStyle(MORPHFILLSTYLE*);
	UI32 WriteMorphFillStyle(MORPHFILLSTYLE*);
	UI32 ReadMorphLineStyle(MORPHLINESTYLE*);
	UI32 WriteMorphLineStyle(MORPHLINESTYLE*);
	UI32 ReadMorphGradient(MORPHGRADIENT*);
	UI32 WriteMorphGradient(MORPHGRADIENT*);
	UI32 ReadMorphGradientRecord(MORPHGRADIENTRECORD*);
	UI32 WriteMorphGradientRecord(MORPHGRADIENTRECORD*);
	UI32 ExportMorphShapeDependancies(DEFINEMORPHSHAPE*,CSWF*);
	UI32 SaveDefineMorphShape(DEFINEMORPHSHAPE*);
	
	/*Images*/
	UI32 FixJPEGData(UI8*,UI32);
	UI32 ReadJPEGTables(JPEGTABLES*);
	UI32 WriteJPEGTables(JPEGTABLES*,FILE*);
	UI32 ReadDefineBits(DEFINEBITS*);
	UI32 WriteDefineBits(DEFINEBITS*,FILE*);
	UI32 SaveDefineBitsAsJPEG(DEFINEBITS*);
	UI32 ReadDefineBitsJPEG2(DEFINEBITSJPEG2*);
	UI32 WriteDefineBitsJPEG2(DEFINEBITSJPEG2*,FILE*);
	UI32 SaveDefineBitsJPEG2AsJPEG(DEFINEBITSJPEG2*);
	UI32 ReadDefineBitsJPEG3(DEFINEBITSJPEG3*);
	UI32 WriteDefineBitsJPEG3(DEFINEBITSJPEG3*,FILE*);
	UI32 SaveDefineBitsJPEG3AsJPEG(DEFINEBITSJPEG3*);
	UI32 ReadDefineBitsLossless(DEFINEBITSLOSSLESS*);
	UI32 WriteDefineBitsLossless(DEFINEBITSLOSSLESS*,FILE*);
	UI32 SaveDefineBitsLosslessAsFile(DEFINEBITSLOSSLESS*,char*);
	
	/*Sprites*/
	UI32 ReadDefineSprite(DEFINESPRITE*);
	UI32 WriteDefineSprite(DEFINESPRITE*,FILE*);
	UI32 SaveDefineSprite(DEFINESPRITE*,UI16);
	UI32 ExportSpriteDependancies(DEFINESPRITE*,CSWF*);

	/*Sounds*/
	UI32 ReadDefineSound(DEFINESOUND*);
	UI32 WriteDefineSound(DEFINESOUND*,FILE*);
	UI32 SaveDefineSoundAsFile(DEFINESOUND*);
	UI32 SaveSoundStream(DEFINESPRITE*);
	UI32 ReadSoundStreamBlock(SOUNDSTREAMBLOCK*);
	UI32 WriteSoundStreamBlock(SOUNDSTREAMBLOCK*,FILE*);
	UI32 ReadStartSound(STARTSOUND*);
	UI32 WriteStartSound(STARTSOUND*,FILE*);
	UI32 ReadSoundInfo(SOUNDINFO*);
	UI32 WriteSoundInfo(SOUNDINFO*,FILE*);
	UI32 ReadSoundEnvelope(SOUNDENVELOPE*);
	UI32 WriteSoundEnvelope(SOUNDENVELOPE*,FILE*);

	/*Fonts*/
	UI32 ReadDefineFont(DEFINEFONT*);
	UI32 WriteDefineFont(DEFINEFONT*,FILE*);
	UI32 DeleteDefineFont(DEFINEFONT*);
	UI32 ReadDefineFont2(DEFINEFONT2*);
	UI32 WriteDefineFont2(DEFINEFONT2*,FILE*);
	UI32 DeleteDefineFont2(DEFINEFONT2*);
	UI32 ReadDefineFontInfo(DEFINEFONTINFO*);
	UI32 WriteDefineFontInfo(DEFINEFONTINFO*,FILE*);
	UI32 SaveDefineFont(void*);

	/*Text*/
	UI32 ReadDefineText(DEFINETEXT*,UI8);
	UI32 WriteDefineText(DEFINETEXT*,UI8,FILE*);
	UI32 DeleteDefineText(DEFINETEXT*);
	UI32 ReadTextRecord(TEXTRECORD*,UI8);
	UI32 WriteTextRecord(TEXTRECORD*,UI8,FILE*);
	UI32 ReadGlyphEntry(GLYPHENTRY*);
	UI32 WriteGlyphEntry(GLYPHENTRY*,FILE*);
	UI32 ExportTextDependancies(DEFINETEXT*,CSWF*);
	UI32 SaveDefineTextAsSWF(DEFINETEXT*);
	UI32 WriteDefineTextAsText(DEFINETEXT*,UI8,FILE*);
	UI32 SaveDefineTextAsTXT(DEFINETEXT*);
	UI32 ReadDefineEditText(DEFINEEDITTEXT*);
	UI32 WriteDefineEditText(DEFINEEDITTEXT*,FILE*);
	
	/*Actions*/
	void GetActionName(UI8,char*);
	UI32 ReadDoAction(DOACTION*);
	UI32 WriteDoAction(DOACTION*,FILE*);
	UI32 ReadActionArray(ACTIONARRAY*);
	UI32 WriteActionArray(ACTIONARRAY*);
	UI32 DeleteActionArray(ACTIONARRAY*);
	UI32 ReadActionRecord(ACTIONRECORD*);
	UI32 WriteActionRecord(ACTIONRECORD*);
	UI32 DeleteActionRecord(ACTIONRECORD*);
	UI32 WriteActions(ACTIONARRAY*,UI8,FILE*);

	/*Buttons*/
	UI32 ReadDefineButton(DEFINEBUTTON*);
	UI32 WriteDefineButton(DEFINEBUTTON*,FILE*);
	UI32 DeleteDefineButton(DEFINEBUTTON*);
	UI32 ReadDefineButton2(DEFINEBUTTON2*);
	UI32 WriteDefineButton2(DEFINEBUTTON2*,FILE*);
	UI32 DeleteDefineButton2(DEFINEBUTTON2*);
	UI32 ReadButtonRecord(BUTTONRECORD*,UI8);
	UI32 WriteButtonRecord(BUTTONRECORD*,UI8);
	UI32 ReadButtonCondAction(BUTTONCONDACTION*);
	UI32 WriteButtonCondAction(BUTTONCONDACTION*);
	UI32 ReadDefineButtonSound(DEFINEBUTTONSOUND*);
	UI32 WriteDefineButtonSound(DEFINEBUTTONSOUND*,FILE*);
	UI32 ReadDefineButtonCXForm(DEFINEBUTTONCXFORM*);
	UI32 WriteDefineButtonCXForm(DEFINEBUTTONCXFORM*,FILE*);
	UI32 SaveDefineButton(DEFINEBUTTON*);
	UI32 ExportButtonDependancies(DEFINEBUTTON*,CSWF*);

	/*Video*/
	UI32 ReadDefineVideoStream(DEFINEVIDEOSTREAM*);
	UI32 WriteDefineVideoStream(DEFINEVIDEOSTREAM*,FILE*);
	UI32 ReadVideoFrame(VIDEOFRAME*);
	UI32 WriteVideoFrame(VIDEOFRAME*,FILE*);
	UI32 SaveDefineVideoStream(DEFINEVIDEOSTREAM*);

	/*SSWF*/
	UI32 WriteDefineShapeSSWF(DEFINESHAPE*,UI8,FILE*);

	//UI32 LoadZLIB();
	//compress_type compress;
	//uncompress_type uncompress;
	
	SWFFILEHEADER fhFileHeader;			//File header
	SWFFRAMEHEADER fhFrameHeader;		//Frame header
	RECORDHEADER rhTagHeader;			//Current tag header
	SETBACKGROUNDCOLOR *lpbcBackground; //Background color
	JPEGTABLES *lpjtJPEGTables;			//JpegTables
	
	char *szSWFName;
	char szLastRes[MAX_FILEPATH];
	FILE *pfileInfo, *pfileActions;
	void *lprgvpTags[MAX_TAGS];		//Array of tags
	UI32 nTags;
	void *lprgvpCharacters[0x10000];
	
	UI16 nFrames;
	FRAMELABEL **lprgflpLabels;
	UI16 usNumLabels;
	
	
	CBitstream bsInFileStream;		//Bitstream for reading
	CBitstream bsOutFileStream;		//Bitstream for writing

	UI8 nFillBits, nLineBits, nGlyphBits, nAdvanceBits;
	UI8 fLoaded, fUsePCX, fZLIB, fDeleteTags;

	UI8 ubShapeFormat, ubImageFormat, ubTextFormat, ubFrameFormat;
};

#endif
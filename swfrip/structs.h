#ifndef _SWF_STRUCTS_
#define _SWF_STRUCTS_

/*Basic Data Types*/

struct SRECT
{
	UI8	 bNbits;	//Number of bits in each rect value field
	SI32 slXMin;	//X minimum position for rect
	SI32 slXMax;	//X maxium position for rect
	SI32 slYMin;	//Y minimum position for rect
	SI32 slYMax;	//Y maxium position for rect
};

struct RGBA
{
	UI8 ubRed;
	UI8 ubGreen;
	UI8 ubBlue;
	UI8 ubAlpha;
};

struct MATRIX
{
	UI8  fHasScale;
	UI8	 ubNScaleBits;
	FP32 lfpScaleX;
	FP32 lfpScaleY;
	UI8  fHasRotate;
	UI8  ubNRotateBits;
	FP32 lfpRotateSkew0;
	FP32 lfpRotateSkew1;
	UI8  ubNTranslateBits;
	SI32 slTranslateX;
	SI32 slTranslateY;
};

struct CXFORM
{
	UI8  fHasAddTerms;
	UI8  fHasMultTerms;
	UI8  ubNBits;
	SFP16 sfpRedMultTerm;
	SFP16 sfpBlueMultTerm;
	SFP16 sfpGreenMultTerm;
	SFP16 sfpAlphaMultTerm;
	SI32  sbRedAddTerm;
	SI32  sbGreenAddTerm;
	SI32  sbBlueAddTerm;
	SI32  sbAlphaAddTerm;
};


/*Tags*/

struct RECORDHEADER
{
	UI16 usTagCodeAndLength; //Tag code and length
	UI16 usTagCode;			//Tag code
	UI32 ulLength;			//Tag Length
};

struct UNKNOWNTAG
{
	RECORDHEADER rhTagHeader;
	UI16 usCharacterID;			//if it's a character
	UI8 *prgbData;
};


/*Actions*/

struct ACTIONRECORD
{
	UI8  ubActionCode;
	UI16 usLength;			//if ubActionCode & 0x80
	UI8 *lpvActionData;
};

struct ACTIONARRAY
{
	ACTIONRECORD **lprgarpActions;
	UI32 ulNumActions;
	UI32 ulTotalLength;
};

struct DOACTION
{
	RECORDHEADER rhTagHeader;		//tag type = 12 (DOACTION), 59 (DOINITACTION)
	ACTIONARRAY aaActionArray;
	UI16 usSpriteID;				//Sprite to which these actions apply
};

/*Shape*/

struct GRADIENTRECORD
{
	UI8  ubRatio;
	RGBA rgbaColor;
};

struct GRADIENT
{
	UI8 ubNumGradients;
	GRADIENTRECORD *lprggrGradientRecords;
};

struct FILLSTYLE
{
	UI8		 ubFillStyleType;  
	RGBA	 rgbaColor;		
	MATRIX	 mGradientMatrix;
	GRADIENT gGradient;
	UI16	 usBitmapID;
	MATRIX	 mBitmapMatrix;
};

struct FILLSTYLEARRAY
{
	UI16	   usFillStyleCountExtended;
	FILLSTYLE *lprgfsFillStyles;
};

struct LINESTYLE
{
	UI16 usWidth;
	RGBA rgbaColor;
};

struct LINESTYLEARRAY
{
	UI16	  usLineStyleCountExtended;
	LINESTYLE *lprglsLineStyles;
};

struct STYLECHANGERECORD
{
	UI8  fTypeFlag;
	UI8  fStateNewStyles;
	UI8  fStateLineStyle;
	UI8  fStateFillStyle0;
	UI8  fStateFillStyle1;
	UI8  fStateMoveTo;
	UI8  ubMoveBits;			//5bits
	SI32 slMoveDeltaX;			//bits defined in parent SHAPE
	SI32 slMoveDeltaY;
	UI32 ulFillStyle0;
	UI32 ulFillStyle1;
	UI32 ulLineStyle;
	FILLSTYLEARRAY fsaFillStyleArray;
	LINESTYLEARRAY lsaLineStyleArray;
	UI8 ubNumFillBits;			//4bits
	UI8 ubNumLineBits;
};

struct STRAIGHTEDGERECORD
{
	UI8  fTypeFlag;
	UI8  fStraightFlag;
	UI8  ubNumBits;				//2 less than actual number (4bits)
	UI8  fGeneralLineFlag;
	SI32 slGeneralDeltaX;
	SI32 slGeneralDeltaY;
	UI8  fVertLineFlag;
	SI32 slVertDeltaX;
	SI32 slVertDeltaY;
};

struct CURVEDEDGERECORD
{
	UI8  fTypeFlag;
	UI8  fStraightFlag;
	UI8  ubNumBits;				//2 less than actual number (4bits)
	SI32 slControlDeltaX;
	SI32 slControlDeltaY;
	SI32 slAnchorDeltaX;
	SI32 slAnchorDeltaY;
};

struct SHAPE
{
	UI8 ubNumFillBits;			//Number of Fill Index Bits (4bits)
	UI8 ubNumLineBits;			//Number of Line Index Bits (4bits)
	void **lprgvpShapeRecords;
	UI32 ulNumShapeRecords;
};

struct SHAPEWITHSTYLE
{
	UI8 ubNumFillBits;			//Number of Fill Index Bits (4bits)
	UI8 ubNumLineBits;			//Number of Line Index Bits (4bits)
	void **lprgvpShapeRecords;
	UI32 ulNumShapeRecords;
	FILLSTYLEARRAY fsaFillStyleArray;
	LINESTYLEARRAY lsaLineStyleArray;
};

struct DEFINESHAPE
{
	RECORDHEADER rhTagHeader;
	UI16 usShapeID;
	SRECT rShapeBounds;
	SHAPEWITHSTYLE swsShapeWithStyle;
};

/*Morph Shapes*/

struct MORPHGRADIENTRECORD
{
	UI8  ubStartRatio;
	RGBA rgbaStartColor;
	UI8  ubEndRatio;
	RGBA rgbaEndColor;
};

struct MORPHGRADIENT
{
	UI8 ubNumGradients;
	MORPHGRADIENTRECORD *lprgmgrGradientRecords;
};

struct MORPHFILLSTYLE
{
	UI8	ubFillStyleType;  
	RGBA rgbaStartColor;			//If type = 0x00	
	RGBA rgbaEndColor;				//If type = 0x00
	MATRIX mStartGradientMatrix;	//If type = 0x10 or 0x12
	MATRIX mEndGradientMatrix;		//If type = 0x10 or 0x12
	MORPHGRADIENT mgGradient;		//If type = 0x10 or 0x12
	UI16 usBitmapID;				//If type = 0x40, 0x41, 0x42 or 0x43 
	MATRIX mStartBitmapMatrix;		//If type = 0x40, 0x41, 0x42 or 0x43
	MATRIX mEndBitmapMatrix;		//If type = 0x40, 0x41, 0x42 or 0x43
};

struct MORPHFILLSTYLEARRAY
{
	UI16 usFillStyleCountExtended;
	MORPHFILLSTYLE *lprgmfsFillStyles;
};

struct MORPHLINESTYLE
{
	UI16 usStartWidth;
	UI16 usEndWidth;
	RGBA rgbaStartColor;
	RGBA rgbaEndColor;
};

struct MORPHLINESTYLEARRAY
{
	UI16  usLineStyleCountExtended;
	MORPHLINESTYLE *lprgmlsLineStyles;
};

struct DEFINEMORPHSHAPE
{
	RECORDHEADER rhTagHeader;						//Tag type = 46
	UI16 usCharacterID;								//ID for this character
	SRECT rStartBounds;								//RECT Bounds of the start shape
	SRECT rEndBounds;								//Bounds of the end shape
	UI32 ulOffset;									//Indicates offset to EndEdges
	MORPHFILLSTYLEARRAY mfsaMorphFillStyleArray;	//Fill style information
	MORPHLINESTYLEARRAY mlsaMorphLineStyleArray;	//Line style information 
	SHAPE sStartEdges;								//start shape
	SHAPE sEndEdges;								//end shape
};

/*Images*/

struct DEFINEBITS
{
	RECORDHEADER rhTagHeader;	//Tag type = 6
	UI16 usCharacterID;			//ID for this character
	UI32 ulJPEGDataSize;
	UI8 *prgbJPEGData;			//JPEG compressed image
};

struct JPEGTABLES
{
	RECORDHEADER rhTagHeader;	//Tag type = 8
	UI32 ulJPEGDataSize;
	UI8 *prgbJPEGData;			//JPEG encoding table
};

struct DEFINEBITSJPEG2
{	
	RECORDHEADER rhTagHeader;	//Tag type = 21
	UI16 usCharacterID;			//ID for this character
	UI32 ulJPEGDataSize;
	UI8 *prgbJPEGData;			//UI8 JPEG encoding table and compressed image
};

struct DEFINEBITSJPEG3
{
	RECORDHEADER rhTagHeader;	//Tag type = 35.
	UI16 usCharacterID;			//ID for this character
	UI32 ulAlphaDataOffset;		//Count of bytes in JPEGData.
	UI8 *prgbJPEGData;			//JPEG encoding table and compressed image.
	UI32 ulAlphaDataSize;		//Total size after decompression must equal (width * height) of JPEG image.
	UI8 *prgbBitmapAlphaData;	//ZLIB compressed array of alpha data. One byte per pixel.
};

struct DEFINEBITSLOSSLESS
{
	RECORDHEADER rhTagHeader;		//Tag type = 20
	UI16 usCharacterID;				//ID for this character
	UI8 ubBitmapFormat;				//Format of compressed data 3=8-bit, 4=15-bit, 5=24-bit
	UI16 usBitmapWidth;				//Width of bitmap image
	UI16 usBitmapHeight;			//Height of bitmap image
	UI8 ubBitmapColorTableSize;		//If BitmapFormat=3, Number of palette entries - 1
	UI32 ulZlibDataSize;			//Length of compressed data
	UI8 *lprgbZlibBitmapData;		//If BitmapFormat=3, COLORMAPDATA. If BitmapFormat=4 | 5 BITMAPDATA
	UI32 ulDataSize;				//Uncompressed data size
	UI8 *lprgbBitmapData;			//Uncompressed data
};


/*Sounds*/

struct DEFINESOUND
{
	RECORDHEADER rhTagHeader;		//Tag type = 14
	UI16 usSoundID;					//Sound ID
	UI8  ubSoundFormat;				//UB[4] 0=RAW, 1=ADPCM, 2=MP3, 3=RAW (little-endian), 6=Nellymoser
	UI8  ubSoundRate;				//UB[2] 0=5.5kHz (not for MP3), 1=11kHz, 2=22kHz, 3=44kHz
	UI8  ubSoundSize;				//UB[1] 0=8bit (RAW only), 1=16bit
	UI8  ubSoundType;				//UB[1] 0=Mono, 1=Stereo (not for Nellymoser)
	UI32 ulSoundSampleCount;		//Number of samples, sample pairs for stereo
	UI8  *lprgbSoundData;			//Sound Data
	UI32 ulSoundDataSize;			//Sound Data Size
};

struct SOUNDSTREAMBLOCK
{
	RECORDHEADER rhTagHeader;		//Tag type = 19
	UI8  *lprgbSoundStreamData;		//Compressed sound data
	UI32 ulSoundStreamDataSize;		//Compressed sound data size
};

struct SOUNDENVELOPE
{
	UI32 ulPos44;		//Position of envelope point as anumber of 44kHz samples. 
	UI16 usLeftLevel;	//Volume level for left channel. 
	UI16 usRightLevel;	//Volume level for right channel.
};

struct SOUNDINFO
{	
	UI8  ubReserved;						//UB[2] Always 0
	UI8  ubSyncStop;						//UB[1] Stop the sound now
	UI8  ubSyncNoMultiple;					//UB[1] Dont start the sound if already playing
	UI8  ubHasEnvelope;						//UB[1] Has envelope information
	UI8  ubHasLoops;						//UB[1] Has loop information
	UI8  ubHasOutPoint;						//UB[1] Has out-point information
	UI8  ubHasInPoint;						//UB[1] Has in-point information
	UI32 ulInPoint;							//If HasInPoint UI32 Number of samples to skip at beginning
	UI32 ulOutPoint;						//If HasOutPoint UI32 Position in samples of last sample to play
	UI16 usLoopCount;						//If HasLoops UI16 Sound loop count
	UI8  ubEnvPoints;						//If HasEnvelope UI8 Sound Envelope point count
	SOUNDENVELOPE *lprgseEnvelopeRecords;	//If HasEnvelope Sound Envelope records
};

struct STARTSOUND
{
	RECORDHEADER rhTagHeader;		//Tag type = 15
	UI16 usSoundID;					//ID of sound character to play
	SOUNDINFO siSoundInfo;			//Sound style information
};

/*Fonts*/

struct DEFINEFONTINFO
{
	RECORDHEADER rhTagHeader;				// Tag type = 13
	UI16 usFontID;							//Font ID this information is for
	UI8 ubFontNameLen;						//UI8 Length of font name
	UI8 *lprgubFontName;					//UI8[FontNameLen] Name of the font (see below)
	UI8 ubFontFlagsReserved;				//UB[2] Reserved bit fields
	UI8 fFontFlagsSmallText;				//UB[1] SWF 7 or later: font is small
	UI8 fFontFlagsShiftJIS;					//UB[1] ShiftJIS character codes
	UI8 fFontFlagsANSI;						//UB[1] ANSI character codes
	UI8 fFontFlagsItalic;					//UB[1] Font is italic
	UI8 fFontFlagsBold;						//UB[1] Font is bold
	UI8 fFontFlagsWideCodes;				//UB[1] if 1 codeTable is UI16s else UI8s
	UI8 ubLanguageCode;						//if DEFINEFONTINFO2
	UI16 *lprgusCodeTable;					//If FontFlagsWideCodes UI16[nGlyphs] Otherwise UI8[nGlyphs]
	UI16 usNumGlyphs;
};

struct DEFINEFONT
{
	RECORDHEADER rhTagHeader;		//Tag type = 10
	UI16 usFontID;					//ID for this font character
	UI32 *lprgulOffsetTable;		//Array of shape offsets
	SHAPE *lprgsGlyphShapeTable;	//Array of shapes
	UI16 usNumGlyphs;
	DEFINEFONTINFO *lpdfiFontInfo;	//pointer to it's DEFINEFONTINFO
};

struct KERNINGRECORD
{
	UI16 usFontKerningCode1;		//If FontFlagsWideCodes UI16 Otherwise UI8
	UI16 usFontKerningCode2;		//If FontFlagsWideCodes UI16 Otherwise UI8
	SI16 usFontKerningAdjustment;	//SI16 Adjustment relative to left characterfs advance value
};

struct DEFINEFONT2
{
	RECORDHEADER rhTagHeader;				//Tag type = 48
	UI16 usFontID;							//ID for this font character
	UI8 fFontFlagsHasLayout;				//UB[1] Has font metrics/layout information
	UI8 fFontFlagsShiftJIS;					//UB[1] ShiftJIS encoding
	UI8 fFontFlagsSmallText;				//UB[1] Font is small
	UI8 fFontFlagsANSI;						//UB[1] ANSI encoding
	UI8 fFontFlagsWideOffsets;				//UB[1] If 1, uses 32 bit offsets
	UI8 fFontFlagsWideCodes;				//UB[1] If 1, font uses 16-bit codes, otherwise font uses 8 bit codes
	UI8 fFontFlagsItalic;					//UB[1] Italic Font
	UI8 fFontFlagsBold;						//UB[1] Bold Font
	UI8 ubLanguageCode;						//LANGCODE SWF 5 or earlier: always 0
	UI8 ubFontNameLen;						//UI8 Length of name
	UI8 *lprgubFontName;					//UI8[FontNameLen] Name of font (see DefineFontInfo)
	UI16 usNumGlyphs;						//Count of glyphs in font May be zero for device fonts
	UI32 *lprgulOffsetTable;				//If FontFlagsWideOffsets UI32[NumGlyphs] Otherwise UI16[NumGlyphs]
	UI32 ulCodeTableOffset;					//If FontFlagsWideOffsets UI32 Otherwise UI16
	SHAPE *lprgsGlyphShapeTable;			//Same as in DefineFont
	UI16 *lprgusCodeTable;					//If FontFlagsWideCodes UI16[NumGlyphs] Otherwise UI8[NumGlyphs]
	SI16 ssFontAscent;						//If FontFlagsHasLayout SI16 Font ascender height
	SI16 ssFontDescent;						//If FontFlagsHasLayout SI16 Font descender height
	SI16 ssFontLeading;						//If FontFlagsHasLayout SI16 Font leading height (see below)
	SI16 *lprgssFontAdvanceTable;			//If FontFlagsHasLayout SI16[NumGlyphs]
	SRECT *lprgrFontBoundsTable;			//If FontFlagsHasLayout RECT[NumGlyphs]
	UI16 usKerningCount;					//If FontFlagsHasLayout UI16 
	KERNINGRECORD *lprgkrFontKerningTable;	//If FontFlagsHasLayout [KerningCount]
};

/*Text*/

struct GLYPHENTRY
{
	UI32 ulGlyphIndex;		//UB[GlyphBits] Glyph index into current font
	SI32 slGlyphAdvance;	//SB[AdvanceBits] x advance value for glyph
};

struct TEXTRECORD
{
	UI8  ubTextRecordType;			//UB[1] Always 1
	UI8  ubStyleFlagsReserved;		//UB[3] Always 0
	UI8  fStyleFlagsHasFont;		//UB[1] 1 if text font specified
	UI8  fStyleFlagsHasColor;		//UB[1] 1 if text color specified
	UI8  fStyleFlagsHasYOffset;		//UB[1] 1 if y offset specified
	UI8  fStyleFlagsHasXOffset;		//UB[1] 1 if x offset specified
	UI16 usFontID;					//If StyleFlagsHasFont, Font ID for following text
	RGBA rgbaTextColor;				//If StyleFlagsHasColor RGB, If part of a DefineText2 tag then RGBA
	SI16 ssXOffset;					//If StyleFlagsHasXOffset, x offset for following text
	SI16 ssYOffset;					//If StyleFlagsHasYOffset, y offset for following text
	UI16 usTextHeight;				//If hasFont, Font height for following text
	UI8  ubGlyphCount;				//Number of glyphs in record
	GLYPHENTRY *lprggeGlyphEntries; //[GlyphCount] Glyph entry
};

struct DEFINETEXT
{
	RECORDHEADER rhTagHeader;			//Tag type = 11
	UI16 usCharacterID;					//ID for this text character
	SRECT rTextBounds;					//RECT Bounds of the text
	MATRIX mTextMatrix;					//MATRIX Transformation matrix for thetext
	UI8 ubGlyphBits;					//UI8 Bits in each glyph index
	UI8 ubAdvanceBits;					//UI8 Bits in each advance value
	TEXTRECORD **lprgtrpTextRecords;	//TEXTRECORD[zero or more] Text records
	UI32 ulNumTextRecords;
};

struct DEFINEEDITTEXT
{
	RECORDHEADER rhTagHeader;			//Tag type = 37
	UI16 usCharacterID;					//ID for this dynamic text character
	SRECT rBounds;						// Rectangle that completely encloses the text field
	UI8 fHasText;						//UB[1] 0=no default text, 1=initially displays InitialText
	UI8 fWordWrap;						//UB[1] 0=no wrap, 1=text will wrap
	UI8 fMultiline;						//UB[1] 0=one line only, 1=multi-line and will scroll automatically
	UI8 fPassword;						//UB[1] 0=characters displayed as typed, 1=characters displayed as an asterisk
	UI8 fReadOnly;						//UB[1] 0=text editing is enabled, 1=text editing is disabled
	UI8 fHasTextColor;					//UB[1] 0=use default color, 1=use specified color (TextColor)
	UI8 fHasMaxLength;					//UB[1] 0=length is unlimited, 1=maximum length of is MaxLength
	UI8 fHasFont;						//UB[1] 0=use default font, 1=use specified font (FontID) and height (FontHeight)
	UI8 fReserved1;						//UB[1] Always 0 
	UI8 fAutoSize;						//UB[1] 0=fixed size, 1=sizes to content (SWF 6 or later only)
	UI8 fHasLayout;						//UB[1] Layout information provided 
	UI8 fNoSelect;						//UB[1] Enables or disables interactive text selection
	UI8 fBorder;						//UB[1] Causes a border to be drawn around the text field
	UI8 fReserved2;						//UB[1] Always 0
	UI8 fHTML;							//UB[1] 0=plaintext content, 1=HTML content (see below)
	UI8 fUseOutlines;					//UB[1] 0=use device font, 1=use glyph font
	UI16 usFontID;						//If HasFont UI16 ID of font to use FontHeight If HasFont UI16 Height of font in twips
	RGBA rgbaTextColor;					//If HasTextColor RGBA Color of text
	UI16 usMaxLength;					//If HasMaxLength UI16 Text is restricted to this length
	UI8 ubAlign;						//If HasLayout UI8 0=Left, 1=Right, 2=Center, 3=Justify
	UI16 usLeftMargin;					//If HasLayout UI16 Left margin in twips
	UI16 usRightMargin;					//If HasLayout UI16 Right margin in twips
	UI16 usIndent;						//If HasLayout UI16 Indent in twips
	UI16 usLeading;						//If HasLayout UI16 Leading in twips
	char *szVariableName;				//STRING Name of the variable where the contents of the text field are stored.
	char *szInitialText;				//If HasText STRING Text that is initially displayed
};

/*Control tags*/

struct SETBACKGROUNDCOLOR
{
	RECORDHEADER rhTagHeader;
	RGBA rgbBackgroundColor;
};

struct PROTECT
{
	RECORDHEADER rhTagHeader;
	char *szPassword;
};

struct PLACEOBJECT
{
	RECORDHEADER rhTagHeader;
	UI16 usCharacterID;
	UI16 usDepth;
	MATRIX mMatrix;
	CXFORM cxfColorTransform; //optional
};

struct CLIPEVENTFLAGS
{
	UI8 fClipEventKeyUp;
	UI8 fClipEventKeyDown;
	UI8 fClipEventMouseUp;
	UI8 fClipEventMouseDown;
	UI8 fClipEventMouseMove;
	UI8 fClipEventUnload;
	UI8 fClipEventEnterFrame;
	UI8 fClipEventLoad;
	UI8 fClipEventDragOver;
	UI8 fClipEventRollOut;
	UI8 fClipEventRollOver;
	UI8 fClipEventReleaseOutside;
	UI8 fClipEventRelease;
	UI8 fClipEventPress;
	UI8 fClipEventInitialize;
	UI8 fClipEventData; 
	UI8 ubReserved1;			//UB[5] If SWF version >= 6
	UI8 fClipEventConstruct;	//UB[1] If SWF version >= 6
	UI8 fClipEventKeyPress;		//UB[1] If SWF version >= 6
	UI8 fClipEventDragOut;		//UB[1] If SWF version >= 6
	UI8 ubReserved2;			//UB[8] If SWF version >= 6
};

struct CLIPACTIONRECORD
{
	CLIPEVENTFLAGS cefEventFlags;
	UI32 ulActionRecordSize; 
	UI8 KeyCode;
	ACTIONARRAY aaActionArray;
};

struct CLIPACTIONS
{
	UI16 usReserved;
	CLIPEVENTFLAGS cefAllEventFlags;
	CLIPACTIONRECORD **lprgcarpClipActionRecords;
	UI32 ulNumClipActionRecords;
};

struct PLACEOBJECT2
{
	RECORDHEADER rhTagHeader;
	UI8 fPlaceFlagHasClipActions;
	UI8 fPlaceFlagHasClipDepth; 
	UI8 fPlaceFlagHasName;
	UI8 fPlaceFlagHasRatio;
	UI8 fPlaceFlagHasColorTransform;
	UI8 fPlaceFlagHasMatrix;
	UI8 fPlaceFlagHasCharacter;
	UI8 fPlaceFlagMove;
	UI16 usDepth;
	UI16 usCharacterID;
	MATRIX mMatrix;
	CXFORM cxfwaColorTransform;
	UI16 usRatio;
	char *szName;
	UI16 usClipDepth; 
	CLIPACTIONS caClipActions;
};

struct REMOVEOBJECT
{
	RECORDHEADER rhTagHeader;			//Tag type = 5
	UI16 usCharacterID;
	UI16 usDepth;
};

struct REMOVEOBJECT2
{
	RECORDHEADER rhTagHeader;			//Tag type = 28
	UI16 usDepth;
};

struct SHOWFRAME
{
	RECORDHEADER rhTagHeader;			//Tag type = 1
	UI16 usFrame;
};

struct FRAMELABEL
{
	RECORDHEADER rhTagHeader;			//Tag type = 43
	char *szLabel;						//STRING Label for frame
	UI16 usFrame;
};

struct ASSET
{
	UI16 usCharacterID;
	char *szName;
};

struct EXPORTASSETS
{
	RECORDHEADER rhTagHeader;			//Tag type = 56
	UI16 usCount;						//Number of assets to export
	ASSET *lprgaAssets;					//assets
};

/*Buttons*/

struct DEFINEBUTTONSOUND
{
	RECORDHEADER rhTagHeader;			//Tag type = 17
	UI16 usButtonId;					//The ID of the button these sounds apply to.
	UI16 usButtonSoundChar0;			//Sound ID for OverUpToIdle
	SOUNDINFO siButtonSoundInfo0;		//(if ButtonSoundChar0 is nonzero) Sound style for OverUpToIdle
	UI16 usButtonSoundChar1;			//Sound ID for IdleToOverUp
	SOUNDINFO siButtonSoundInfo1;		//(if ButtonSoundChar1 is nonzero) Sound style for IdleToOverUp
	UI16 usButtonSoundChar2;			//Sound ID for OverUpToOverDown
	SOUNDINFO siButtonSoundInfo2;		//(if ButtonSoundChar2 is nonzero) Sound style for OverUpToOverDown
	UI16 usButtonSoundChar3;			//Sound ID for OverDownToOverUp
	SOUNDINFO siButtonSoundInfo3;		//(if ButtonSoundChar3 is nonzero) Sound style for OverDownToOverUp
};

struct DEFINEBUTTONCXFORM
{
	RECORDHEADER rhTagHeader;			//Tag type = 23
	UI16 usButtonID;					//Button ID for this information
	CXFORM cxfwaButtonCXForm;			//Character color transform
};

struct BUTTONRECORD
{
	UI8  ubButtonReserved;					//UB[4] Reserved bits; always 0
	UI8  fButtonStateHitTest;				//UB[1] Present in hit test state
	UI8  fButtonStateDown;					//UB[1] Present in down state
	UI8  fButtonStateOver;					//UB[1] Present in over state
	UI8  fButtonStateUp;					//UB[1] Present in up state
	UI16 usCharacterID;						//UI16 ID of character to place
	UI16 usPlaceDepth;						//UI16 Depth at which to place character
	MATRIX mPlaceMatrix;					//MATRIX Transformation matrix for character placement
	CXFORM cxfwaColorTransform;				//if within DefineButton2, otherwise absent
};

struct DEFINEBUTTON
{
	RECORDHEADER rhTagHeader;			//Tag type = 7
	UI16 usButtonID;					//UI16 ID for this character
	BUTTONRECORD **lprgbrpCharacters;	//Characters that make up the button (end with UI8 0)
	UI32 ulNumCharacters;
	DEFINEBUTTONSOUND *lpdbsButtonSound;
	DEFINEBUTTONCXFORM *lpdbsButtonCXForm;
	ACTIONARRAY aaActionArray;
};

struct BUTTONCONDACTION
{
	UI16 usCondActionSize;				//UI16 Offset in bytes from start of this field to next BUTTONCONDACTION, or 0 if last action
	UI8  fCondIdleToOverDown;			//UB[1] Idle to OverDown
	UI8  fCondOutDownToIdle;			//UB[1] OutDown to Idle
	UI8  fCondOutDownToOverDown;		//UB[1] OutDown to OverDown
	UI8  fCondOverDownToOutDown;		//UB[1] OverDown to OutDown
	UI8  fCondOverDownToOverUp;			//UB[1] OverDown to OverUp
	UI8  fCondOverUpToOverDown;			//UB[1] OverUp to OverDown
	UI8  fCondOverUpToIdle;				//UB[1] OverUp to Idle
	UI8  fCondIdleToOverUp;				//UB[1] Idle to OverUp
	UI8  ubCondKeyPress;				//UB[7] SWF 4 or later: key code, Otherwise: always 0
	UI8  fCondOverDownToIdle;			//UB[1] OverDown to Idle
	ACTIONARRAY aaActionArray;
};

struct DEFINEBUTTON2
{
	RECORDHEADER rhTagHeader;			//Tag type = 34
	UI16 usButtonID;					//UI16 ID for this character
	BUTTONRECORD **lprgbrpCharacters;	//Characters that make up the button (end with UI8 0)
	UI32 ulNumCharacters;
	DEFINEBUTTONSOUND *lpdbsButtonSound;
	BUTTONCONDACTION **lprgbcapActions;	//Actions to execute at particular button events
	UI32 ulNumActions;
	UI8  ubReservedFlags;				//UB[7] Always 0
	UI8  fTrackAsMenu;					//UB[1] 0 = track as normal button 1 = track as menu button
	UI16 usActionOffset;				//Offset from start of this field to the first BUTTONCONDACTION, 0=no actions
};

/*Video*/

struct DEFINEVIDEOSTREAM
{
	RECORDHEADER rhTagHeader;			//Tag type = 60
	UI16 usCharacterID;					//ID for this video character
	UI16 usNumFrames;					//Number of VideoFrame tags that will make up this stream
	UI16 usWidth;						//Width in pixels
	UI16 usHeight;						//Height in pixels
	UI8  ubVideoFlagsReserved;			//UB[5] Reserved bitfields
	UI8  ubVideoFlagsDeblocking;		//UB[2] 00: use VIDEOPACKET value 01: off 10: on 11: reserved
	UI8  fVideoFlagsSmoothing;			//UB[1] 0: smoothing off (faster) 1: smoothing on (higher quality)
	UI8	 ubCodecID;						//UI8 2: Sorenson H.263 3: Screen video (SWF 7+ only)
};

struct VIDEOFRAME
{
	RECORDHEADER rhTagHeader;			//Tag type = 61
	UI16 usStreamID;					//ID of video stream character of which this frame is a part
	UI16 usFrameNum;					//Sequence number of this frame within its video stream (start at 0).
	UI8 *lprgbVideoData;				//if CodecID = 2 H263VIDEOPACKET, if CodecID = 3 SCREENVIDEOPACKET
	UI32 ulVideoDataSize;
};

/*Sprites*/

struct DEFINESPRITE
{
	RECORDHEADER rhTagHeader;		//Tag type = 39
	UI16 usSpriteID;				//Character ID of sprite
	UI16 usFrameCount;				//Number of frames in sprite
	void **lprgvpControlTags;		//A series of tags
	UI32 ulNumControlTags;
	FRAMELABEL **lprgflpLabels;
	UI16 usNumLabels;
};

/*Headers*/

struct SWFFILEHEADER
{
	UI8  bSignature1;	//F=Uncompressed, C=Compressed
	UI8  bSignature2;	//Always 'W'
	UI8  bSignature3;	//Always 'S'
	UI8  bVersion;		//File Version
	UI32 ulFileLength;	//Length of the File
};

struct SWFFRAMEHEADER
{
	SRECT srFrameSize;	//Frame width & height
	FP16  sfpFrameRate;	//Frames per second
	UI16  usFrameCount;	//Number of frames
};

/*info*/

/*Info Struct*/

struct SWFINFO
{
	UI8 bVersion;
	UI8 bCompressed;
	UI32 ulFileLength;
	RGBA rgbaBackground;
	SRECT srFrameRect;
	float spfFrameRate;
	UI32 usFrameCount;
	UI32 ulNumTags;
};

struct TAGINFO
{
	UI16 usTagCode;
	char szTagName[128];
	UI16 usCharacterID;
	UI16 usFrame;
	char szName[128];
	char szInfo[1024];
};

#endif
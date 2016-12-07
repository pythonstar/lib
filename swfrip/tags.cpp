#include "swf.h"

UI32 CSWF::ReadTags(void **lprgvpTagArray, UI16 *lpusNumFrames, 
					FRAMELABEL **&lprgflpLabelArray, UI16 *lpusNumLabels)
{
	char szIndent[4];
	UI32 nTagsRead=0, ulCurOffset, ulNextOffset, ulHeaderLength;
	FRAMELABEL *lprgflpLabelArrayTemp[0x10000];
	UI16 usFrameCount=0, usLabelCount=0;

	if(lprgvpTagArray!=lprgvpTags) strcpy(szIndent,"\t");
	else szIndent[0]='\0';

	/*info file*/
	fprintf(pfileInfo,"\n\n%s*** Frame %lu ***\n",szIndent,usFrameCount+1);

	ulNextOffset=bsInFileStream.GetByteOffset();

	do
	{
		/*check offset*/
		ulCurOffset=bsInFileStream.GetByteOffset();
		
		if(ulCurOffset!=ulNextOffset) 
		{
			bsInFileStream.Seek(ulNextOffset);
			ulCurOffset=bsInFileStream.GetByteOffset();
		}
			
		/*read tag header and set next offset*/
		ulHeaderLength=ReadTagHeader(&rhTagHeader);
		ulNextOffset+=ulHeaderLength+rhTagHeader.ulLength;

		/*info file*/
		fprintf(pfileInfo,"\n%s%08X: %s",
				szIndent,
				ulCurOffset+sizeof(SWFFILEHEADER),
				TagNames[rhTagHeader.usTagCode]);

		fflush(pfileInfo);

		switch(rhTagHeader.usTagCode)
		{
			case TAGCODE_SETBACKGROUNDCOLOR:
				lprgvpTagArray[nTagsRead]=new SETBACKGROUNDCOLOR;
				ReadSetBackgroundColor((SETBACKGROUNDCOLOR*)lprgvpTagArray[nTagsRead]);
				lpbcBackground=(SETBACKGROUNDCOLOR*)lprgvpTagArray[nTagsRead];
				break;
			
			case TAGCODE_DEFINESHAPE:
			case TAGCODE_DEFINESHAPE2:
			case TAGCODE_DEFINESHAPE3:
				lprgvpTagArray[nTagsRead]=new DEFINESHAPE;			
				ReadDefineShape((DEFINESHAPE*)lprgvpTagArray[nTagsRead],(rhTagHeader.usTagCode==32? 1:0));
				lprgvpCharacters[((DEFINESHAPE*)lprgvpTagArray[nTagsRead])->usShapeID]=lprgvpTagArray[nTagsRead];
				break;

			case TAGCODE_DEFINEMORPHSHAPE:
				lprgvpTagArray[nTagsRead]=new DEFINEMORPHSHAPE;			
				ReadDefineMorphShape((DEFINEMORPHSHAPE*)lprgvpTagArray[nTagsRead]);
				lprgvpCharacters[((DEFINEMORPHSHAPE*)lprgvpTagArray[nTagsRead])->usCharacterID]=lprgvpTagArray[nTagsRead];
				break;

			case TAGCODE_JPEGTABLES:
				lprgvpTagArray[nTagsRead]=new JPEGTABLES;
				ReadJPEGTables((JPEGTABLES*)lprgvpTagArray[nTagsRead]);
				lpjtJPEGTables=(JPEGTABLES*)lprgvpTagArray[nTagsRead];
				break;

			case TAGCODE_DEFINEBITS:
				lprgvpTagArray[nTagsRead]=new DEFINEBITS;
				ReadDefineBits((DEFINEBITS*)lprgvpTagArray[nTagsRead]);
				lprgvpCharacters[((DEFINEBITS*)lprgvpTagArray[nTagsRead])->usCharacterID]=lprgvpTagArray[nTagsRead];
				break;
			
			case TAGCODE_DEFINEBITSJPEG2:
				lprgvpTagArray[nTagsRead]=new DEFINEBITSJPEG2;
				ReadDefineBitsJPEG2((DEFINEBITSJPEG2*)lprgvpTagArray[nTagsRead]);
				lprgvpCharacters[((DEFINEBITSJPEG2*)lprgvpTagArray[nTagsRead])->usCharacterID]=lprgvpTagArray[nTagsRead];
				break;

			case TAGCODE_DEFINEBITSJPEG3:
				lprgvpTagArray[nTagsRead]=new DEFINEBITSJPEG3;
				ReadDefineBitsJPEG3((DEFINEBITSJPEG3*)lprgvpTagArray[nTagsRead]);
				lprgvpCharacters[((DEFINEBITSJPEG2*)lprgvpTagArray[nTagsRead])->usCharacterID]=lprgvpTagArray[nTagsRead];
				break;

			case TAGCODE_DEFINEBITSLOSSLESS:
			case TAGCODE_DEFINEBITSLOSSLESS2:
				lprgvpTagArray[nTagsRead]=new DEFINEBITSLOSSLESS;
				ReadDefineBitsLossless((DEFINEBITSLOSSLESS*)lprgvpTagArray[nTagsRead]);
				lprgvpCharacters[((DEFINEBITSLOSSLESS*)lprgvpTagArray[nTagsRead])->usCharacterID]=lprgvpTagArray[nTagsRead];
				break;

			case TAGCODE_DEFINEBUTTON:
				lprgvpTagArray[nTagsRead]=new DEFINEBUTTON;
				ReadDefineButton((DEFINEBUTTON*)lprgvpTagArray[nTagsRead]);
				lprgvpCharacters[((DEFINEBUTTON*)lprgvpTagArray[nTagsRead])->usButtonID]=lprgvpTagArray[nTagsRead];
				break;

			case TAGCODE_DEFINEBUTTON2:
				lprgvpTagArray[nTagsRead]=new DEFINEBUTTON2;
				ReadDefineButton2((DEFINEBUTTON2*)lprgvpTagArray[nTagsRead]);
				lprgvpCharacters[((DEFINEBUTTON2*)lprgvpTagArray[nTagsRead])->usButtonID]=lprgvpTagArray[nTagsRead];
				break;

			case TAGCODE_DEFINEBUTTONSOUND:
				lprgvpTagArray[nTagsRead]=new DEFINEBUTTONSOUND;
				ReadDefineButtonSound((DEFINEBUTTONSOUND*)lprgvpTagArray[nTagsRead]);
				break;

			case TAGCODE_DEFINEBUTTONCXFORM:
				lprgvpTagArray[nTagsRead]=new DEFINEBUTTONCXFORM;
				ReadDefineButtonCXForm((DEFINEBUTTONCXFORM*)lprgvpTagArray[nTagsRead]);
				break;

			case TAGCODE_DEFINEFONT:
				lprgvpTagArray[nTagsRead]=new DEFINEFONT;
				ReadDefineFont((DEFINEFONT*)lprgvpTagArray[nTagsRead]);
				lprgvpCharacters[((DEFINEFONT*)lprgvpTagArray[nTagsRead])->usFontID]=lprgvpTagArray[nTagsRead];
				break;

			case TAGCODE_DEFINEFONT2:
				lprgvpTagArray[nTagsRead]=new DEFINEFONT2;
				ReadDefineFont2((DEFINEFONT2*)lprgvpTagArray[nTagsRead]);
				lprgvpCharacters[((DEFINEFONT2*)lprgvpTagArray[nTagsRead])->usFontID]=lprgvpTagArray[nTagsRead];
				break;

			case TAGCODE_DEFINEFONTINFO:
			case TAGCODE_DEFINEFONTINFO2:
				lprgvpTagArray[nTagsRead]=new DEFINEFONTINFO;
				ReadDefineFontInfo((DEFINEFONTINFO*)lprgvpTagArray[nTagsRead]);
				break;
		
			case TAGCODE_DEFINESOUND:
				lprgvpTagArray[nTagsRead]=new DEFINESOUND;
				ReadDefineSound((DEFINESOUND*)lprgvpTagArray[nTagsRead]);
				lprgvpCharacters[((DEFINESOUND*)lprgvpTagArray[nTagsRead])->usSoundID]=lprgvpTagArray[nTagsRead];
				break;

			case TAGCODE_DEFINESPRITE:
				lprgvpTagArray[nTagsRead]=new DEFINESPRITE;
				ReadDefineSprite((DEFINESPRITE*)lprgvpTagArray[nTagsRead]);
				lprgvpCharacters[((DEFINESPRITE*)lprgvpTagArray[nTagsRead])->usSpriteID]=lprgvpTagArray[nTagsRead];
				rhTagHeader.usTagCode=0xFFFFF;
				break;

			case TAGCODE_DEFINEVIDEOSTREAM:
				lprgvpTagArray[nTagsRead]=new DEFINEVIDEOSTREAM;
				ReadDefineVideoStream((DEFINEVIDEOSTREAM*)lprgvpTagArray[nTagsRead]);
				lprgvpCharacters[((DEFINEVIDEOSTREAM*)lprgvpTagArray[nTagsRead])->usCharacterID]=lprgvpTagArray[nTagsRead];
				break;

			case TAGCODE_VIDEOFRAME:
				lprgvpTagArray[nTagsRead]=new VIDEOFRAME;
				ReadVideoFrame((VIDEOFRAME*)lprgvpTagArray[nTagsRead]);
				break;

			case TAGCODE_DEFINETEXT:
			case TAGCODE_DEFINETEXT2:
				lprgvpTagArray[nTagsRead]=new DEFINETEXT;
				ReadDefineText((DEFINETEXT*)lprgvpTagArray[nTagsRead],(rhTagHeader.usTagCode==33? 1:0));
				lprgvpCharacters[((DEFINETEXT*)lprgvpTagArray[nTagsRead])->usCharacterID]=lprgvpTagArray[nTagsRead];
				break;

			case TAGCODE_DEFINEEDITTEXT:
				lprgvpTagArray[nTagsRead]=new DEFINEEDITTEXT;
				ReadDefineEditText((DEFINEEDITTEXT*)lprgvpTagArray[nTagsRead]);
				lprgvpCharacters[((DEFINEEDITTEXT*)lprgvpTagArray[nTagsRead])->usCharacterID]=lprgvpTagArray[nTagsRead];
				break;

			case TAGCODE_DOACTION:
			case TAGCODE_DOINITACTION:
				lprgvpTagArray[nTagsRead]=new DOACTION;
				ReadDoAction((DOACTION*)lprgvpTagArray[nTagsRead]);
				break;

			case TAGCODE_EXPORTASSETS:
				lprgvpTagArray[nTagsRead]=new EXPORTASSETS;
				ReadExportAssets((EXPORTASSETS*)lprgvpTagArray[nTagsRead]);
				break;

			case TAGCODE_FRAMELABEL:
				lprgvpTagArray[nTagsRead]=new FRAMELABEL;
				ReadFrameLabel((FRAMELABEL*)lprgvpTagArray[nTagsRead]);
				((FRAMELABEL*)lprgvpTagArray[nTagsRead])->usFrame=usFrameCount+1;
				lprgflpLabelArrayTemp[usLabelCount]=(FRAMELABEL*)lprgvpTagArray[nTagsRead];
				usLabelCount++;
				break;

			case TAGCODE_PLACEOBJECT:
				lprgvpTagArray[nTagsRead]=new PLACEOBJECT;
				ReadPlaceObject((PLACEOBJECT*)lprgvpTagArray[nTagsRead]);
				break;

			case TAGCODE_PLACEOBJECT2:
				lprgvpTagArray[nTagsRead]=new PLACEOBJECT2;
				ReadPlaceObject2((PLACEOBJECT2*)lprgvpTagArray[nTagsRead]);
				break;

			case TAGCODE_PROTECT:
				lprgvpTagArray[nTagsRead]=new PROTECT;
				ReadProtect((PROTECT*)lprgvpTagArray[nTagsRead]);
				break;

			case TAGCODE_REMOVEOBJECT:
				lprgvpTagArray[nTagsRead]=new REMOVEOBJECT;
				ReadRemoveObject((REMOVEOBJECT*)lprgvpTagArray[nTagsRead]);
				break;

			case TAGCODE_REMOVEOBJECT2:
				lprgvpTagArray[nTagsRead]=new REMOVEOBJECT2;
				ReadRemoveObject2((REMOVEOBJECT2*)lprgvpTagArray[nTagsRead]);
				break;

			case TAGCODE_SOUNDSTREAMBLOCK:
				lprgvpTagArray[nTagsRead]=new SOUNDSTREAMBLOCK;
				ReadSoundStreamBlock((SOUNDSTREAMBLOCK*)lprgvpTagArray[nTagsRead]);
				break;

			case TAGCODE_STARTSOUND:
				lprgvpTagArray[nTagsRead]=new STARTSOUND;
				ReadStartSound((STARTSOUND*)lprgvpTagArray[nTagsRead]);
				break;

			case TAGCODE_SHOWFRAME:
				lprgvpTagArray[nTagsRead]=new SHOWFRAME;
				usFrameCount++;
				memcpy(&((SHOWFRAME*)lprgvpTagArray[nTagsRead])->rhTagHeader,
						&rhTagHeader,sizeof(RECORDHEADER));
				((SHOWFRAME*)lprgvpTagArray[nTagsRead])->usFrame=usFrameCount;
				fprintf(pfileInfo,"\n\n%s*** Frame %lu ***\n",szIndent,usFrameCount+1);
				break;

			case TAGCODE_END:
				lprgvpTagArray[nTagsRead]=new RECORDHEADER;
				memcpy(lprgvpTagArray[nTagsRead],&rhTagHeader,sizeof(RECORDHEADER));
				break;
			
			default:
				lprgvpTagArray[nTagsRead]=new UNKNOWNTAG;
				ReadUnknownTag((UNKNOWNTAG*)lprgvpTagArray[nTagsRead]);
						
				if(IsCharacter((void*)lprgvpTagArray[nTagsRead]))
					lprgvpCharacters[((UNKNOWNTAG*)lprgvpTagArray[nTagsRead])->usCharacterID]=lprgvpTagArray[nTagsRead];
		}
		
		nTagsRead++;		
	} 
	while(rhTagHeader.usTagCode);

	/*set return info*/
	*lpusNumFrames=usFrameCount;
	*lpusNumLabels=usLabelCount;
	if(usLabelCount)
	{
		lprgflpLabelArray=new FRAMELABEL*[usLabelCount];
		memcpy(lprgflpLabelArray,lprgflpLabelArrayTemp,4*usLabelCount);
	}

	/*info file*/
	fprintf(pfileInfo,"\n\n%s%lu tags read\n",szIndent,nTagsRead);

	return nTagsRead;
}

UI32 CSWF::WriteTags(void **lprgvpTagArray, UI32 ulNumTags, FILE *pfileSWF)
{
	UI32 iCurTag=0;
	for( iCurTag=0; iCurTag<ulNumTags; iCurTag++)
	{
		switch(((UNKNOWNTAG*)(lprgvpTagArray[iCurTag]))->rhTagHeader.usTagCode)
		{
			case TAGCODE_SETBACKGROUNDCOLOR:
				WriteSetBackgroundColor((SETBACKGROUNDCOLOR*)lprgvpTagArray[iCurTag],pfileSWF);
				break;
			
			case TAGCODE_DEFINESHAPE:
			case TAGCODE_DEFINESHAPE2:
				WriteDefineShape((DEFINESHAPE*)lprgvpTagArray[iCurTag],false,pfileSWF);
				break;

			case TAGCODE_DEFINESHAPE3:
				WriteDefineShape((DEFINESHAPE*)lprgvpTagArray[iCurTag],true,pfileSWF);
				break;

		case TAGCODE_DEFINEMORPHSHAPE:
				WriteDefineMorphShape((DEFINEMORPHSHAPE*)lprgvpTagArray[iCurTag],pfileSWF);
				break;

			case TAGCODE_JPEGTABLES:
				WriteJPEGTables((JPEGTABLES*)lprgvpTagArray[iCurTag],pfileSWF);
				break;

			case TAGCODE_DEFINEBITS:
				WriteDefineBits((DEFINEBITS*)lprgvpTagArray[iCurTag],pfileSWF);
				break;

			case TAGCODE_DEFINEBITSJPEG2:
				WriteDefineBitsJPEG2((DEFINEBITSJPEG2*)lprgvpTagArray[iCurTag],pfileSWF);
				break;

			case TAGCODE_DEFINEBITSJPEG3:
				WriteDefineBitsJPEG3((DEFINEBITSJPEG3*)lprgvpTagArray[iCurTag],pfileSWF);
				break;

			case TAGCODE_DEFINEBITSLOSSLESS:
			case TAGCODE_DEFINEBITSLOSSLESS2:
				WriteDefineBitsLossless((DEFINEBITSLOSSLESS*)lprgvpTagArray[iCurTag],pfileSWF);
				break;

			case TAGCODE_DEFINEBUTTON:
				WriteDefineButton((DEFINEBUTTON*)lprgvpTagArray[iCurTag],pfileSWF);
				break;

			case TAGCODE_DEFINEBUTTON2:
				WriteDefineButton2((DEFINEBUTTON2*)lprgvpTagArray[iCurTag],pfileSWF);
				break;
		
			case TAGCODE_DEFINEBUTTONSOUND:
				WriteDefineButtonSound((DEFINEBUTTONSOUND*)lprgvpTagArray[iCurTag],pfileSWF);
				break;

			case TAGCODE_DEFINEBUTTONCXFORM:
				WriteDefineButtonCXForm((DEFINEBUTTONCXFORM*)lprgvpTagArray[iCurTag],pfileSWF);
				break;

			case TAGCODE_DEFINEFONT:
				WriteDefineFont((DEFINEFONT*)lprgvpTagArray[iCurTag],pfileSWF);
				break;

			case TAGCODE_DEFINEFONT2:
				WriteDefineFont2((DEFINEFONT2*)lprgvpTagArray[iCurTag],pfileSWF);
				break;

			case TAGCODE_DEFINEFONTINFO:
			case TAGCODE_DEFINEFONTINFO2:
				WriteDefineFontInfo((DEFINEFONTINFO*)lprgvpTagArray[iCurTag],pfileSWF);
				break;

			case TAGCODE_DEFINESOUND:
				WriteDefineSound((DEFINESOUND*)lprgvpTagArray[iCurTag],pfileSWF);
				break;

			case TAGCODE_DEFINESPRITE:
				WriteDefineSprite((DEFINESPRITE*)lprgvpTagArray[iCurTag],pfileSWF);
				break;

			case TAGCODE_DEFINETEXT:
				WriteDefineText((DEFINETEXT*)lprgvpTagArray[iCurTag],false,pfileSWF);
				break;

			case TAGCODE_DEFINETEXT2:
				WriteDefineText((DEFINETEXT*)lprgvpTagArray[iCurTag],true,pfileSWF);
				break;

			case TAGCODE_DEFINEEDITTEXT:
				WriteDefineEditText((DEFINEEDITTEXT*)lprgvpTagArray[iCurTag],pfileSWF);
				break;

			case TAGCODE_DEFINEVIDEOSTREAM:
				WriteDefineVideoStream((DEFINEVIDEOSTREAM*)lprgvpTagArray[iCurTag],pfileSWF);
				break;

			case TAGCODE_VIDEOFRAME:
				WriteVideoFrame((VIDEOFRAME*)lprgvpTagArray[iCurTag],pfileSWF);
				break;

			case TAGCODE_DOACTION:
			case TAGCODE_DOINITACTION:
				WriteDoAction((DOACTION*)lprgvpTagArray[iCurTag],pfileSWF);
				break;

			case TAGCODE_EXPORTASSETS:
				WriteExportAssets((EXPORTASSETS*)lprgvpTagArray[iCurTag],pfileSWF);
				break;

			case TAGCODE_FRAMELABEL:
				WriteFrameLabel((FRAMELABEL*)lprgvpTagArray[iCurTag],pfileSWF);
				break;

			case TAGCODE_PLACEOBJECT:
				WritePlaceObject((PLACEOBJECT*)lprgvpTagArray[iCurTag],pfileSWF);
				break;

			case TAGCODE_PLACEOBJECT2:
				WritePlaceObject2((PLACEOBJECT2*)lprgvpTagArray[iCurTag],pfileSWF);
				break;

			case TAGCODE_REMOVEOBJECT:
				WriteRemoveObject((REMOVEOBJECT*)lprgvpTagArray[iCurTag],pfileSWF);
				break;

			case TAGCODE_REMOVEOBJECT2:
				WriteRemoveObject2((REMOVEOBJECT2*)lprgvpTagArray[iCurTag],pfileSWF);
				break;
				
			case TAGCODE_SOUNDSTREAMBLOCK:
				WriteSoundStreamBlock((SOUNDSTREAMBLOCK*)lprgvpTagArray[iCurTag],pfileSWF);
				break;

			case TAGCODE_STARTSOUND:
				WriteStartSound((STARTSOUND*)lprgvpTagArray[iCurTag],pfileSWF);
				break;

			case TAGCODE_SHOWFRAME:
			case TAGCODE_END:
				WriteTagHeader((RECORDHEADER*)lprgvpTagArray[iCurTag],pfileSWF);
				break;

			case TAGCODE_PROTECT:
				//WriteProtect((PROTECT*)lprgvpTagArray[iCurTag],pfileSWF);
				break;
			
			default:
				WriteUnknownTag((UNKNOWNTAG*)lprgvpTagArray[iCurTag],pfileSWF);
		}		
	}

	return iCurTag;
}

UI32 CSWF::DeleteTags(void **lprgvpTagArray, UI32 ulNumTags) 
{
	SHAPEWITHSTYLE *lpswsShapeWithStyle;
	SHAPE *lpsShape;
	void **lprgvpShapeRecords;
	UI32 ulNumShapeRecords;
	ASSET *lprgaAssets;
	UI32 usCount;

	/*Deallocate everything!*/
	for(UI32 iCurTag=0; iCurTag<ulNumTags; iCurTag++)
	{
		switch(((UNKNOWNTAG*)lprgvpTagArray[iCurTag])->rhTagHeader.usTagCode)
		{
			case TAGCODE_DEFINESHAPE:
			case TAGCODE_DEFINESHAPE2:
			case TAGCODE_DEFINESHAPE3:
				lpswsShapeWithStyle=&((DEFINESHAPE*)lprgvpTagArray[iCurTag])->swsShapeWithStyle;
				lprgvpShapeRecords=lpswsShapeWithStyle->lprgvpShapeRecords;
				ulNumShapeRecords=lpswsShapeWithStyle->ulNumShapeRecords;
				DeleteShapeRecords(lprgvpShapeRecords,ulNumShapeRecords);
				delete lpswsShapeWithStyle->fsaFillStyleArray.lprgfsFillStyles;
				delete lpswsShapeWithStyle->lsaLineStyleArray.lprglsLineStyles;
				break;

			case TAGCODE_DEFINEMORPHSHAPE:
				DeleteDefineMorphShape((DEFINEMORPHSHAPE*)lprgvpTagArray[iCurTag]);
				break;

			case TAGCODE_JPEGTABLES:
				delete ((JPEGTABLES*)lprgvpTagArray[iCurTag])->prgbJPEGData;
				break;

			case TAGCODE_DEFINEBITS:
			case TAGCODE_DEFINEBITSJPEG2:
				delete ((DEFINEBITSJPEG2*)lprgvpTagArray[iCurTag])->prgbJPEGData;
				break;

			case TAGCODE_DEFINEBITSJPEG3:
				delete ((DEFINEBITSJPEG3*)lprgvpTagArray[iCurTag])->prgbJPEGData;
				delete ((DEFINEBITSJPEG3*)lprgvpTagArray[iCurTag])->prgbBitmapAlphaData;
				break;

			case TAGCODE_DEFINEBITSLOSSLESS:
			case TAGCODE_DEFINEBITSLOSSLESS2:
				delete ((DEFINEBITSLOSSLESS*)lprgvpTagArray[iCurTag])->lprgbBitmapData;
				delete ((DEFINEBITSLOSSLESS*)lprgvpTagArray[iCurTag])->lprgbZlibBitmapData;
				break;

			case TAGCODE_DEFINEBUTTON:
				DeleteDefineButton(((DEFINEBUTTON*)lprgvpTagArray[iCurTag]));
				break;

			case TAGCODE_DEFINEBUTTON2:
				DeleteDefineButton2(((DEFINEBUTTON2*)lprgvpTagArray[iCurTag]));
				break;
		
			case TAGCODE_DEFINEFONT:
				DeleteDefineFont(((DEFINEFONT*)lprgvpTagArray[iCurTag]));
				break;

			case TAGCODE_DEFINEFONT2:
				DeleteDefineFont2(((DEFINEFONT2*)lprgvpTagArray[iCurTag]));
				break;

			case TAGCODE_DEFINEFONTINFO:
				delete ((DEFINEFONTINFO*)lprgvpTagArray[iCurTag])->lprgubFontName;
				delete ((DEFINEFONTINFO*)lprgvpTagArray[iCurTag])->lprgusCodeTable;
				break;

			case TAGCODE_DEFINESOUND:
				delete ((DEFINESOUND*)lprgvpTagArray[iCurTag])->lprgbSoundData;
				break;

			case TAGCODE_DEFINESPRITE:
				DeleteTags( ((DEFINESPRITE*)lprgvpTagArray[iCurTag])->lprgvpControlTags,
							((DEFINESPRITE*)lprgvpTagArray[iCurTag])->ulNumControlTags);
				if(((DEFINESPRITE*)lprgvpTagArray[iCurTag])->lprgflpLabels) 
					delete ((DEFINESPRITE*)lprgvpTagArray[iCurTag])->lprgflpLabels;
				break;

			case TAGCODE_DEFINETEXT:
			case TAGCODE_DEFINETEXT2:
				DeleteDefineText(((DEFINETEXT*)lprgvpTagArray[iCurTag]));
				break;

			case TAGCODE_DEFINEEDITTEXT:
				if(((DEFINEEDITTEXT*)lprgvpTagArray[iCurTag])->szVariableName)
					delete ((DEFINEEDITTEXT*)lprgvpTagArray[iCurTag])->szVariableName;
				if(((DEFINEEDITTEXT*)lprgvpTagArray[iCurTag])->szInitialText)
					delete ((DEFINEEDITTEXT*)lprgvpTagArray[iCurTag])->szInitialText;
				break;

			case TAGCODE_DOACTION:
			case TAGCODE_DOINITACTION:
				DeleteActionArray(&((DOACTION*)lprgvpTagArray[iCurTag])->aaActionArray);
				break;

			case TAGCODE_EXPORTASSETS:
				lprgaAssets=((EXPORTASSETS*)lprgvpTagArray[iCurTag])->lprgaAssets;
				usCount=((EXPORTASSETS*)lprgvpTagArray[iCurTag])->usCount;
				DeleteAssets(lprgaAssets,usCount);
				break;

			case TAGCODE_FRAMELABEL:
				delete ((FRAMELABEL*)lprgvpTagArray[iCurTag])->szLabel;
				break;

			case TAGCODE_PLACEOBJECT2:
				DeletePlaceObject2((PLACEOBJECT2*)lprgvpTagArray[iCurTag]);
				break;
				
			case TAGCODE_SOUNDSTREAMBLOCK:
				delete ((SOUNDSTREAMBLOCK*)lprgvpTagArray[iCurTag])->lprgbSoundStreamData;
				break;

			case TAGCODE_STARTSOUND:
				delete ((STARTSOUND*)lprgvpTagArray[iCurTag])->siSoundInfo.lprgseEnvelopeRecords;
				break;

			case TAGCODE_VIDEOFRAME:
				delete ((VIDEOFRAME*)lprgvpTagArray[iCurTag])->lprgbVideoData;
				break;

			case TAGCODE_DEFINEVIDEOSTREAM:
			case TAGCODE_SETBACKGROUNDCOLOR:
			case TAGCODE_PLACEOBJECT:
			case TAGCODE_REMOVEOBJECT:
			case TAGCODE_REMOVEOBJECT2:
			case TAGCODE_SHOWFRAME: 
			case TAGCODE_END:
			case TAGCODE_PROTECT:
				break;

			default: 
				if(((UNKNOWNTAG*)lprgvpTagArray[iCurTag])->prgbData) 
					 delete ((UNKNOWNTAG*)lprgvpTagArray[iCurTag])->prgbData;
		}

		delete lprgvpTagArray[iCurTag];	
	}

	return 0;
}

UI32 CSWF::GetNumTags(UI16 usClipID) 
{
	DEFINESPRITE *lpdsSprite;

	if(!usClipID) return nTags;
		lpdsSprite=(DEFINESPRITE*)lprgvpCharacters[usClipID];
	if(!lpdsSprite) return 0;
	if(lpdsSprite->rhTagHeader.usTagCode!=TAGCODE_DEFINESPRITE) return 0;
	return lpdsSprite->ulNumControlTags;
}

UI16 CSWF::GetNumFrames(UI16 usClipID)
{
	DEFINESPRITE *lpdsSprite;

	if(!usClipID) return nFrames;

	lpdsSprite=(DEFINESPRITE*)lprgvpCharacters[usClipID];
	if(!lpdsSprite) return 0;
	if(lpdsSprite->rhTagHeader.usTagCode!=TAGCODE_DEFINESPRITE) return 0;
	return lpdsSprite->usFrameCount;
}
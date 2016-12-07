#include "swf.h"
#include "displaylist.h"
#include "svg.h"

/*PLACEOBJECT(2)*/

UI32 CSWF::ReadPlaceObject(PLACEOBJECT *lppoPlace)
{
	UI32 ulTagLength=bsInFileStream.GetByteOffset();

	memset(lppoPlace,0,sizeof(PLACEOBJECT));
	memcpy(&lppoPlace->rhTagHeader,&rhTagHeader,sizeof(RECORDHEADER));

	lppoPlace->usCharacterID=bsInFileStream.ReadUI16();
	lppoPlace->usDepth=bsInFileStream.ReadUI16(); 
	ReadMatrix(&lppoPlace->mMatrix);
	
	/*Read CXFORM*/
	if(lppoPlace->rhTagHeader.ulLength>ulTagLength)
		ReadCXForm(&lppoPlace->cxfColorTransform,false);

	ulTagLength=bsInFileStream.GetByteOffset()-ulTagLength;

	return ulTagLength;
}

UI32 CSWF::WritePlaceObject(PLACEOBJECT *lppoPlace, FILE *pfileSWF)
{
	bsOutFileStream.Create(sizeof(PLACEOBJECT));

	bsOutFileStream.WriteUI16(lppoPlace->usCharacterID);
	bsOutFileStream.WriteUI16(lppoPlace->usDepth);
	WriteMatrix(&lppoPlace->mMatrix);

	/*Write CXFORM*/
	if(lppoPlace->cxfColorTransform.ubNBits)
		WriteCXForm(&lppoPlace->cxfColorTransform,false);

	lppoPlace->rhTagHeader.usTagCode=TAGCODE_PLACEOBJECT;
	lppoPlace->rhTagHeader.ulLength=bsOutFileStream.GetByteOffset();
	WriteTagHeader(&lppoPlace->rhTagHeader,pfileSWF);
	bsOutFileStream.WriteToFile(pfileSWF);

	return bsOutFileStream.GetByteOffset();
}

UI32 CSWF::ReadPlaceObject2(PLACEOBJECT2 *lppoPlace)
{
	UI32 ulTagLength=bsInFileStream.GetByteOffset();
	
	memset(lppoPlace,0,sizeof(PLACEOBJECT2));
	memcpy(&lppoPlace->rhTagHeader,&rhTagHeader,sizeof(RECORDHEADER));

	UI8 ulFlags=bsInFileStream.ReadUI8();

	lppoPlace->fPlaceFlagHasClipActions=ulFlags & 0x80;
	lppoPlace->fPlaceFlagHasClipDepth=ulFlags & 0x40;
	lppoPlace->fPlaceFlagHasName=ulFlags & 0x20;
	lppoPlace->fPlaceFlagHasRatio=ulFlags & 0x10;
	lppoPlace->fPlaceFlagHasColorTransform=ulFlags & 0x08;
	lppoPlace->fPlaceFlagHasMatrix=ulFlags & 0x04;
	lppoPlace->fPlaceFlagHasCharacter=ulFlags & 0x02;
	lppoPlace->fPlaceFlagMove=ulFlags & 0x01;
	
	lppoPlace->usDepth=bsInFileStream.ReadUI16();

	if(lppoPlace->fPlaceFlagHasCharacter)
		lppoPlace->usCharacterID=bsInFileStream.ReadUI16();

	if(lppoPlace->fPlaceFlagHasMatrix)
		ReadMatrix(&lppoPlace->mMatrix);

	if(lppoPlace->fPlaceFlagHasColorTransform)
		ReadCXForm(&lppoPlace->cxfwaColorTransform,true);

	if(lppoPlace->fPlaceFlagHasRatio)
		lppoPlace->usRatio=bsInFileStream.ReadUI16();

	if(lppoPlace->fPlaceFlagHasName)
		bsInFileStream.ReadString(lppoPlace->szName);
	
	if(lppoPlace->fPlaceFlagHasClipDepth)
		lppoPlace->usClipDepth=bsInFileStream.ReadUI16();

	if(lppoPlace->fPlaceFlagHasClipActions)
		ReadClipActions(&lppoPlace->caClipActions);

	ulTagLength=bsInFileStream.GetByteOffset()-ulTagLength;

	return ulTagLength;
}

UI32 CSWF::WritePlaceObject2(PLACEOBJECT2 *lppoPlace, FILE *pfileSWF)
{
	bsOutFileStream.Create(sizeof(PLACEOBJECT2)+lppoPlace->rhTagHeader.ulLength);
	
	UI8 ulFlags=0;

	if(lppoPlace->fPlaceFlagHasClipActions)		ulFlags|=0x80;
	if(lppoPlace->fPlaceFlagHasClipDepth)		ulFlags|=0x40;
	if(lppoPlace->fPlaceFlagHasName)			ulFlags|=0x20;
	if(lppoPlace->fPlaceFlagHasRatio)			ulFlags|=0x10;
	if(lppoPlace->fPlaceFlagHasColorTransform)	ulFlags|=0x08;
	if(lppoPlace->fPlaceFlagHasMatrix)			ulFlags|=0x04;
	if(lppoPlace->fPlaceFlagHasCharacter)		ulFlags|=0x02;
	if(lppoPlace->fPlaceFlagMove)				ulFlags|=0x01;
	
	bsOutFileStream.WriteUI8(ulFlags);
	bsOutFileStream.WriteUI16(lppoPlace->usDepth);

	if(lppoPlace->fPlaceFlagHasCharacter)
		bsOutFileStream.WriteUI16(lppoPlace->usCharacterID);
		
	if(lppoPlace->fPlaceFlagHasMatrix)
		WriteMatrix(&lppoPlace->mMatrix);

	if(lppoPlace->fPlaceFlagHasColorTransform)
		WriteCXForm(&lppoPlace->cxfwaColorTransform,true);

	if(lppoPlace->fPlaceFlagHasRatio)
		bsOutFileStream.WriteUI16(lppoPlace->usRatio);
	
	if(lppoPlace->fPlaceFlagHasName)
		bsOutFileStream.WriteString(lppoPlace->szName);
	
	if(lppoPlace->fPlaceFlagHasClipDepth)
		bsOutFileStream.WriteUI16(lppoPlace->usClipDepth);

	if(lppoPlace->fPlaceFlagHasClipActions)
		WriteClipActions(&lppoPlace->caClipActions);

	lppoPlace->rhTagHeader.usTagCode=TAGCODE_PLACEOBJECT2;
	lppoPlace->rhTagHeader.ulLength=bsOutFileStream.GetByteOffset();
	WriteTagHeader(&lppoPlace->rhTagHeader,pfileSWF);
	bsOutFileStream.WriteToFile(pfileSWF);

	return bsOutFileStream.GetByteOffset();
}

UI32 CSWF::DeletePlaceObject2(PLACEOBJECT2 *lppo2Place)
{
	delete lppo2Place->szName;

	CLIPACTIONRECORD **lprgcarpClipActions=lppo2Place->caClipActions.lprgcarpClipActionRecords;
	UI32 ulNumClipsActions=lppo2Place->caClipActions.ulNumClipActionRecords;

	for(UI32 iClipAction=0; iClipAction<ulNumClipsActions; iClipAction++)
		DeleteActionArray(&lprgcarpClipActions[iClipAction]->aaActionArray);
	
	return 0;
}

UI32 CSWF::ReadClipActions(CLIPACTIONS *lpcaClipActions)
{
	UI32 ulByteLength=bsInFileStream.GetByteOffset();

	memset(lpcaClipActions,0,sizeof(CLIPACTIONS));

	lpcaClipActions->usReserved=bsInFileStream.ReadUI16();
	ReadClipEventFlags(&lpcaClipActions->cefAllEventFlags);
	ReadClipActionRecords(lpcaClipActions);

	bsInFileStream.SeekNextByte();
	ulByteLength=bsInFileStream.GetByteOffset()-ulByteLength;
	
	return ulByteLength;
}

UI32 CSWF::WriteClipActions(CLIPACTIONS *lpcaClipActions)
{
	UI32 ulByteLength=bsOutFileStream.GetByteOffset();
	
	bsOutFileStream.WriteUI16(lpcaClipActions->usReserved);
	WriteClipEventFlags(&lpcaClipActions->cefAllEventFlags);
	WriteClipActionRecords(lpcaClipActions);

	bsOutFileStream.SeekNextByte();
	ulByteLength=bsOutFileStream.GetByteOffset()-ulByteLength;
	
	return ulByteLength;
}

UI32 CSWF::ReadClipEventFlags(CLIPEVENTFLAGS *lpcefClipEventFlags)
{
	UI32 ulByteLength=bsInFileStream.GetByteOffset();

	memset(lpcefClipEventFlags,0,sizeof(CLIPEVENTFLAGS));

	lpcefClipEventFlags->fClipEventKeyUp=(UI8)bsInFileStream.ReadUB(1);
	lpcefClipEventFlags->fClipEventKeyDown=(UI8)bsInFileStream.ReadUB(1);
	lpcefClipEventFlags->fClipEventMouseUp=(UI8)bsInFileStream.ReadUB(1);
	lpcefClipEventFlags->fClipEventMouseDown=(UI8)bsInFileStream.ReadUB(1);
	lpcefClipEventFlags->fClipEventMouseMove=(UI8)bsInFileStream.ReadUB(1);
	lpcefClipEventFlags->fClipEventUnload=(UI8)bsInFileStream.ReadUB(1);
	lpcefClipEventFlags->fClipEventEnterFrame=(UI8)bsInFileStream.ReadUB(1);
	lpcefClipEventFlags->fClipEventLoad=(UI8)bsInFileStream.ReadUB(1);
	lpcefClipEventFlags->fClipEventDragOver=(UI8)bsInFileStream.ReadUB(1);
	lpcefClipEventFlags->fClipEventRollOut=(UI8)bsInFileStream.ReadUB(1);
	lpcefClipEventFlags->fClipEventRollOver=(UI8)bsInFileStream.ReadUB(1);
	lpcefClipEventFlags->fClipEventReleaseOutside=(UI8)bsInFileStream.ReadUB(1);
	lpcefClipEventFlags->fClipEventRelease=(UI8)bsInFileStream.ReadUB(1);
	lpcefClipEventFlags->fClipEventPress=(UI8)bsInFileStream.ReadUB(1);
	lpcefClipEventFlags->fClipEventInitialize=(UI8)bsInFileStream.ReadUB(1);
	lpcefClipEventFlags->fClipEventData=(UI8)bsInFileStream.ReadUB(1);

	if(fhFileHeader.bVersion>=6)
	{
		lpcefClipEventFlags->ubReserved1=bsInFileStream.ReadUB(5);
		lpcefClipEventFlags->fClipEventConstruct=bsInFileStream.ReadUB(1);
		lpcefClipEventFlags->fClipEventKeyPress=bsInFileStream.ReadUB(1);
		lpcefClipEventFlags->fClipEventDragOut=bsInFileStream.ReadUB(1);
		lpcefClipEventFlags->ubReserved2=bsInFileStream.ReadUB(8);
	}

	bsInFileStream.SeekNextByte();
	ulByteLength=bsInFileStream.GetByteOffset()-ulByteLength;
	
	return ulByteLength;
}

UI32 CSWF::WriteClipEventFlags(CLIPEVENTFLAGS *lpcefClipEventFlags)
{
	UI32 ulByteLength=bsOutFileStream.GetByteOffset();

	bsOutFileStream.WriteUB(lpcefClipEventFlags->fClipEventKeyUp,1);
	bsOutFileStream.WriteUB(lpcefClipEventFlags->fClipEventKeyDown,1);
	bsOutFileStream.WriteUB(lpcefClipEventFlags->fClipEventMouseUp,1);
	bsOutFileStream.WriteUB(lpcefClipEventFlags->fClipEventMouseDown,1);
	bsOutFileStream.WriteUB(lpcefClipEventFlags->fClipEventMouseMove,1);
	bsOutFileStream.WriteUB(lpcefClipEventFlags->fClipEventUnload,1);
	bsOutFileStream.WriteUB(lpcefClipEventFlags->fClipEventEnterFrame,1);
	bsOutFileStream.WriteUB(lpcefClipEventFlags->fClipEventLoad,1);
	bsOutFileStream.WriteUB(lpcefClipEventFlags->fClipEventDragOver,1);
	bsOutFileStream.WriteUB(lpcefClipEventFlags->fClipEventRollOut,1);
	bsOutFileStream.WriteUB(lpcefClipEventFlags->fClipEventRollOver,1);
	bsOutFileStream.WriteUB(lpcefClipEventFlags->fClipEventReleaseOutside,1);
	bsOutFileStream.WriteUB(lpcefClipEventFlags->fClipEventRelease,1);
	bsOutFileStream.WriteUB(lpcefClipEventFlags->fClipEventPress,1);
	bsOutFileStream.WriteUB(lpcefClipEventFlags->fClipEventInitialize,1);
	bsOutFileStream.WriteUB(lpcefClipEventFlags->fClipEventData,1);

	if(fhFileHeader.bVersion>=6)
	{
		bsOutFileStream.WriteUB(lpcefClipEventFlags->ubReserved1,5);
		bsOutFileStream.WriteUB(lpcefClipEventFlags->fClipEventConstruct,1);
		bsOutFileStream.WriteUB(lpcefClipEventFlags->fClipEventKeyPress,1);
		bsOutFileStream.WriteUB(lpcefClipEventFlags->fClipEventDragOut,1);
		bsOutFileStream.WriteUB(lpcefClipEventFlags->ubReserved2,8);

	}

	bsOutFileStream.SeekNextByte();
	ulByteLength=bsOutFileStream.GetByteOffset()-ulByteLength;
	
	return ulByteLength;
}

UI32 CSWF::ClipActionEndFlag(CLIPEVENTFLAGS *lpcefClipEventFlags)
{
	if(lpcefClipEventFlags->fClipEventKeyUp)			return 0;
	if(lpcefClipEventFlags->fClipEventKeyDown)			return 0;
	if(lpcefClipEventFlags->fClipEventMouseUp)			return 0;
	if(lpcefClipEventFlags->fClipEventMouseDown)		return 0;
	if(lpcefClipEventFlags->fClipEventMouseMove)		return 0;
	if(lpcefClipEventFlags->fClipEventUnload)			return 0;
	if(lpcefClipEventFlags->fClipEventEnterFrame)		return 0;
	if(lpcefClipEventFlags->fClipEventLoad)				return 0;
	if(lpcefClipEventFlags->fClipEventDragOver)			return 0;
	if(lpcefClipEventFlags->fClipEventRollOut)			return 0;
	if(lpcefClipEventFlags->fClipEventRollOver)			return 0;
	if(lpcefClipEventFlags->fClipEventReleaseOutside)	return 0;
	if(lpcefClipEventFlags->fClipEventRelease)			return 0;
	if(lpcefClipEventFlags->fClipEventPress)			return 0;
	if(lpcefClipEventFlags->fClipEventInitialize)		return 0;
	if(lpcefClipEventFlags->fClipEventData)				return 0;
	if(lpcefClipEventFlags->ubReserved1)				return 0;
	if(lpcefClipEventFlags->fClipEventConstruct)		return 0;
	if(lpcefClipEventFlags->fClipEventKeyPress)			return 0;
	if(lpcefClipEventFlags->fClipEventDragOut)			return 0;
	if(lpcefClipEventFlags->ubReserved2)				return 0;

	return 1;
}

UI32 CSWF::ReadClipActionRecords(CLIPACTIONS *lpcaClipActions)
{
	UI32 ulBitLength=bsInFileStream.GetByteOffset();
	CLIPEVENTFLAGS cefClipEventFlags;
	CLIPACTIONRECORD *lpcarClipActionRecord;
	
	CLIPACTIONRECORD **lprgcarpClipActionRecords=new CLIPACTIONRECORD*[MAX_ACTIONS];
	
	while(true)
	{
		ReadClipEventFlags(&cefClipEventFlags);
		if(ClipActionEndFlag(&cefClipEventFlags)) break;

		lprgcarpClipActionRecords[lpcaClipActions->ulNumClipActionRecords]=new CLIPACTIONRECORD;
		lpcarClipActionRecord=lprgcarpClipActionRecords[lpcaClipActions->ulNumClipActionRecords];
		memset(lpcarClipActionRecord,0,sizeof(CLIPACTIONRECORD));
		memcpy(&lpcarClipActionRecord->cefEventFlags,&cefClipEventFlags,sizeof(CLIPEVENTFLAGS));
		lpcarClipActionRecord->ulActionRecordSize=bsInFileStream.ReadUI32();

		if(cefClipEventFlags.fClipEventKeyPress)
			lpcarClipActionRecord->KeyCode=bsInFileStream.ReadUI8();

		ReadActionArray(&lpcarClipActionRecord->aaActionArray);

		lpcaClipActions->ulNumClipActionRecords++;
	}
	
	lpcaClipActions->lprgcarpClipActionRecords=new CLIPACTIONRECORD*[lpcaClipActions->ulNumClipActionRecords];
	memcpy(lpcaClipActions->lprgcarpClipActionRecords,lprgcarpClipActionRecords,lpcaClipActions->ulNumClipActionRecords<<2);
	delete lprgcarpClipActionRecords;
	
	bsInFileStream.SeekNextByte();
	ulBitLength=bsInFileStream.GetByteOffset()-ulBitLength;

	return ulBitLength;
}

UI32 CSWF::WriteClipActionRecords(CLIPACTIONS *lpcaClipActions)
{
	UI32 ulByteLength=bsOutFileStream.GetByteOffset();
	CLIPACTIONRECORD *lpcarClipActionRecord;

	for(UI32 iClipAction=0; iClipAction<lpcaClipActions->ulNumClipActionRecords; iClipAction++)
	{
		lpcarClipActionRecord=lpcaClipActions->lprgcarpClipActionRecords[iClipAction];
		
		WriteClipEventFlags(&lpcarClipActionRecord->cefEventFlags);
		bsOutFileStream.WriteUI32(lpcarClipActionRecord->ulActionRecordSize); 
		
		UI32 ulOffset=bsOutFileStream.GetByteOffset();
			
		if(lpcarClipActionRecord->cefEventFlags.fClipEventKeyPress)
			bsOutFileStream.WriteUI8(lpcarClipActionRecord->KeyCode);

		WriteActionArray(&lpcarClipActionRecord->aaActionArray);

		lpcarClipActionRecord->ulActionRecordSize=bsOutFileStream.GetByteOffset()-ulOffset;

		bsOutFileStream.SeekBytes(-1*lpcarClipActionRecord->ulActionRecordSize-4);
		bsOutFileStream.WriteUI32(lpcarClipActionRecord->ulActionRecordSize);
		bsOutFileStream.SeekBytes(lpcarClipActionRecord->ulActionRecordSize);
	}

	/*ClipActionEndFlag (even without this the other programs can handle it)*/
	if(fhFileHeader.bVersion<6) bsOutFileStream.WriteUI16(0);
	else bsOutFileStream.WriteUI32(0);
	
	bsOutFileStream.SeekNextByte();
	ulByteLength=bsOutFileStream.GetByteOffset()-ulByteLength;
	
	return ulByteLength;
}

UI32 CSWF::ReadRemoveObject(REMOVEOBJECT *lproRemove)
{
	UI32 ulTagLength=bsInFileStream.GetByteOffset();

	memset(lproRemove,0,sizeof(REMOVEOBJECT));
	memcpy(&lproRemove->rhTagHeader,&rhTagHeader,sizeof(RECORDHEADER));

	lproRemove->usCharacterID=bsInFileStream.ReadUI16();
	lproRemove->usDepth=bsInFileStream.ReadUI16(); 
	
	ulTagLength=bsInFileStream.GetByteOffset()-ulTagLength;

	return ulTagLength;
}

UI32 CSWF::WriteRemoveObject(REMOVEOBJECT *lproRemove, FILE* pfileSWF)
{
	bsOutFileStream.Create(sizeof(REMOVEOBJECT));

	bsOutFileStream.WriteUI16(lproRemove->usCharacterID);
	bsOutFileStream.WriteUI16(lproRemove->usDepth); 

	lproRemove->rhTagHeader.usTagCode=TAGCODE_REMOVEOBJECT;
	lproRemove->rhTagHeader.ulLength=bsOutFileStream.GetByteOffset();
	WriteTagHeader(&lproRemove->rhTagHeader,pfileSWF);
	bsOutFileStream.WriteToFile(pfileSWF);

	return bsOutFileStream.GetByteOffset();

}

UI32 CSWF::ReadRemoveObject2(REMOVEOBJECT2 *lpro2Remove)
{
	UI32 ulTagLength=bsInFileStream.GetByteOffset();

	memset(lpro2Remove,0,sizeof(REMOVEOBJECT2));
	memcpy(&lpro2Remove->rhTagHeader,&rhTagHeader,sizeof(RECORDHEADER));

	lpro2Remove->usDepth=bsInFileStream.ReadUI16(); 
	
	ulTagLength=bsInFileStream.GetByteOffset()-ulTagLength;

	return ulTagLength;
}

UI32 CSWF::WriteRemoveObject2(REMOVEOBJECT2 *lpro2Remove, FILE* pfileSWF)
{
	bsOutFileStream.Create(sizeof(REMOVEOBJECT2));

	bsOutFileStream.WriteUI16(lpro2Remove->usDepth); 

	lpro2Remove->rhTagHeader.usTagCode=TAGCODE_REMOVEOBJECT2;
	lpro2Remove->rhTagHeader.ulLength=bsOutFileStream.GetByteOffset();
	WriteTagHeader(&lpro2Remove->rhTagHeader,pfileSWF);
	bsOutFileStream.WriteToFile(pfileSWF);

	return bsOutFileStream.GetByteOffset();

}

UI32 CSWF::WriteShowFrame(FILE* pfileSWF)
{
	RECORDHEADER rhShowFrameHeader;
	rhShowFrameHeader.usTagCode=TAGCODE_SHOWFRAME;
	rhShowFrameHeader.ulLength=0;
	return WriteTagHeader(&rhShowFrameHeader,pfileSWF); 
}

UI32 CSWF::ExportCharacterDependancies(UNKNOWNTAG *lputCharacter, CSWF *lpcswfCharacterFlash)
{
	UI16 usCharacterID=lputCharacter->usCharacterID;

	switch(lputCharacter->rhTagHeader.usTagCode)
	{
		case TAGCODE_DEFINESHAPE:
			ExportShapeDependancies((DEFINESHAPE*)lprgvpCharacters[usCharacterID],lpcswfCharacterFlash);
			break;

		case TAGCODE_DEFINESPRITE:
			ExportSpriteDependancies((DEFINESPRITE*)lprgvpCharacters[usCharacterID],lpcswfCharacterFlash);
			break;

		case TAGCODE_DEFINETEXT:
		case TAGCODE_DEFINETEXT2:
			ExportTextDependancies((DEFINETEXT*)lprgvpCharacters[usCharacterID],lpcswfCharacterFlash);
			break;

		case TAGCODE_DEFINEBUTTON:
		case TAGCODE_DEFINEBUTTON2:
			ExportButtonDependancies((DEFINEBUTTON*)lprgvpCharacters[usCharacterID],lpcswfCharacterFlash);
			break;
	}

	return 1;
}

UI32 CSWF::SaveFrame(UI16 usClipID, UI16 usFrame)
{
	UI16 usFrameCount=0;
	char szFrameFileName[MAX_FILEPATH];
	DEFINESPRITE *lpdsSprite;
	CSWF cswfFrameFlash;

	if(ubFrameFormat==1)
		return SaveFrameAsSVG(usClipID,usFrame);

	if(usClipID) 
	{
		lpdsSprite=(DEFINESPRITE*)lprgvpCharacters[usClipID];
		if(lpdsSprite->rhTagHeader.usTagCode!=TAGCODE_DEFINESPRITE) 
			return 0;
		return SaveDefineSprite(lpdsSprite,usFrame);
	}

	cswfFrameFlash.SetFileHeader(&fhFileHeader);
	cswfFrameFlash.SetFrameHeader(&fhFrameHeader);
	cswfFrameFlash.AddTag(lpbcBackground);

	for(UI32 iTag=0; usFrameCount<usFrame && iTag<nTags; iTag++)
	{
		UI16 usTagCode=((UNKNOWNTAG*)lprgvpTags[iTag])->rhTagHeader.usTagCode;
		
		if(usTagCode==TAGCODE_SOUNDSTREAMBLOCK) continue;
		if(usTagCode==TAGCODE_DEFINESOUND) continue;
		if(usTagCode==TAGCODE_STARTSOUND) continue;

		if(usTagCode==TAGCODE_SHOWFRAME)
		{
			usFrameCount++;
			continue;
		}
		
		cswfFrameFlash.AddTag(lprgvpTags[iTag]);	
	}

	/*END*/
	UNKNOWNTAG utShowFrame;
	memset(&utShowFrame,0,sizeof(UNKNOWNTAG));
	utShowFrame.rhTagHeader.usTagCode=TAGCODE_SHOWFRAME;
	cswfFrameFlash.AddTag(&utShowFrame);

	UNKNOWNTAG utEnd;
	memset(&utEnd,0,sizeof(UNKNOWNTAG));
	utEnd.rhTagHeader.usTagCode=TAGCODE_END;
	cswfFrameFlash.AddTag(&utEnd);

	CreateFileName(szFrameFileName,"frame",usFrame,"swf");
	cswfFrameFlash.Save(szFrameFileName);
	return 1;
}

SI32 CSWF::FindFrameStart(UI16 usClipID, UI16 usFrame)
{
	SI32 slLeft, slRight, slMiddle, slShowFrame;
	void **lprgvpTagArray;
	UI32 ulNumTags;
	UNKNOWNTAG* lputTag;
	SHOWFRAME *lpsfShowFrame;

	if(usFrame==0) return -1;
	if(usFrame==1) return 0;

	usFrame--;

	if(!GetTagArray(usClipID,lprgvpTagArray,ulNumTags))
		return -1;

	if(!lprgvpTagArray) return -1;

	slLeft=0;
	slRight=ulNumTags-1;

	while(slLeft<=slRight)
	{
		slMiddle=(slLeft+slRight)/2;

		slShowFrame=slMiddle;
		lputTag=(UNKNOWNTAG*)lprgvpTagArray[slShowFrame];
				
		while(lputTag->rhTagHeader.usTagCode!=TAGCODE_SHOWFRAME)
		{
			slShowFrame++;
			lputTag=(UNKNOWNTAG*)lprgvpTagArray[slShowFrame];
		}

		lpsfShowFrame=(SHOWFRAME*)lprgvpTagArray[slShowFrame];

		if(lpsfShowFrame->usFrame==usFrame)
			return slShowFrame+1;
		
		else if(lpsfShowFrame->usFrame<usFrame)
			slLeft=slMiddle+1;
		
		else slRight=slMiddle-1;
	}

	return -1;
}

UI32 CSWF::SaveFrameAsSVG(UI16 usClipID, UI16 usFrame)
{
	FILE *pfileSVG;
	char szSVGFileName[MAX_FILEPATH], szID[256];
	double sdXMin, sdXMax, sdYMin, sdYMax, sdWidth, sdHeight;
	void **lprgvpTagArray;
	UI32 ulNumTags, ulStart;
	UI16 usFrameCount=0;
	UNKNOWNTAG* lputTag;
	SHOWFRAME *lpsfShowFrame;
	DEFINESHAPE *lpdsShape;
	CDisplayList cdlDisplayList;
	LAYER lLayer;

	if(!GetTagArray(usClipID,lprgvpTagArray,ulNumTags))
		return -1;	

	/*dimension calculations*/
	sdXMin=double(fhFrameHeader.srFrameSize.slXMin)/20;
	sdXMax=double(fhFrameHeader.srFrameSize.slXMax)/20;
	sdYMin=double(fhFrameHeader.srFrameSize.slYMin)/20;
	sdYMax=double(fhFrameHeader.srFrameSize.slYMax)/20;
	sdWidth=sdXMax-sdXMin;
	sdHeight=sdYMax-sdYMin;

	/*open file*/
	CreateFileName(szSVGFileName,"frame",usFrame,"svg");
	pfileSVG=fopen(szSVGFileName,"w");
	
	/*info*/
	fprintf(pfileSVG,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	fprintf(pfileSVG,"<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\n");
	fprintf(pfileSVG,"\"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg11.dtd\">\n\n");
	fprintf(pfileSVG,"<!-- Generated by SWFRIP -->\n\n");
	
	/*svg*/
	fprintf(pfileSVG,"<svg width=\"%0.2f\" height=\"%0.2f\" viewBox=\"%0.2f %0.2f %0.2f %0.2f\">\n\n",
			sdWidth,sdHeight,sdXMin,sdYMin,sdWidth,sdHeight);

	/*frame*/
	//fprintf(pfileSVG,"<!-- Frame %i -->\n\n",usFrame);
	//fprintf(pfileSVG,"<g id=\"frame_%i\">\n\n",usFrame);

	/*generate display list*/
	for(UI32 iTag=0; usFrameCount<usFrame && iTag<ulNumTags; iTag++)
	{
		lputTag=(UNKNOWNTAG*)lprgvpTagArray[iTag];

		switch(lputTag->rhTagHeader.usTagCode)
		{
			case TAGCODE_PLACEOBJECT:
			case TAGCODE_PLACEOBJECT2:
				cdlDisplayList.Place(lputTag);
				break;

			case TAGCODE_REMOVEOBJECT:
			case TAGCODE_REMOVEOBJECT2:
				cdlDisplayList.Remove(lputTag);
				break;

			case TAGCODE_SHOWFRAME:
				usFrameCount++;
				break;
		}
	}

	/*make frame*/
	while(cdlDisplayList.GetNextLayer(&lLayer))
	{
		if(GetCharacterType(lLayer.po2Place.usCharacterID)!=SWF_SHAPE) continue;

		lpdsShape=(DEFINESHAPE*)lprgvpCharacters[lLayer.po2Place.usCharacterID];
		//WriteDefineShapeAsSVG(lpdsShape,false,pfileSVG);
		
		/*save character*/
		//SetExportFormat(SWF_SHAPE,1);
		//SaveCharacter(lLayer.po2Place.usCharacterID);
				
		/*write color transform filter*/
		if(lLayer.po2Place.fPlaceFlagHasColorTransform)
		{
			sprintf(szID,"color_transform_%p",lLayer.lplNext);
			WriteColorMatrixFilter(&lLayer.po2Place.cxfwaColorTransform,szID,pfileSVG);
			fprintf(pfileSVG,"<g filter=\"url(#%s)\">\n",szID);
		}

		/*write image*/	
		/*fprintf(pfileSVG,"<image x=\"%.2f\" y=\"%.2f\"",
				double(lpdsShape->rShapeBounds.slXMin)/20,
				double(lpdsShape->rShapeBounds.slYMin)/20);

		fprintf(pfileSVG," width=\"%.2f\" height=\"%.2f\"",
				double(lpdsShape->rShapeBounds.slXMax-lpdsShape->rShapeBounds.slXMin)/20,
				double(lpdsShape->rShapeBounds.slYMax-lpdsShape->rShapeBounds.slYMin)/20);
		*/

		//fprintf(pfileSVG,"<use xlink:href=\"#shape_%i\"",lpdsShape->usShapeID);

		/*inline
		if(lLayer.po2Place.fPlaceFlagHasColorTransform)
			fprintf(pfileSVG," filter=\"url(#color_transform_%p)\"",lLayer.lplNext);
		*/

		/*if(lLayer.po2Place.fPlaceFlagHasMatrix)
			WriteTransform(&lLayer.po2Place.mMatrix,pfileSVG);
		
		fprintf(pfileSVG," />\n");
		*/
		WriteDefineShapeAsSVG(lpdsShape,true,pfileSVG,&lLayer.po2Place);

		if(lLayer.po2Place.fPlaceFlagHasColorTransform)
			fprintf(pfileSVG,"</g>\n");
		
		fprintf(pfileSVG,"\n");

	}
	
	/*end file*/
	//fprintf(pfileSVG,"</g>\n\n");
	fprintf(pfileSVG,"</svg>");
	fclose(pfileSVG);

	/*so that szLastRes is this frame*/
	CreateFileName(szSVGFileName,"frame",usFrame,"svg");

	return 0;
}
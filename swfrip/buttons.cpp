#include "swf.h"

UI32 CSWF::ReadDefineButton(DEFINEBUTTON *lpdbDefineButton)
{
	UI32 ulTagLength=bsInFileStream.GetByteOffset();
	BUTTONRECORD *lpbrCharacter;
	BUTTONRECORD **lprgbrpCharacters=new BUTTONRECORD*[0x10000];
	
	memset(lpdbDefineButton,0,sizeof(DEFINEBUTTON));
	memcpy(&lpdbDefineButton->rhTagHeader,&rhTagHeader,sizeof(RECORDHEADER));
	
	lpdbDefineButton->usButtonID=bsInFileStream.ReadUI16();

	/*Read characters*/
	while(bsInFileStream.GetUI8())
	{
		lprgbrpCharacters[lpdbDefineButton->ulNumCharacters]=new BUTTONRECORD;
		lpbrCharacter=lprgbrpCharacters[lpdbDefineButton->ulNumCharacters];
		ReadButtonRecord(lpbrCharacter,false);
		lpdbDefineButton->ulNumCharacters++;
	}

	bsInFileStream.ReadUI8(); //skip end marker
	lpdbDefineButton->lprgbrpCharacters=new BUTTONRECORD*[lpdbDefineButton->ulNumCharacters];
	memcpy(lpdbDefineButton->lprgbrpCharacters,lprgbrpCharacters,lpdbDefineButton->ulNumCharacters<<2);
	delete lprgbrpCharacters;

	ReadActionArray(&lpdbDefineButton->aaActionArray);
	
	bsInFileStream.SeekNextByte();
	ulTagLength=bsInFileStream.GetByteOffset()-ulTagLength;

	return ulTagLength;
}

UI32 CSWF::WriteDefineButton(DEFINEBUTTON *lpdbDefineButton, FILE *pfileSWF)
{
	bsOutFileStream.Create(sizeof(DEFINEBUTTON)+lpdbDefineButton->rhTagHeader.ulLength);
	
	bsOutFileStream.WriteUI16(lpdbDefineButton->usButtonID);

	for(UI32 iCharacter=0; iCharacter<lpdbDefineButton->ulNumCharacters; iCharacter++)
		WriteButtonRecord(lpdbDefineButton->lprgbrpCharacters[iCharacter],false);
	
	bsOutFileStream.WriteUI8(0);

	WriteActionArray(&lpdbDefineButton->aaActionArray);

	lpdbDefineButton->rhTagHeader.ulLength=bsOutFileStream.GetByteOffset();
	WriteTagHeader(&lpdbDefineButton->rhTagHeader,pfileSWF);
	bsOutFileStream.WriteToFile(pfileSWF);

	return lpdbDefineButton->rhTagHeader.ulLength;
}

UI32 CSWF::DeleteDefineButton(DEFINEBUTTON *lpdbDefineButton)
{
	/*delete characters*/
	for(UI32 ulCharacter=0; ulCharacter<lpdbDefineButton->ulNumCharacters; ulCharacter++)
		delete lpdbDefineButton->lprgbrpCharacters[ulCharacter];

	delete lpdbDefineButton->lprgbrpCharacters;

	DeleteActionArray(&lpdbDefineButton->aaActionArray);

	return 0;
}

UI32 CSWF::ReadDefineButton2(DEFINEBUTTON2 *lpdbDefineButton)
{
	UI32 ulTagLength=bsInFileStream.GetByteOffset(), ulCondActionSize;
	BUTTONRECORD *lpbrCharacter;
	BUTTONRECORD **lprgbrpCharacters=new BUTTONRECORD*[0x10000];
	BUTTONCONDACTION *lpbcaButtonAction;
	BUTTONCONDACTION **lprgbcapActions=new BUTTONCONDACTION*[MAX_ACTIONS];

	memset(lpdbDefineButton,0,sizeof(DEFINEBUTTON2));
	memcpy(&lpdbDefineButton->rhTagHeader,&rhTagHeader,sizeof(RECORDHEADER));
	
	lpdbDefineButton->usButtonID=bsInFileStream.ReadUI16();
	lpdbDefineButton->ubReservedFlags=bsInFileStream.ReadUB(7);
	lpdbDefineButton->fTrackAsMenu=bsInFileStream.ReadUB(1);
	lpdbDefineButton->usActionOffset=bsInFileStream.ReadUI16(); lpdbDefineButton->lpdbsButtonSound;

	/*Read characters*/
	while(bsInFileStream.GetUI8())
	{
		lprgbrpCharacters[lpdbDefineButton->ulNumCharacters]=new BUTTONRECORD;
		lpbrCharacter=lprgbrpCharacters[lpdbDefineButton->ulNumCharacters];
		ReadButtonRecord(lpbrCharacter,true);
		lpdbDefineButton->ulNumCharacters++;
	}

	bsInFileStream.ReadUI8(); //skip end marker
	lpdbDefineButton->lprgbrpCharacters=new BUTTONRECORD*[lpdbDefineButton->ulNumCharacters];
	memcpy(lpdbDefineButton->lprgbrpCharacters,lprgbrpCharacters,lpdbDefineButton->ulNumCharacters<<2);
	delete lprgbrpCharacters;

	/*Read actions*/
	ulCondActionSize=lpdbDefineButton->usActionOffset;
	while(ulCondActionSize)
	{
		lprgbcapActions[lpdbDefineButton->ulNumActions]=new BUTTONCONDACTION;
		lpbcaButtonAction=lprgbcapActions[lpdbDefineButton->ulNumActions];
		ulCondActionSize=ReadButtonCondAction(lpbcaButtonAction);
		lpdbDefineButton->ulNumActions++;
	} 

	lpdbDefineButton->lprgbcapActions=new BUTTONCONDACTION*[lpdbDefineButton->ulNumActions];
	memcpy(lpdbDefineButton->lprgbcapActions,lprgbcapActions,lpdbDefineButton->ulNumActions<<2);
	delete lprgbcapActions;
	
	bsInFileStream.SeekNextByte();
	ulTagLength=bsInFileStream.GetByteOffset()-ulTagLength;

	return ulTagLength;
}

UI32 CSWF::WriteDefineButton2(DEFINEBUTTON2 *lpdbDefineButton, FILE *pfileSWF)
{
	bsOutFileStream.Create(sizeof(DEFINEBUTTON2)+lpdbDefineButton->rhTagHeader.ulLength);
	BUTTONRECORD *lpbrCharacter;
	BUTTONCONDACTION *lpbcaButtonAction;
	
	bsOutFileStream.WriteUI16(lpdbDefineButton->usButtonID);
	bsOutFileStream.WriteUB(lpdbDefineButton->ubReservedFlags,7);
	bsOutFileStream.WriteUB(lpdbDefineButton->fTrackAsMenu,1);
	bsOutFileStream.WriteUI16(lpdbDefineButton->usActionOffset);//this should be calc.

	for(UI32 iCharacter=0; iCharacter<lpdbDefineButton->ulNumCharacters; iCharacter++)
	{
		lpbrCharacter=lpdbDefineButton->lprgbrpCharacters[iCharacter];
		WriteButtonRecord(lpbrCharacter,true);
	}

	bsOutFileStream.WriteUI8(0);

	for(UI32 iAction=0; iAction<lpdbDefineButton->ulNumActions; iAction++)
	{
		lpbcaButtonAction=lpdbDefineButton->lprgbcapActions[iAction];
		WriteButtonCondAction(lpbcaButtonAction);
	}

	lpdbDefineButton->rhTagHeader.ulLength=bsOutFileStream.GetByteOffset();
	WriteTagHeader(&lpdbDefineButton->rhTagHeader,pfileSWF);
	bsOutFileStream.WriteToFile(pfileSWF);

	return bsOutFileStream.GetByteOffset();
}

UI32 CSWF::DeleteDefineButton2(DEFINEBUTTON2 *lpdbDefineButton)
{
	/*delete characters*/
	for(UI32 ulCharacter=0; ulCharacter<lpdbDefineButton->ulNumCharacters; ulCharacter++)
		delete lpdbDefineButton->lprgbrpCharacters[ulCharacter];

	delete lpdbDefineButton->lprgbrpCharacters;

	/*delete BUTTONCONDACTIONS*/
	for(UI32 ulBCAction=0; ulBCAction<lpdbDefineButton->ulNumActions; ulBCAction++)
	{
		DeleteActionArray(&lpdbDefineButton->lprgbcapActions[ulBCAction]->aaActionArray);

		delete lpdbDefineButton->lprgbcapActions[ulBCAction];
	}
	
	delete lpdbDefineButton->lprgbcapActions;

	return 0;
}

UI32 CSWF::ReadButtonRecord(BUTTONRECORD *lpbrButtonRecord, UI8 fCXForm)
{
	UI32 ulTagLength=bsInFileStream.GetByteOffset();

	lpbrButtonRecord->ubButtonReserved=(UI8)bsInFileStream.ReadUB(4);
	lpbrButtonRecord->fButtonStateHitTest=(UI8)bsInFileStream.ReadUB(1);
	lpbrButtonRecord->fButtonStateDown=(UI8)bsInFileStream.ReadUB(1);
	lpbrButtonRecord->fButtonStateOver=(UI8)bsInFileStream.ReadUB(1);
	lpbrButtonRecord->fButtonStateUp=(UI8)bsInFileStream.ReadUB(1);
	
	lpbrButtonRecord->usCharacterID=bsInFileStream.ReadUI16();
	lpbrButtonRecord->usPlaceDepth=bsInFileStream.ReadUI16();
	ReadMatrix(&lpbrButtonRecord->mPlaceMatrix);
	if(fCXForm) ReadCXForm(&lpbrButtonRecord->cxfwaColorTransform,true);
	
	bsInFileStream.SeekNextByte();
	ulTagLength=bsInFileStream.GetByteOffset()-ulTagLength;

	return ulTagLength;
}

UI32 CSWF::WriteButtonRecord(BUTTONRECORD *lpbrButtonRecord, UI8 fCXForm)
{
	UI32 ulByteLength=bsOutFileStream.GetByteOffset();

	bsOutFileStream.WriteUB(lpbrButtonRecord->ubButtonReserved,4);
	bsOutFileStream.WriteUB(lpbrButtonRecord->fButtonStateHitTest,1);
	bsOutFileStream.WriteUB(lpbrButtonRecord->fButtonStateDown,1);
	bsOutFileStream.WriteUB(lpbrButtonRecord->fButtonStateOver,1);
	bsOutFileStream.WriteUB(lpbrButtonRecord->fButtonStateUp,1);
	
	bsOutFileStream.WriteUI16(lpbrButtonRecord->usCharacterID);
	bsOutFileStream.WriteUI16(lpbrButtonRecord->usPlaceDepth);
	
	WriteMatrix(&lpbrButtonRecord->mPlaceMatrix);
	if(fCXForm) WriteCXForm(&lpbrButtonRecord->cxfwaColorTransform,true);	

	bsOutFileStream.SeekNextByte();
	ulByteLength=bsOutFileStream.GetByteOffset()-ulByteLength;
	
	return ulByteLength;
}

UI32 CSWF::ReadButtonCondAction(BUTTONCONDACTION *lpbcaButtonAction)
{
	UI32 ulTagLength=bsInFileStream.GetByteOffset();
	ACTIONRECORD *lparActionRecord;
	ACTIONRECORD **lprgarpActions=new ACTIONRECORD*[MAX_ACTIONS];

	lpbcaButtonAction->usCondActionSize=bsInFileStream.ReadUI16();
	lpbcaButtonAction->fCondIdleToOverDown=(UI8)bsInFileStream.ReadUB(1);
	lpbcaButtonAction->fCondOutDownToIdle=(UI8)bsInFileStream.ReadUB(1);
	lpbcaButtonAction->fCondOutDownToOverDown=(UI8)bsInFileStream.ReadUB(1);
	lpbcaButtonAction->fCondOverDownToOutDown=(UI8)bsInFileStream.ReadUB(1);
	lpbcaButtonAction->fCondOverDownToOverUp=(UI8)bsInFileStream.ReadUB(1);
	lpbcaButtonAction->fCondOverUpToOverDown=(UI8)bsInFileStream.ReadUB(1);
	lpbcaButtonAction->fCondOverUpToIdle=(UI8)bsInFileStream.ReadUB(1);
	lpbcaButtonAction->fCondIdleToOverUp=(UI8)bsInFileStream.ReadUB(1);
	lpbcaButtonAction->ubCondKeyPress=(UI8)bsInFileStream.ReadUB(7);
	lpbcaButtonAction->fCondOverDownToIdle=(UI8)bsInFileStream.ReadUB(1);

	ReadActionArray(&lpbcaButtonAction->aaActionArray);
	
	bsInFileStream.SeekNextByte();
	ulTagLength=bsInFileStream.GetByteOffset()-ulTagLength;

	return lpbcaButtonAction->usCondActionSize;
}

UI32 CSWF::WriteButtonCondAction(BUTTONCONDACTION *lpbcaButtonAction)
{
	UI32 ulByteLength=bsOutFileStream.GetByteOffset();
	ACTIONRECORD *lparActionRecord;

	bsOutFileStream.WriteUI16(lpbcaButtonAction->usCondActionSize);//this should be calc.
	bsOutFileStream.WriteUB(lpbcaButtonAction->fCondIdleToOverDown,1);
	bsOutFileStream.WriteUB(lpbcaButtonAction->fCondOutDownToIdle,1);
	bsOutFileStream.WriteUB(lpbcaButtonAction->fCondOutDownToOverDown,1);
	bsOutFileStream.WriteUB(lpbcaButtonAction->fCondOverDownToOutDown,1);
	bsOutFileStream.WriteUB(lpbcaButtonAction->fCondOverDownToOverUp,1);
	bsOutFileStream.WriteUB(lpbcaButtonAction->fCondOverUpToOverDown,1);
	bsOutFileStream.WriteUB(lpbcaButtonAction->fCondOverUpToIdle,1);
	bsOutFileStream.WriteUB(lpbcaButtonAction->fCondIdleToOverUp,1);
	bsOutFileStream.WriteUB(lpbcaButtonAction->ubCondKeyPress,7);
	bsOutFileStream.WriteUB(lpbcaButtonAction->fCondOverDownToIdle,1);
	
	WriteActionArray(&lpbcaButtonAction->aaActionArray);
	
	bsOutFileStream.SeekNextByte();
	ulByteLength=bsOutFileStream.GetByteOffset()-ulByteLength;
	
	return lpbcaButtonAction->usCondActionSize;
}

UI32 CSWF::ReadDefineButtonSound(DEFINEBUTTONSOUND *lpdbsButtonSound)
{
	UI32 ulTagLength=bsInFileStream.GetByteOffset();

	memset(lpdbsButtonSound,0,sizeof(DEFINEBUTTONSOUND));
	memcpy(&lpdbsButtonSound->rhTagHeader,&rhTagHeader,sizeof(RECORDHEADER));
	
	lpdbsButtonSound->usButtonId=bsInFileStream.ReadUI16();
	if(lpdbsButtonSound->usButtonSoundChar0=bsInFileStream.ReadUI16())
		ReadSoundInfo(&lpdbsButtonSound->siButtonSoundInfo0);
	if(lpdbsButtonSound->usButtonSoundChar1=bsInFileStream.ReadUI16())
		ReadSoundInfo(&lpdbsButtonSound->siButtonSoundInfo1);
	if(lpdbsButtonSound->usButtonSoundChar2=bsInFileStream.ReadUI16())
		ReadSoundInfo(&lpdbsButtonSound->siButtonSoundInfo2);
	if(lpdbsButtonSound->usButtonSoundChar3=bsInFileStream.ReadUI16())
		ReadSoundInfo(&lpdbsButtonSound->siButtonSoundInfo3);

	ulTagLength=bsInFileStream.GetByteOffset()-ulTagLength;

	((DEFINEBUTTON*)lprgvpCharacters[lpdbsButtonSound->usButtonId])->lpdbsButtonSound=lpdbsButtonSound;

	return ulTagLength;
}

UI32 CSWF::WriteDefineButtonSound(DEFINEBUTTONSOUND *lpdbsButtonSound, FILE *pfileSWF)
{
	lpdbsButtonSound->rhTagHeader.ulLength=lpdbsButtonSound->siButtonSoundInfo0.ubEnvPoints*sizeof(SOUNDENVELOPE);
	lpdbsButtonSound->rhTagHeader.ulLength+=lpdbsButtonSound->siButtonSoundInfo1.ubEnvPoints*sizeof(SOUNDENVELOPE);
	lpdbsButtonSound->rhTagHeader.ulLength+=lpdbsButtonSound->siButtonSoundInfo2.ubEnvPoints*sizeof(SOUNDENVELOPE);
	lpdbsButtonSound->rhTagHeader.ulLength+=lpdbsButtonSound->siButtonSoundInfo3.ubEnvPoints*sizeof(SOUNDENVELOPE);

	bsOutFileStream.Create(sizeof(DEFINEBUTTONSOUND)+lpdbsButtonSound->rhTagHeader.ulLength);

	bsOutFileStream.WriteUI16(lpdbsButtonSound->usButtonId);
	bsOutFileStream.WriteUI16(lpdbsButtonSound->usButtonSoundChar0);
	if(lpdbsButtonSound->usButtonSoundChar0)
		WriteSoundInfo(&lpdbsButtonSound->siButtonSoundInfo0,pfileSWF);
	bsOutFileStream.WriteUI16(lpdbsButtonSound->usButtonSoundChar1);
	if(lpdbsButtonSound->usButtonSoundChar1)
		WriteSoundInfo(&lpdbsButtonSound->siButtonSoundInfo1,pfileSWF);
	bsOutFileStream.WriteUI16(lpdbsButtonSound->usButtonSoundChar2);
	if(lpdbsButtonSound->usButtonSoundChar2)
		WriteSoundInfo(&lpdbsButtonSound->siButtonSoundInfo2,pfileSWF);
	bsOutFileStream.WriteUI16(lpdbsButtonSound->usButtonSoundChar3);
	if(lpdbsButtonSound->usButtonSoundChar3)
		WriteSoundInfo(&lpdbsButtonSound->siButtonSoundInfo3,pfileSWF);

	lpdbsButtonSound->rhTagHeader.ulLength=bsOutFileStream.GetByteOffset();
	WriteTagHeader(&lpdbsButtonSound->rhTagHeader,pfileSWF);
	bsOutFileStream.WriteToFile(pfileSWF);

	return lpdbsButtonSound->rhTagHeader.ulLength;
}

UI32 CSWF::ReadDefineButtonCXForm(DEFINEBUTTONCXFORM *lpdbcxfButtonCXForm)
{
	UI32 ulTagLength=bsInFileStream.GetByteOffset();

	memset(lpdbcxfButtonCXForm,0,sizeof(DEFINEBUTTONSOUND));
	memcpy(&lpdbcxfButtonCXForm->rhTagHeader,&rhTagHeader,sizeof(RECORDHEADER));

	lpdbcxfButtonCXForm->usButtonID=bsInFileStream.ReadUI16();
	ReadCXForm(&lpdbcxfButtonCXForm->cxfwaButtonCXForm,false);

	ulTagLength=bsInFileStream.GetByteOffset()-ulTagLength;

	return ulTagLength;
}

UI32 CSWF::WriteDefineButtonCXForm(DEFINEBUTTONCXFORM *lpdbcxfButtonCXForm, FILE *pfileSWF)
{
	bsOutFileStream.Create(sizeof(DEFINEBUTTONCXFORM));

	bsOutFileStream.WriteUI16(lpdbcxfButtonCXForm->usButtonID);
	WriteCXForm(&lpdbcxfButtonCXForm->cxfwaButtonCXForm,false);

	lpdbcxfButtonCXForm->rhTagHeader.ulLength=bsOutFileStream.GetByteOffset();
	WriteTagHeader(&lpdbcxfButtonCXForm->rhTagHeader,pfileSWF);
	bsOutFileStream.WriteToFile(pfileSWF);

	return lpdbcxfButtonCXForm->rhTagHeader.ulLength;
}

UI32 CSWF::ExportButtonDependancies(DEFINEBUTTON *lpdbDefineButton, CSWF *cswfButtonFlash)
{
	for(UI16 iCharacter=0; iCharacter<lpdbDefineButton->ulNumCharacters; iCharacter++)
	{
		UI16 usCharacterID=lpdbDefineButton->lprgbrpCharacters[iCharacter]->usCharacterID;
		UNKNOWNTAG *lputCharacter=(UNKNOWNTAG*)lprgvpCharacters[usCharacterID];

		/*export this characters dependancies*/
		if(lputCharacter->rhTagHeader.usTagCode==TAGCODE_DEFINESHAPE)
			ExportShapeDependancies((DEFINESHAPE*)lprgvpCharacters[usCharacterID],cswfButtonFlash);

		if(lputCharacter->rhTagHeader.usTagCode==TAGCODE_DEFINESPRITE)
			ExportSpriteDependancies((DEFINESPRITE*)lprgvpCharacters[usCharacterID],cswfButtonFlash);

		/*If it's text, write the proper font*/
		if(lputCharacter->rhTagHeader.usTagCode==TAGCODE_DEFINETEXT ||
		   lputCharacter->rhTagHeader.usTagCode==TAGCODE_DEFINETEXT2)
			for(UI32 ulTextRecord=0; ulTextRecord<((DEFINETEXT*)lputCharacter)->ulNumTextRecords; ulTextRecord++)
			{
				TEXTRECORD *lptrTextRecord=((DEFINETEXT*)lputCharacter)->lprgtrpTextRecords[ulTextRecord];

				if(lptrTextRecord->fStyleFlagsHasFont)
					cswfButtonFlash->AddTag(lprgvpCharacters[lptrTextRecord->usFontID]);
			}

		cswfButtonFlash->AddTag(lprgvpCharacters[usCharacterID]);
	}

	/*export button sounds*/
	if(lpdbDefineButton->lpdbsButtonSound)
	{
		cswfButtonFlash->AddTag(lprgvpCharacters[lpdbDefineButton->lpdbsButtonSound->usButtonSoundChar0]);
		cswfButtonFlash->AddTag(lprgvpCharacters[lpdbDefineButton->lpdbsButtonSound->usButtonSoundChar1]);
		cswfButtonFlash->AddTag(lprgvpCharacters[lpdbDefineButton->lpdbsButtonSound->usButtonSoundChar2]);
		cswfButtonFlash->AddTag(lprgvpCharacters[lpdbDefineButton->lpdbsButtonSound->usButtonSoundChar3]);
	}

	return 1;
}

UI32 CSWF::SaveDefineButton(DEFINEBUTTON* lpdbDefineButton)
{
	UI32 ulLength;
	char szShapeFileName[MAX_FILEPATH];
	CSWF cswfButtonFlash;

	SI32 slWidth=(fhFrameHeader.srFrameSize.slXMax-fhFrameHeader.srFrameSize.slXMin)/2;
	SI32 slHeight=(fhFrameHeader.srFrameSize.slYMax-fhFrameHeader.srFrameSize.slYMin)/2;
	
	/*SWFFILEHEADER*/
	SWFFILEHEADER fhShapeFileHeader;
	fhShapeFileHeader.bSignature1='F';
	fhShapeFileHeader.bSignature2='W';
	fhShapeFileHeader.bSignature3='S';
	fhShapeFileHeader.bVersion=6;
	fhShapeFileHeader.ulFileLength=0;
	cswfButtonFlash.SetFileHeader(&fhShapeFileHeader);
	
	/*SWFFRAMEHEADER*/
	SWFFRAMEHEADER fhShapeFrameHeader;
	fhShapeFrameHeader.sfpFrameRate=fhFrameHeader.sfpFrameRate;
	fhShapeFrameHeader.usFrameCount=1;
	fhShapeFrameHeader.srFrameSize.bNbits=31;
	fhShapeFrameHeader.srFrameSize.slXMin=-1*slWidth;
	fhShapeFrameHeader.srFrameSize.slYMin=-1*slHeight;
	fhShapeFrameHeader.srFrameSize.slXMax=slWidth;
	fhShapeFrameHeader.srFrameSize.slYMax=slHeight;
	cswfButtonFlash.SetFrameHeader(&fhShapeFrameHeader);

	/*SETBACKGROUNDCOLOR*/
	SETBACKGROUNDCOLOR sbcShapeBackground;
	sbcShapeBackground.rhTagHeader.usTagCode=TAGCODE_SETBACKGROUNDCOLOR;
	sbcShapeBackground.rhTagHeader.ulLength=3;
	sbcShapeBackground.rgbBackgroundColor.ubRed=128;
	sbcShapeBackground.rgbBackgroundColor.ubGreen=128;
	sbcShapeBackground.rgbBackgroundColor.ubBlue=128;
	cswfButtonFlash.AddTag(&sbcShapeBackground);
	
	/*Fill Images*/
	cswfButtonFlash.AddTag(lpjtJPEGTables);

	/*DEFINEBUTTON*/
	ExportButtonDependancies(lpdbDefineButton,&cswfButtonFlash);
	cswfButtonFlash.AddTag(lpdbDefineButton);
	cswfButtonFlash.AddTag(lpdbDefineButton->lpdbsButtonSound);

	/*PLACEOBJECT(2)*/
	MATRIX mMatrix;
	mMatrix.fHasScale=0;
	mMatrix.lfpScaleX=1;
	mMatrix.lfpScaleY=1;
	mMatrix.fHasRotate=0;
	mMatrix.ubNTranslateBits=31;
	mMatrix.slTranslateX=0;
	mMatrix.slTranslateY=0;

	PLACEOBJECT2 po2Place;
	memset(&po2Place,0,sizeof(PLACEOBJECT2));
	po2Place.rhTagHeader.usTagCode=TAGCODE_PLACEOBJECT2;
	po2Place.fPlaceFlagHasMatrix=true;
	po2Place.fPlaceFlagHasCharacter=true;
	po2Place.fPlaceFlagHasClipDepth=true;
	po2Place.usCharacterID=lpdbDefineButton->usButtonID;
	po2Place.usDepth=1;
	memcpy(&po2Place.mMatrix,&mMatrix,sizeof(MATRIX));
	cswfButtonFlash.AddTag(&po2Place);
	
	/*SHOWFRAME and END*/
	UNKNOWNTAG utShowFrame;
	UNKNOWNTAG utEnd;
	memset(&utShowFrame,0,sizeof(UNKNOWNTAG));
	memset(&utEnd,0,sizeof(UNKNOWNTAG));
	utShowFrame.rhTagHeader.usTagCode=TAGCODE_SHOWFRAME;
	utEnd.rhTagHeader.usTagCode=TAGCODE_END;
	cswfButtonFlash.AddTag(&utShowFrame);
	cswfButtonFlash.AddTag(&utEnd);
	
	/*Save*/
	CreateFileName(szShapeFileName,"button",lpdbDefineButton->usButtonID,"swf");
	cswfButtonFlash.Save(szShapeFileName);

	return 0;
}
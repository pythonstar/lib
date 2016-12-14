#include "swf.h"

UI32 CSWF::ReadDefineSound(DEFINESOUND *lpdsDefineSound)
{
	UI32 ulTagLength=bsInFileStream.GetByteOffset();

	memset(lpdsDefineSound,0,sizeof(DEFINESOUND));
	memcpy(&lpdsDefineSound->rhTagHeader,&rhTagHeader,sizeof(RECORDHEADER));

	lpdsDefineSound->usSoundID=(UI16)bsInFileStream.ReadUI16();
	lpdsDefineSound->ubSoundFormat=(UI8)bsInFileStream.ReadUB(4);
	lpdsDefineSound->ubSoundRate=(UI8)bsInFileStream.ReadUB(2);
	lpdsDefineSound->ubSoundSize=(UI8)bsInFileStream.ReadUB(1);
	lpdsDefineSound->ubSoundType=(UI8)bsInFileStream.ReadUB(1);
	lpdsDefineSound->ulSoundSampleCount=bsInFileStream.ReadUI32();

	lpdsDefineSound->ulSoundDataSize=lpdsDefineSound->rhTagHeader.ulLength-(bsInFileStream.GetByteOffset()-ulTagLength);
	lpdsDefineSound->lprgbSoundData=new UI8[lpdsDefineSound->ulSoundDataSize];
	bsInFileStream.ReadBytes(lpdsDefineSound->lprgbSoundData,lpdsDefineSound->ulSoundDataSize);	

	bsInFileStream.SeekNextByte();
	ulTagLength=bsInFileStream.GetByteOffset()-ulTagLength;

	return ulTagLength;
}

UI32 CSWF::WriteDefineSound(DEFINESOUND *lpdsDefineSound, FILE *pfileSWF)
{
	bsOutFileStream.Create(sizeof(DEFINESOUND)+lpdsDefineSound->ulSoundDataSize);

	bsOutFileStream.WriteUI16(lpdsDefineSound->usSoundID);
	bsOutFileStream.WriteUB(lpdsDefineSound->ubSoundFormat,4);
	bsOutFileStream.WriteUB(lpdsDefineSound->ubSoundRate,2);
	bsOutFileStream.WriteUB(lpdsDefineSound->ubSoundSize,1);
	bsOutFileStream.WriteUB(lpdsDefineSound->ubSoundType,1);
	bsOutFileStream.WriteUI32(lpdsDefineSound->ulSoundSampleCount);

	bsOutFileStream.WriteBytes(lpdsDefineSound->lprgbSoundData,lpdsDefineSound->ulSoundDataSize);	

	lpdsDefineSound->rhTagHeader.ulLength=bsOutFileStream.GetByteOffset();
	WriteTagHeader(&lpdsDefineSound->rhTagHeader,pfileSWF);
	bsOutFileStream.WriteToFile(pfileSWF);

	return lpdsDefineSound->rhTagHeader.ulLength;
}

UI32 CSWF::SaveDefineSoundAsFile(DEFINESOUND *lpdsDefineSound)
{
	FILE *pfileSound;
	char szSoundFileName[4096], szExt[10];

	switch(lpdsDefineSound->ubSoundFormat)
	{
		case 0: strcpy(szExt,"RAW"); break;
		case 1: strcpy(szExt,"ADPCM"); break;
		case 2: strcpy(szExt,"mp3"); break;
		case 3: strcpy(szExt,"WAR"); break;
		case 6: strcat(szExt,".NEL"); break;
	}

	/*Create file*/
	CreateFileName(szSoundFileName,"sound",lpdsDefineSound->usSoundID,szExt);
	pfileSound=fopen(szSoundFileName,"wb");
	fwrite(lpdsDefineSound->lprgbSoundData,1,lpdsDefineSound->ulSoundDataSize,pfileSound);
	fclose(pfileSound);

	return 0;
}

UI32 CSWF::ReadSoundStreamBlock(SOUNDSTREAMBLOCK *lpssbSoundStreamBlock)
{
	UI32 ulTagLength=bsInFileStream.GetByteOffset();

	memset(lpssbSoundStreamBlock,0,sizeof(SOUNDSTREAMBLOCK));
	memcpy(&lpssbSoundStreamBlock->rhTagHeader,&rhTagHeader,sizeof(RECORDHEADER));

	lpssbSoundStreamBlock->ulSoundStreamDataSize=lpssbSoundStreamBlock->rhTagHeader.ulLength-(bsInFileStream.GetByteOffset()-ulTagLength);
	lpssbSoundStreamBlock->lprgbSoundStreamData=new UI8[lpssbSoundStreamBlock->ulSoundStreamDataSize];
	bsInFileStream.ReadBytes(lpssbSoundStreamBlock->lprgbSoundStreamData,lpssbSoundStreamBlock->ulSoundStreamDataSize);	

	bsInFileStream.SeekNextByte();
	ulTagLength=bsInFileStream.GetByteOffset()-ulTagLength;

	return ulTagLength;
}

UI32 CSWF::WriteSoundStreamBlock(SOUNDSTREAMBLOCK *lpssbSoundStreamBlock, FILE *pfileSWF)
{
	bsOutFileStream.Create(sizeof(SOUNDSTREAMBLOCK)+lpssbSoundStreamBlock->ulSoundStreamDataSize);

	bsOutFileStream.WriteBytes(lpssbSoundStreamBlock->lprgbSoundStreamData,lpssbSoundStreamBlock->ulSoundStreamDataSize);	

	lpssbSoundStreamBlock->rhTagHeader.ulLength=bsOutFileStream.GetByteOffset();
	WriteTagHeader(&lpssbSoundStreamBlock->rhTagHeader,pfileSWF);
	bsOutFileStream.WriteToFile(pfileSWF);

	return lpssbSoundStreamBlock->rhTagHeader.ulLength;

}

UI32 CSWF::SaveSoundStream(DEFINESPRITE *lpdsSprite)
{
	FILE *pfileSound;
	char szSoundFileName[4096];
	void **lprgvpTagArray;
	UI32 ulNumTags;
	UI32 ulID;

	if(lpdsSprite)
	{
		lprgvpTagArray=lpdsSprite->lprgvpControlTags;
		ulNumTags=lpdsSprite->ulNumControlTags;
		ulID=lpdsSprite->usSpriteID;
		if(!HasSoundStream(lpdsSprite->usSpriteID)) return 0;
	}

	else
	{
		lprgvpTagArray=lprgvpTags;
		ulNumTags=nTags;
		ulID=0;
		if(!HasSoundStream(0)) return 0;
	}


	/*Create file*/
	CreateFileName(szSoundFileName,"sound",ulID,"mp3");
	pfileSound=fopen(szSoundFileName,"wb");

	for(UI32 iTag=0; iTag<ulNumTags; iTag++)
	{
		if(((UNKNOWNTAG*)lprgvpTagArray[iTag])->rhTagHeader.usTagCode==TAGCODE_SOUNDSTREAMBLOCK)
		{
		
			fwrite(((SOUNDSTREAMBLOCK*)lprgvpTagArray[iTag])->lprgbSoundStreamData+4,1,
				   ((SOUNDSTREAMBLOCK*)lprgvpTagArray[iTag])->ulSoundStreamDataSize-4,pfileSound);
		}

	}
	
	fclose(pfileSound);
	return 0;
}

UI32 CSWF::SaveSound(UI16 usSoundID)
{
	UNKNOWNTAG *lputTag=(UNKNOWNTAG*)lprgvpCharacters[usSoundID];
	
	if(!usSoundID) SaveSoundStream(NULL);

	else if(lputTag->rhTagHeader.usTagCode==TAGCODE_DEFINESPRITE)
		return SaveSoundStream((DEFINESPRITE*)lputTag);

	else if(lputTag->rhTagHeader.usTagCode==TAGCODE_DEFINESOUND)
		SaveDefineSoundAsFile((DEFINESOUND*)lputTag);\

	return 0;
}

UI32 CSWF::ReadStartSound(STARTSOUND *lpssStartSound)
{
	UI32 ulTagLength=bsInFileStream.GetByteOffset();

	memset(lpssStartSound,0,sizeof(STARTSOUND));
	memcpy(&lpssStartSound->rhTagHeader,&rhTagHeader,sizeof(RECORDHEADER));

	lpssStartSound->usSoundID=bsInFileStream.ReadUI16();
	ReadSoundInfo(&lpssStartSound->siSoundInfo);

	bsInFileStream.SeekNextByte();
	ulTagLength=bsInFileStream.GetByteOffset()-ulTagLength;

	return ulTagLength;

}

UI32 CSWF::WriteStartSound(STARTSOUND *lpssStartSound, FILE *pfileSWF)
{
	bsOutFileStream.Create(sizeof(STARTSOUND)+lpssStartSound->siSoundInfo.ubEnvPoints*sizeof(SOUNDENVELOPE));

	bsOutFileStream.WriteUI16(lpssStartSound->usSoundID);
	WriteSoundInfo(&lpssStartSound->siSoundInfo,pfileSWF);

	lpssStartSound->rhTagHeader.ulLength=bsOutFileStream.GetByteOffset();
	WriteTagHeader(&lpssStartSound->rhTagHeader,pfileSWF);
	bsOutFileStream.WriteToFile(pfileSWF);

	return lpssStartSound->rhTagHeader.ulLength;
}

UI32 CSWF::ReadSoundInfo(SOUNDINFO *lpsiSoundInfo)
{
	UI32 ulTagLength=bsInFileStream.GetByteOffset();

	lpsiSoundInfo->ubReserved=(UI8)bsInFileStream.ReadUB(2);
	lpsiSoundInfo->ubSyncStop=(UI8)bsInFileStream.ReadUB(1);
	lpsiSoundInfo->ubSyncNoMultiple=(UI8)bsInFileStream.ReadUB(1);
	lpsiSoundInfo->ubHasEnvelope=(UI8)bsInFileStream.ReadUB(1);
	lpsiSoundInfo->ubHasLoops=(UI8)bsInFileStream.ReadUB(1);
	lpsiSoundInfo->ubHasOutPoint=(UI8)bsInFileStream.ReadUB(1);
	lpsiSoundInfo->ubHasInPoint=(UI8)bsInFileStream.ReadUB(1);

	if(lpsiSoundInfo->ubHasInPoint)		lpsiSoundInfo->ulInPoint=bsInFileStream.ReadUI32();
	if(lpsiSoundInfo->ubHasOutPoint)	lpsiSoundInfo->ulOutPoint=bsInFileStream.ReadUI32();
	if(lpsiSoundInfo->ubHasLoops)		lpsiSoundInfo->usLoopCount=bsInFileStream.ReadUI16();
	if(lpsiSoundInfo->ubHasEnvelope)	
	{
		lpsiSoundInfo->ubEnvPoints=bsInFileStream.ReadUI8();
		lpsiSoundInfo->lprgseEnvelopeRecords=new SOUNDENVELOPE[lpsiSoundInfo->ubEnvPoints];
		
		for(UI8 ubCurEnvelope=0; ubCurEnvelope<lpsiSoundInfo->ubEnvPoints; ubCurEnvelope++)
			ReadSoundEnvelope(&lpsiSoundInfo->lprgseEnvelopeRecords[ubCurEnvelope]);
	}

	ulTagLength=bsInFileStream.GetByteOffset()-ulTagLength;
	return ulTagLength;
}

UI32 CSWF::WriteSoundInfo(SOUNDINFO *lpsiSoundInfo, FILE *pfileSWF)
{
	UI32 ulLength=bsOutFileStream.GetByteOffset();

	bsOutFileStream.WriteUB(lpsiSoundInfo->ubReserved,2);
	bsOutFileStream.WriteUB(lpsiSoundInfo->ubSyncStop,1);
	bsOutFileStream.WriteUB(lpsiSoundInfo->ubSyncNoMultiple,1);
	bsOutFileStream.WriteUB(lpsiSoundInfo->ubHasEnvelope,1);
	bsOutFileStream.WriteUB(lpsiSoundInfo->ubHasLoops,1);
	bsOutFileStream.WriteUB(lpsiSoundInfo->ubHasOutPoint,1);
	bsOutFileStream.WriteUB(lpsiSoundInfo->ubHasInPoint,1);

	if(lpsiSoundInfo->ubHasInPoint)		bsOutFileStream.WriteUI32(lpsiSoundInfo->ulInPoint);
	if(lpsiSoundInfo->ubHasOutPoint)	bsOutFileStream.WriteUI32(lpsiSoundInfo->ulOutPoint);
	if(lpsiSoundInfo->ubHasLoops)		bsOutFileStream.WriteUI16(lpsiSoundInfo->usLoopCount);
	if(lpsiSoundInfo->ubHasEnvelope)	
	{
		bsOutFileStream.WriteUI8(lpsiSoundInfo->ubEnvPoints);
		for(UI8 ubCurEnvelope=0; ubCurEnvelope<lpsiSoundInfo->ubEnvPoints; ubCurEnvelope++)
			WriteSoundEnvelope(&lpsiSoundInfo->lprgseEnvelopeRecords[ubCurEnvelope],pfileSWF);
	}

	ulLength=bsOutFileStream.GetByteOffset()-ulLength;
	return ulLength;
}

UI32 CSWF::ReadSoundEnvelope(SOUNDENVELOPE *lpseSoundEnvelope)
{
	UI32 ulTagLength=bsInFileStream.GetByteOffset();

	lpseSoundEnvelope->ulPos44=bsInFileStream.ReadUI32();
	lpseSoundEnvelope->usLeftLevel=bsInFileStream.ReadUI16();
	lpseSoundEnvelope->usRightLevel=bsInFileStream.ReadUI16();

	ulTagLength=bsInFileStream.GetByteOffset()-ulTagLength;

	return ulTagLength;
}

UI32 CSWF::WriteSoundEnvelope(SOUNDENVELOPE *lpseSoundEnvelope, FILE *pfileSWF)
{
	UI32 ulLength=bsOutFileStream.GetByteOffset();

	bsOutFileStream.WriteUI32(lpseSoundEnvelope->ulPos44);
	bsOutFileStream.WriteUI16(lpseSoundEnvelope->usLeftLevel);
	bsOutFileStream.WriteUI16(lpseSoundEnvelope->usRightLevel);

	ulLength=bsOutFileStream.GetByteOffset()-ulLength;
	return ulLength;
}
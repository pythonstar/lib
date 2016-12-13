#include "swf.h"

/*SETBACKGROUNDCOLOR*/

UI32 CSWF::ReadSetBackgroundColor(SETBACKGROUNDCOLOR *lpsbcSetBackground)
{
	memset(lpsbcSetBackground,0,sizeof(SETBACKGROUNDCOLOR));
	memcpy(&lpsbcSetBackground->rhTagHeader,&rhTagHeader,sizeof(RECORDHEADER));

	ReadRGB(&lpsbcSetBackground->rgbBackgroundColor,false);

	return rhTagHeader.ulLength;
}

UI32 CSWF::WriteSetBackgroundColor(SETBACKGROUNDCOLOR *lpsbcSetBackground, FILE *pfileSWF)
{
	bsOutFileStream.Create(sizeof(SETBACKGROUNDCOLOR));

	WriteRGB(&lpsbcSetBackground->rgbBackgroundColor,false);
	
	lpsbcSetBackground->rhTagHeader.usTagCode=TAGCODE_SETBACKGROUNDCOLOR;
	lpsbcSetBackground->rhTagHeader.ulLength=bsOutFileStream.GetByteOffset();
	WriteTagHeader(&lpsbcSetBackground->rhTagHeader,pfileSWF);
	bsOutFileStream.WriteToFile(pfileSWF);

	return bsOutFileStream.GetByteOffset();  
}

UI32 CSWF::ReadProtect(PROTECT *lppProtect)
{
	memcpy(&lppProtect->rhTagHeader,&rhTagHeader,sizeof(RECORDHEADER));
	if(lppProtect->rhTagHeader.ulLength)
		bsInFileStream.ReadString(lppProtect->szPassword,lppProtect->rhTagHeader.ulLength);
	else lppProtect->szPassword=NULL;
	return lppProtect->rhTagHeader.ulLength;
}

UI32 CSWF::WriteProtect(PROTECT *lppProtect, FILE *pfileSWF)
{

	if(lppProtect->szPassword) lppProtect->rhTagHeader.ulLength=strlen(lppProtect->szPassword);
	else lppProtect->rhTagHeader.ulLength=0;
	
	bsOutFileStream.Create(sizeof(RECORDHEADER)+lppProtect->rhTagHeader.ulLength);
	
	if(lppProtect->rhTagHeader.ulLength)
		bsOutFileStream.WriteString(lppProtect->szPassword,lppProtect->rhTagHeader.ulLength);

	lppProtect->rhTagHeader.usTagCode=TAGCODE_PROTECT;
	lppProtect->rhTagHeader.ulLength=bsOutFileStream.GetByteOffset();
	WriteTagHeader(&lppProtect->rhTagHeader,pfileSWF);
	if(lppProtect->rhTagHeader.ulLength)
		bsOutFileStream.WriteToFile(pfileSWF);
	
	return lppProtect->rhTagHeader.ulLength;
}

UI32 CSWF::ReadFrameLabel(FRAMELABEL *lpflFrameLabel)
{
	memset(lpflFrameLabel,0,sizeof(FRAMELABEL));
	memcpy(&lpflFrameLabel->rhTagHeader,&rhTagHeader,sizeof(RECORDHEADER));

	bsInFileStream.ReadString(lpflFrameLabel->szLabel);

	return rhTagHeader.ulLength;
}

UI32 CSWF::WriteFrameLabel(FRAMELABEL *lpflFrameLabel, FILE *pfileSWF)
{
	bsOutFileStream.Create(sizeof(FRAMELABEL)+strlen(lpflFrameLabel->szLabel));

	bsOutFileStream.WriteString(lpflFrameLabel->szLabel);
	
	lpflFrameLabel->rhTagHeader.usTagCode=TAGCODE_FRAMELABEL;
	lpflFrameLabel->rhTagHeader.ulLength=bsOutFileStream.GetByteOffset();
	WriteTagHeader(&lpflFrameLabel->rhTagHeader,pfileSWF);
	bsOutFileStream.WriteToFile(pfileSWF);

	return bsOutFileStream.GetByteOffset();  
}

FRAMELABEL *CSWF::GetFrameLabel(UI16 usClipID, UI16 usFrame)
{
	SI32 slLeft, slRight, slMiddle;
	DEFINESPRITE* lpdsSprite;
	FRAMELABEL **lprgflpLabelArray;
	UI16 usLabels;

	if(!usClipID)
	{
		lprgflpLabelArray=lprgflpLabels;
		usLabels=usNumLabels;
	}

	else
	{
		lpdsSprite=(DEFINESPRITE*)lprgvpCharacters[usClipID];
		if(lpdsSprite->rhTagHeader.usTagCode!=TAGCODE_DEFINESPRITE) return 0;
		lprgflpLabelArray=lpdsSprite->lprgflpLabels;
		usLabels=lpdsSprite->usNumLabels;
	}
	
	slLeft=0;
	slRight=usLabels-1;

	if(!lprgflpLabelArray) return 0;

	while(slLeft<=slRight)
	{
		slMiddle=(slLeft+slRight)/2;

		if(lprgflpLabelArray[slMiddle]->usFrame==usFrame)
			return lprgflpLabelArray[slMiddle];
		
		else if(lprgflpLabelArray[slMiddle]->usFrame<usFrame)
			slLeft=slMiddle+1;
		
		else slRight=slMiddle-1;
	}

	return NULL;
}

UI32 CSWF::WriteEnd(FILE* pfileSWF)
{
	UI16 usEnd=0;
	fwrite(&usEnd,1,sizeof(UI16),pfileSWF);
	return ftell(pfileSWF);
}

UI32 CSWF::ReadExportAssets(EXPORTASSETS *lpeaExport)
{
	memset(lpeaExport,0,sizeof(EXPORTASSETS));
	memcpy(&lpeaExport->rhTagHeader,&rhTagHeader,sizeof(RECORDHEADER));

	lpeaExport->usCount=bsInFileStream.ReadUI16();
	lpeaExport->lprgaAssets=new ASSET[lpeaExport->usCount];

	for(UI32 iAsset=0; iAsset<lpeaExport->usCount; iAsset++)
	{
		lpeaExport->lprgaAssets[iAsset].usCharacterID=bsInFileStream.ReadUI16();
		bsInFileStream.ReadString(lpeaExport->lprgaAssets[iAsset].szName);
	}

	return rhTagHeader.ulLength;
}

UI32 CSWF::WriteExportAssets(EXPORTASSETS *lpeaExport, FILE *pfileSWF)
{
	bsOutFileStream.Create(sizeof(EXPORTASSETS)+128*lpeaExport->usCount);

	bsOutFileStream.WriteUI16(lpeaExport->usCount);

	for(UI32 iAsset=0; iAsset<lpeaExport->usCount; iAsset++)
	{
		bsOutFileStream.WriteUI16(lpeaExport->lprgaAssets[iAsset].usCharacterID);
		bsOutFileStream.WriteString(lpeaExport->lprgaAssets[iAsset].szName);
	}
	
	lpeaExport->rhTagHeader.usTagCode=TAGCODE_EXPORTASSETS;
	lpeaExport->rhTagHeader.ulLength=bsOutFileStream.GetByteOffset();
	WriteTagHeader(&lpeaExport->rhTagHeader,pfileSWF);
	bsOutFileStream.WriteToFile(pfileSWF);

	return bsOutFileStream.GetByteOffset();  
}

UI32 CSWF::DeleteAssets(ASSET *lprgaAssets, UI16 usCount)
{
	for(UI16 iAsset=0; iAsset<usCount; iAsset++)
		if(lprgaAssets[iAsset].szName) 
			delete lprgaAssets[iAsset].szName;

	delete lprgaAssets;
	
	return 0;
}

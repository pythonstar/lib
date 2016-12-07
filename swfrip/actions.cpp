#include "swf.h"

void CSWF::GetActionName(UI8 ubActionCode, char *szName)
{
	switch(ubActionCode)
	{
		case ACTIONCODE_GOTOFRAME:			strcpy(szName,"gotoFrame"); break;		
		case ACTIONCODE_GETURL:				strcpy(szName,"getURL"); break;
		case ACTIONCODE_NEXTFRAME:			strcpy(szName,"nextFrame"); break;
		case ACTIONCODE_PREVIOUSFRAME:		strcpy(szName,"prevFrame"); break;
		case ACTIONCODE_PLAY:				strcpy(szName,"play"); break;
		case ACTIONCODE_STOP:				strcpy(szName,"stop"); break;
		case ACTIONCODE_TOGGLEQUALITY:		strcpy(szName,"toggleQuality"); break;
		case ACTIONCODE_STOPSOUNDS:			strcpy(szName,"stopSounds"); break;
		case ACTIONCODE_WAITFORFRAME:		strcpy(szName,"waitForFrame"); break;
		case ACTIONCODE_SETTARGET:			strcpy(szName,"setTarget"); break;
		case ACTIONCODE_GOTOLABEL:			strcpy(szName,"gotoLabel"); break;
		case ACTIONCODE_PUSH:				strcpy(szName,"push"); break;
		case ACTIONCODE_POP:				strcpy(szName,"pop"); break;
		case ACTIONCODE_ADD:				strcpy(szName,"add"); break;
		case ACTIONCODE_SUBTRACT:			strcpy(szName,"subtract"); break;
		case ACTIONCODE_MULTIPLY:			strcpy(szName,"multiply"); break;
		case ACTIONCODE_DIVIDE:				strcpy(szName,"divide"); break;
		case ACTIONCODE_EQUALS:				strcpy(szName,"equals"); break;
		case ACTIONCODE_LESS:				strcpy(szName,"less"); break;
		case ACTIONCODE_AND:				strcpy(szName,"and"); break;
		case ACTIONCODE_OR:					strcpy(szName,"or"); break;
		case ACTIONCODE_NOT:				strcpy(szName,"not"); break;
		case ACTIONCODE_STRINGEQUALS:		strcpy(szName,"stringEquals"); break;
		case ACTIONCODE_STRINGLENGTH:		strcpy(szName,"stringLength"); break;
		case ACTIONCODE_STRINGADD:			strcpy(szName,"stringAdd"); break;
		case ACTIONCODE_STRINGEXTRACT:		strcpy(szName,"stringExtract"); break;
		case ACTIONCODE_STRINGLESS:			strcpy(szName,"stringLess"); break;
		case ACTIONCODE_MBSTRINGLENGTH:		strcpy(szName,"mbStringLength"); break;
		case ACTIONCODE_MBSTRINGEXTRACT:	strcpy(szName,"mbStringExtract"); break;
		case ACTIONCODE_TOINTEGER:			strcpy(szName,"toInteger"); break;
		case ACTIONCODE_CHARTOASCII:		strcpy(szName,"CHARToASCII"); break;
		case ACTIONCODE_ASCIITOCHAR:		strcpy(szName,"ASCIIToCHAR"); break;
		case ACTIONCODE_MBCHARTOASCII:		strcpy(szName,"MBCHARToASCII"); break;
		case ACTIONCODE_MBASCIITOCHAR:		strcpy(szName,"MBASCIIToCHAR"); break;
		case ACTIONCODE_JUMP:				strcpy(szName,"jump"); break;
		case ACTIONCODE_IF:					strcpy(szName,"if"); break;
		case ACTIONCODE_CALL:				strcpy(szName,"call"); break;
		case ACTIONCODE_GETVARIABLE:		strcpy(szName,"getVariable"); break;
		case ACTIONCODE_SETVARIABLE:		strcpy(szName,"setVariable"); break;
		case ACTIONCODE_GETURL2:			strcpy(szName,"getURL2"); break;
		case ACTIONCODE_GOTOFRAME2:			strcpy(szName,"gotoFrame2"); break;
		case ACTIONCODE_SETTARGET2:			strcpy(szName,"setTarget2"); break;
		case ACTIONCODE_GETPROPERTY:		strcpy(szName,"getProperty"); break;
		case ACTIONCODE_SETPROPERTY:		strcpy(szName,"setProperty"); break;
		case ACTIONCODE_CLONESPRITE:		strcpy(szName,"cloneSprite"); break;
		case ACTIONCODE_REMOVESPRITE:		strcpy(szName,"removeSprite"); break;
		case ACTIONCODE_STARTDRAG:			strcpy(szName,"startDrag"); break;
		case ACTIONCODE_ENDDRAG:			strcpy(szName,"stopDrag"); break;
		case ACTIONCODE_WAITFORFRAME2:		strcpy(szName,"waitForFrame2"); break;
		case ACTIONCODE_TRACE:				strcpy(szName,"trace"); break;
		case ACTIONCODE_GETTIME:			strcpy(szName,"getTimer"); break;
		case ACTIONCODE_RANDOMNUMBER:		strcpy(szName,"random"); break;
		case ACTIONCODE_CALLFUNCTION:		strcpy(szName,"callFunction"); break;
		case ACTIONCODE_CALLMETHOD:			strcpy(szName,"callMethod"); break;
		case ACTIONCODE_CONSTANTPOOL:		strcpy(szName,"constantPool"); break;
		case ACTIONCODE_DEFINEFUNCTION:		strcpy(szName,"defineFunction"); break;
		case ACTIONCODE_DEFINELOCAL:		strcpy(szName,"defineLocal"); break;
		case ACTIONCODE_DEFINELOCAL2:		strcpy(szName,"defineLocal2"); break;
		case ACTIONCODE_DELETE:				strcpy(szName,"delete"); break;
		case ACTIONCODE_DELETE2:			strcpy(szName,"delete2"); break;
		case ACTIONCODE_ENUMERATE:			strcpy(szName,"enumerate"); break;
		case ACTIONCODE_EQUALS2:			strcpy(szName,"equals2"); break;
		case ACTIONCODE_GETMEMBER:			strcpy(szName,"getMember"); break;
		case ACTIONCODE_INITARRAY:			strcpy(szName,"initArray"); break;
		case ACTIONCODE_INITOBJECT:			strcpy(szName,"initObject"); break;
		case ACTIONCODE_NEWMETHOD:			strcpy(szName,"newMethod"); break;
		case ACTIONCODE_NEWOBJECT:			strcpy(szName,"newObject"); break;
		case ACTIONCODE_SETMEMBER:			strcpy(szName,"setMember"); break;
		case ACTIONCODE_TARGETPATH:			strcpy(szName,"targetPath"); break;
		case ACTIONCODE_WITH:				strcpy(szName,"with"); break;
		case ACTIONCODE_TONUMBER:			strcpy(szName,"toNumber"); break;
		case ACTIONCODE_TOSTRING:			strcpy(szName,"toString"); break;
		case ACTIONCODE_TYPEOF:				strcpy(szName,"typeOf"); break;
		case ACTIONCODE_ADD2:				strcpy(szName,"add2"); break;
		case ACTIONCODE_LESS2:				strcpy(szName,"less2"); break;
		case ACTIONCODE_MODULO:				strcpy(szName,"modulo"); break;
		case ACTIONCODE_BITAND:				strcpy(szName,"bitAnd"); break;
		case ACTIONCODE_BITLSHIFT:			strcpy(szName,"bitLShift"); break;
		case ACTIONCODE_BITOR:				strcpy(szName,"bitOr"); break;
		case ACTIONCODE_BITRSHIFT:			strcpy(szName,"bitRShift"); break;
		case ACTIONCODE_BITURSHIFT:			strcpy(szName,"bitURShift"); break;
		case ACTIONCODE_BITXOR:				strcpy(szName,"bitXor"); break;
		case ACTIONCODE_DECREMENT:			strcpy(szName,"decrement"); break;
		case ACTIONCODE_INCREMENT:			strcpy(szName,"increment"); break;
		case ACTIONCODE_PUSHDUPLICATE:		strcpy(szName,"pushDuplicate"); break;
		case ACTIONCODE_RETURN:				strcpy(szName,"return"); break;
		case ACTIONCODE_STACKSWAP:			strcpy(szName,"stackSwap"); break;
		case ACTIONCODE_STOREREGISTER:		strcpy(szName,"storeRegister"); break;
		case ACTIONCODE_INSTANCEOF:			strcpy(szName,"instanceOf"); break;
		case ACTIONCODE_ENUMERATE2:			strcpy(szName,"enumerate2"); break;
		case ACTIONCODE_STRICTEQUALS:		strcpy(szName,"strictEquals"); break;
		case ACTIONCODE_GREATER:			strcpy(szName,"greater"); break;
		case ACTIONCODE_STRINGGREATER:		strcpy(szName,"stringGreater"); break;
		case ACTIONCODE_DEFINEFUNCTION2:	strcpy(szName,"defineFunction2"); break;
		case ACTIONCODE_EXTENDS:			strcpy(szName,"extends"); break;
		case ACTIONCODE_CASTOP:				strcpy(szName,"caseOp"); break;
		case ACTIONCODE_IMPLEMENTSOP:		strcpy(szName,"implementsOp"); break;
		case ACTIONCODE_TRY:				strcpy(szName,"try"); break;
		case ACTIONCODE_THROW:				strcpy(szName,"throw"); break;
		case 0:								strcpy(szName,"end"); break;
		default:							sprintf(szName,"0x%02X",ubActionCode);
	}
}

UI32 CSWF::ReadDoAction(DOACTION *lpdaDoAction)
{
	UI32 ulBitLength=bsInFileStream.GetByteOffset();
		
	memset(lpdaDoAction,0,sizeof(DOACTION));
	memcpy(&lpdaDoAction->rhTagHeader,&rhTagHeader,sizeof(RECORDHEADER));

	/*SpriteID for DOINITACTION*/
	if(lpdaDoAction->rhTagHeader.usTagCode==TAGCODE_DOINITACTION)
		lpdaDoAction->usSpriteID=bsInFileStream.ReadUI16();

	ReadActionArray(&lpdaDoAction->aaActionArray);

	bsInFileStream.SeekNextByte();
	ulBitLength=bsInFileStream.GetByteOffset()-ulBitLength;

	return ulBitLength;
}

UI32 CSWF::WriteDoAction(DOACTION *lpdaDoAction, FILE *pfileSWF)
{
	ACTIONRECORD *lparActionRecord;

	bsOutFileStream.Create(sizeof(DOACTION)+lpdaDoAction->rhTagHeader.ulLength);
	
	/*SpriteID for DOINITACTION*/
	if(lpdaDoAction->rhTagHeader.usTagCode==TAGCODE_DOINITACTION)
		bsOutFileStream.WriteUI16(lpdaDoAction->usSpriteID);

	WriteActionArray(&lpdaDoAction->aaActionArray);

	lpdaDoAction->rhTagHeader.ulLength=bsOutFileStream.GetByteOffset();
	WriteTagHeader(&lpdaDoAction->rhTagHeader,pfileSWF);
	bsOutFileStream.WriteToFile(pfileSWF);

	return bsOutFileStream.GetByteOffset();
}

UI32 CSWF::ReadActionArray(ACTIONARRAY *lpaaActionArray)
{
	UI32 ulByteLength=bsInFileStream.GetByteOffset();

	ACTIONRECORD *lparActionRecord;
	ACTIONRECORD **lprgarpActions=new ACTIONRECORD*[MAX_ACTIONS];

	lpaaActionArray->ulNumActions=0;

	while(bsInFileStream.GetUI8())
	{
		lprgarpActions[lpaaActionArray->ulNumActions]=new ACTIONRECORD;
		lparActionRecord=lprgarpActions[lpaaActionArray->ulNumActions];
		ReadActionRecord(lparActionRecord);
		lpaaActionArray->ulNumActions++;
	}

	bsInFileStream.ReadUI8(); //skip end marker
	lpaaActionArray->lprgarpActions=new ACTIONRECORD*[lpaaActionArray->ulNumActions];
	memcpy(lpaaActionArray->lprgarpActions,lprgarpActions,lpaaActionArray->ulNumActions<<2);
	delete lprgarpActions;

	ulByteLength=bsInFileStream.GetByteOffset()-ulByteLength;
	lpaaActionArray->ulTotalLength=ulByteLength;

	/*write action records to log file*/
	WriteActions(lpaaActionArray,0,pfileActions);

	return ulByteLength;
}

UI32 CSWF::WriteActionArray(ACTIONARRAY *lpaaActionArray)
{
	UI32 ulByteLength=bsOutFileStream.GetByteOffset();

	ACTIONRECORD *lparActionRecord;

	for(UI32 iAction=0; iAction<lpaaActionArray->ulNumActions; iAction++)
	{
		lparActionRecord=lpaaActionArray->lprgarpActions[iAction];
		WriteActionRecord(lparActionRecord);
	}

	bsOutFileStream.WriteUI8(0);
	ulByteLength=bsOutFileStream.GetByteOffset()-ulByteLength;

	return ulByteLength;
}

UI32 CSWF::DeleteActionArray(ACTIONARRAY *lpaaActionArray)
{
	for(UI32 iAction=0; iAction<lpaaActionArray->ulNumActions; iAction++)
	{
		delete lpaaActionArray->lprgarpActions[iAction]->lpvActionData;
		delete lpaaActionArray->lprgarpActions[iAction];
	}
		
	delete lpaaActionArray->lprgarpActions;

	return 0;
}

UI32 CSWF::ReadActionRecord(ACTIONRECORD *lparActionRecord)
{
	UI32 ulByteLength=bsInFileStream.GetByteOffset();

	memset(lparActionRecord,0,sizeof(ACTIONRECORD));

	lparActionRecord->ubActionCode=bsInFileStream.ReadUI8();

	if(lparActionRecord->ubActionCode & 0x80)
	{
		lparActionRecord->usLength=bsInFileStream.ReadUI16();
		lparActionRecord->lpvActionData=new UI8[lparActionRecord->usLength];
		bsInFileStream.ReadBytes(lparActionRecord->lpvActionData,lparActionRecord->usLength);
	}

	bsInFileStream.SeekNextByte();
	ulByteLength=bsInFileStream.GetByteOffset()-ulByteLength;

	return ulByteLength;
}

UI32 CSWF::WriteActionRecord(ACTIONRECORD *lparActionRecord)
{
	UI32 ulByteLength=bsOutFileStream.GetByteOffset();

	bsOutFileStream.WriteUI8(lparActionRecord->ubActionCode);
	
	if(lparActionRecord->ubActionCode & 0x80)
	{
		bsOutFileStream.WriteUI16(lparActionRecord->usLength);
		bsOutFileStream.WriteBytes(lparActionRecord->lpvActionData,lparActionRecord->usLength);
	}

	bsOutFileStream.SeekNextByte();
	ulByteLength=bsOutFileStream.GetByteOffset()-ulByteLength;
	
	return ulByteLength;
}
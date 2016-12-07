#include "stack.h"

UI32 AddQuotes(char *szString)
{
	if(!szString) return 0;

	char *szTemp=new char[strlen(szString)+3];

	sprintf(szTemp,"\"%s\"",szString);
	strcpy(szString,szTemp);
	delete szTemp;
	return 1;
}

UI32 RemoveQuotes(char *szString)
{
	if(!szString) return 0;
	
	if( szString[0]!='\"' || 
		szString[strlen(szString)-1]!='\"') 
		return 0;

	char *szTemp=new char[strlen(szString)];

	sprintf(szTemp,"%s",szString+1);
	szTemp[strlen(szTemp)-1]='\0';
	strcpy(szString,szTemp);\
	delete szTemp;
	return 1;
}

UI32 AddParens(char *szString)
{
	if(!szString) return 0;

	char *szTemp=new char[strlen(szString)+3];

	sprintf(szTemp,"(%s)",szString);
	strcpy(szString,szTemp);
	delete szTemp;
	return 1;
}

UI32 RemoveParens(char *szString)
{
	if(!szString) return 0;
	
	if( szString[0]!='(' || 
		szString[strlen(szString)-1]!=')') 
		return 0;

	char *szTemp=new char[strlen(szString)];

	sprintf(szTemp,"%s",szString+1);
	szTemp[strlen(szTemp)-1]='\0';
	strcpy(szString,szTemp);
	delete szTemp;
	return 1;
}

UI32 Negate(char *szString)
{
	if(!szString) return 0;

	char *szTemp=new char[strlen(szString)+5];

	if(szString[0]!='!') sprintf(szTemp,"!%s",szString);
	else sprintf(szTemp,"%s",szString+1);
	strcpy(szString,szTemp);
	delete szTemp;
	return 1;
}

double IEEE32TODOUBLE(UI32 ulFloat)
{
	SI8 sbSign;
	SI16 ssExp;
	double sdMult, sdBase, sdDouble;

	sbSign=(ulFloat & 0x80000000? -1:1);
	ssExp=((ulFloat & 0x7F800000)>>23)-127;
	sdBase=(ulFloat & 0x007FFFFF)<<8;
	sdBase=1+sdBase/0x80000000;
	
	/*special values*/
	if(sdBase==1 && ssExp==-127) return 0; /*0*/
	
	/*ssExp>=0 sdMult=2^sbExp; sbExp<0 sdMult=1/(2^sbExp)*/
	sdMult=1<<ABS(ssExp);
	if(ssExp<0) sdMult=1/sdMult;
	
	return sdDouble=sbSign*sdMult*sdBase;
}

double IEEE64TODOUBLE(UI32 ulHigh, UI32 ulLow)
{
	SI8 sbSign;
	SI16 ssExp;
	double sdMult, sdBase, sdDouble;

	sbSign=(ulHigh & 0x80000000? -1:1);
	ssExp=((ulHigh & 0x7FF00000)>>20)-1023;
	sdBase=(ulHigh & 0x000FFFFF)<<11 | (ulLow & 0xFFFFFFFF)>>21;
	sdBase=1+sdBase/0x80000000;

	/*special values*/
	if(sdBase==1 && ssExp==-1023) return 0; /*0*/
	
	/*ssExp>=0 sdMult=2^sbExp; sbExp<0 sdMult=1/(2^sbExp)*/
	sdMult=1<<ABS(ssExp);
	if(ssExp<0) sdMult=1/sdMult;
	
	return sdDouble=sbSign*sdMult*sdBase;
}

UI32 CStack::GetPropertyName(UI8 ubIndex, char *szName)
{
	switch(ubIndex)
	{
		case _x:			strcpy(szName,"_x"); break;
		case _y:			strcpy(szName,"_y"); break;
		case _xscale:		strcpy(szName,"_xscale"); break;
		case _yscale:		strcpy(szName,"_yscale"); break;
		case _currentframe: strcpy(szName,"_currentframe"); break;
		case _totalframes:	strcpy(szName,"_totalframes"); break;
		case _alpha:		strcpy(szName,"_alpha"); break;
		case _visible:		strcpy(szName,"_visible"); break;
		case _width:		strcpy(szName,"_width"); break;
		case _height:		strcpy(szName,"_height"); break;
		case _rotation:		strcpy(szName,"_rotation"); break;
		case _target:		strcpy(szName,"_target"); break;
		case _framesloaded: strcpy(szName,"_framesloaded"); break;
		case _name:			strcpy(szName,"_name"); break;
		case _droptarget:	strcpy(szName,"_droptarget"); break;
		case _url:			strcpy(szName,"_url"); break;
		case _highquality:	strcpy(szName,"_highquality"); break;
		case _focusrect:	strcpy(szName,"_focusrect"); break;
		case _soundbuftime: strcpy(szName,"_soundbuftime"); break;
		case _quality:		strcpy(szName,"_quality"); break;
		case _xmouse:		strcpy(szName,"_xmouse"); break;
		case _ymouse:		strcpy(szName,"_ymouse"); break;
		default: return 0;
	}

	return 1;
}

UI32 CStack::ConvertToString(STACK_ENTRY *lpseStackEntry, UI8 fProperty)
{
	double sdDouble;

	switch(lpseStackEntry->ubType)
	{
		case STACK_STRING:
			if(fProperty)
				GetPropertyName(atoi(lpseStackEntry->szString),lpseStackEntry->szString);
			break;

		case STACK_FLOAT:
			sdDouble=IEEE32TODOUBLE(lpseStackEntry->ulDoubleHigh);
			if(fProperty) GetPropertyName(sdDouble,lpseStackEntry->szString);
			else sprintf(lpseStackEntry->szString,"%.0f",sdDouble);
			break;

		case STACK_NULL:
			sprintf(lpseStackEntry->szString,"NULL");
			break;

		case STACK_UNDEF:
			sprintf(lpseStackEntry->szString,"UNDEFINED");
			break;

		case STACK_REG:
			strcpy(lpseStackEntry->szString,lprgseRegisters[lpseStackEntry->ubRegister].szString);
			//strcat(lpseStackEntry->szString," //register");
			break;

		case STACK_BOOL:
			if(lpseStackEntry->ubBoolean)
				sprintf(lpseStackEntry->szString,"true");
			else sprintf(lpseStackEntry->szString,"false");
			break;

		case STACK_DOUBLE:
			sdDouble=IEEE64TODOUBLE(lpseStackEntry->ulDoubleHigh,lpseStackEntry->ulDoubleLow);
			if(fProperty) GetPropertyName(sdDouble,lpseStackEntry->szString);
			else 
			{
				if(sdDouble==0) sprintf(lpseStackEntry->szString,"%.0f",sdDouble);
				else sprintf(lpseStackEntry->szString,"%.4f",sdDouble);
			}
				
			break;

		case STACK_INT:
			if(fProperty)
				GetPropertyName(lpseStackEntry->slInteger,lpseStackEntry->szString);
			else sprintf(lpseStackEntry->szString,"%i",lpseStackEntry->slInteger);
			break;

		case STACK_CONSTANT:
			sprintf(lpseStackEntry->szString,"%s",
					cpoolConstantPool.Get(lpseStackEntry->usConstant));
			break;
	}

	lpseStackEntry->ubType=STACK_STRING;

	if(!strcmp(lpseStackEntry->szString,"") || !strcmp(lpseStackEntry->szString," "))
		AddQuotes(lpseStackEntry->szString);

	return 0;
}

UI32 CStack::Push(STACK_ENTRY *lpseSource)
{
	STACK_ENTRY *lpseNew;

	if(!lpseSource) return 0;

	lpseNew=new STACK_ENTRY;
	memcpy(lpseNew,lpseSource,sizeof(STACK_ENTRY));
		
	if(!lpseBottom)
	{
		lpseBottom=lpseNew;
		lpseBottom->lpsePrev=NULL;
	}

	else
	{
		lpseNew->lpsePrev=lpseTop;
	}

	lpseTop=lpseNew;
	lpseTop->lpseNext=NULL;

	ConvertToString(lpseTop);

	return 1;
}

UI32 CStack::Pop(STACK_ENTRY *lpseDest)
{
	STACK_ENTRY *lpsePopped;

	if(!lpseBottom) 
	{
		if(lpseDest) 
			strcpy(lpseDest->szString,"???");
		return 0;
	}

	if(lpseDest)
		memcpy(lpseDest,lpseTop,sizeof(STACK_ENTRY));

	lpsePopped=lpseTop;
	lpseTop=lpseTop->lpsePrev;
	delete lpsePopped;
	
	if(!lpseTop) lpseBottom=NULL;

	return 1;
}

UI32 CStack::StoreRegister(UI8 ubRegister, STACK_ENTRY* lpseStackEntry)
{
	memcpy(&lprgseRegisters[ubRegister],lpseStackEntry,sizeof(STACK_ENTRY));
	ConvertToString(&lprgseRegisters[ubRegister]);
	return 0;
}

UI32 CStack::GetRegister(UI8 ubRegister, STACK_ENTRY* lpseStackEntry)
{
	memcpy(lpseStackEntry,&lprgseRegisters[ubRegister],sizeof(STACK_ENTRY));
	return 0;
}
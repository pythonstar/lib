#ifndef _POOL_H_
#define _POOL_H_

#include "swf.h"

class CPool
{
public:
	CPool()
	{
		lprgszConstants=NULL;
		ulSize=ulNumConstants=0;
	}

	CPool(UI32 ulCount)
	{
		lprgszConstants=NULL;
		SetSize(ulCount);
	}

	~CPool() {Clear();}

	UI32 SetSize(UI32 ulCount)
	{
		if(lprgszConstants) return 0;

		lprgszConstants=new char*[ulCount];
		ulSize=ulCount;
		ulNumConstants=0;
		return ulSize;
	}

	UI32 Add(char *szConstant)
	{
		if(ulNumConstants>=ulSize) return 0;
		lprgszConstants[ulNumConstants]=new char[strlen(szConstant)+1];
		strcpy(lprgszConstants[ulNumConstants],szConstant);
		return ++ulNumConstants;
	}

	char *Get(UI32 ulConstant) 
	{
		if(ulConstant>=ulNumConstants) return NULL;
		return lprgszConstants[ulConstant];
	}

	UI32 Clear()
	{
		for(UI32 iConstant=0; iConstant<ulNumConstants; iConstant++)
			delete lprgszConstants[iConstant];
		
		if(lprgszConstants) 
			delete lprgszConstants;

		lprgszConstants=NULL;

		return ulSize=ulNumConstants=0;
	}

private:
	char **lprgszConstants;
	UI32 ulSize, ulNumConstants;
};

#endif
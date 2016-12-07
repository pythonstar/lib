#include "displaylist.h"
//when moving and has a new character*/
UI32 CDisplayList::Place(void *lpvPlace)
{
	PLACEOBJECT *lppoPlace;
	PLACEOBJECT2 *lppo2Place;
	LAYER *lplSearch;
	LAYER *lplNewLayer=new LAYER;
	memset(lplNewLayer,0,sizeof(LAYER));

	switch(((UNKNOWNTAG*)lpvPlace)->rhTagHeader.usTagCode)
	{
		case TAGCODE_PLACEOBJECT:	
			lppoPlace=(PLACEOBJECT*)lpvPlace;
			
			lplNewLayer->po2Place.usCharacterID=lppoPlace->usCharacterID;
			lplNewLayer->po2Place.fPlaceFlagHasCharacter=true;
			lplNewLayer->po2Place.usDepth=lppoPlace->usDepth;
			memcpy(&lplNewLayer->po2Place.mMatrix,&lppoPlace->mMatrix,sizeof(MATRIX));
			lplNewLayer->po2Place.fPlaceFlagHasMatrix=true;
			memcpy(&lplNewLayer->po2Place.cxfwaColorTransform,&lppoPlace->cxfColorTransform,sizeof(CXFORM));
			lplNewLayer->po2Place.fPlaceFlagHasColorTransform=true;
			break;

		case TAGCODE_PLACEOBJECT2:
			lppo2Place=(PLACEOBJECT2*)lpvPlace;
			memcpy(&lplNewLayer->po2Place,lppo2Place,sizeof(PLACEOBJECT2));
			break;

		default:
			delete lplNewLayer;
			return 0;
	}

	if(!lplHead) 
	{
		lplHead=lplNewLayer;
		lplHead->lplNext=lplHead->lplPrev=NULL;
		lplTail=lplHead;
	}

	else /*insertion sort*/
	{
		lplSearch=lplTail;

		if(lplNewLayer->po2Place.usDepth<lplHead->po2Place.usDepth)
		{
			lplHead->lplPrev=lplNewLayer;
			lplNewLayer->lplPrev=NULL;
			lplNewLayer->lplNext=lplHead;
			lplHead=lplNewLayer;
		}

		else while(lplSearch)
		{
			if(lplNewLayer->po2Place.usDepth<lplSearch->po2Place.usDepth)
				lplSearch=lplSearch->lplPrev;

			/*same depth, move or remove and place new*/
			else if(lplNewLayer->po2Place.usDepth==lplSearch->po2Place.usDepth)
			{
				if(lplNewLayer->po2Place.fPlaceFlagMove)
				{
					if(lplNewLayer->po2Place.fPlaceFlagHasClipActions)
					{
						memcpy(&lplSearch->po2Place.caClipActions,&lplNewLayer->po2Place.caClipActions,sizeof(CLIPACTIONS));
						lplSearch->po2Place.fPlaceFlagHasClipActions=true;	
					}

					if(lplNewLayer->po2Place.fPlaceFlagHasClipDepth)
					{
						lplSearch->po2Place.usClipDepth=lplNewLayer->po2Place.usClipDepth;
						lplSearch->po2Place.fPlaceFlagHasClipDepth=true;
					}
		
					if(lplNewLayer->po2Place.fPlaceFlagHasName)
					{
						lplSearch->po2Place.szName=lplNewLayer->po2Place.szName;
						lplSearch->po2Place.fPlaceFlagHasName=true;
					}

					if(lplNewLayer->po2Place.fPlaceFlagHasRatio)
					{
						lplSearch->po2Place.usRatio=lplNewLayer->po2Place.usRatio;
						lplSearch->po2Place.fPlaceFlagHasRatio=true;
					}

					if(lplNewLayer->po2Place.fPlaceFlagHasColorTransform)
					{
						memcpy(&lplSearch->po2Place.cxfwaColorTransform,&lplNewLayer->po2Place.cxfwaColorTransform,sizeof(CXFORM));
						lplSearch->po2Place.fPlaceFlagHasColorTransform=true;
					}

					if(lplNewLayer->po2Place.fPlaceFlagHasMatrix)
					{
						memcpy(&lplSearch->po2Place.mMatrix,&lplNewLayer->po2Place.mMatrix,sizeof(MATRIX));
						lplSearch->po2Place.fPlaceFlagHasMatrix=true;
					}

					if(lplNewLayer->po2Place.fPlaceFlagHasCharacter)
					{
						lplSearch->po2Place.usCharacterID=lplNewLayer->po2Place.usCharacterID;
						lplSearch->po2Place.fPlaceFlagHasCharacter=true;
					}
				}

				else memcpy(&lplSearch->po2Place,&lplNewLayer->po2Place,sizeof(PLACEOBJECT2));

				delete lplNewLayer;
				lplSearch=NULL;
			}

			else /*insert*/ 
			{
				lplNewLayer->lplPrev=lplSearch;
				lplNewLayer->lplNext=lplSearch->lplNext;
				
				if(lplSearch->lplNext)
					lplSearch->lplNext->lplPrev=lplNewLayer;

				lplSearch->lplNext=lplNewLayer;

				if(lplSearch==lplTail)
					lplTail=lplNewLayer;
				
				lplSearch=NULL;
			}
		}
	}

	return 0;
}

UI32 CDisplayList::Remove(void *lpvRemove)
{
	LAYER *lplSearch;
	UI16 usDepth;

	switch(((UNKNOWNTAG*)lpvRemove)->rhTagHeader.usTagCode)
	{
		case TAGCODE_REMOVEOBJECT:	
			usDepth=((REMOVEOBJECT*)lpvRemove)->usDepth;
			break;

		case TAGCODE_REMOVEOBJECT2:
			usDepth=((REMOVEOBJECT2*)lpvRemove)->usDepth;
			break;
	}

	lplSearch=lplHead;

	while(lplSearch)
	{
		if(lplSearch->po2Place.usDepth==usDepth)
		{
			/*cut out the layer*/
			if(lplSearch->lplPrev) lplSearch->lplPrev->lplNext=lplSearch->lplNext;
			else lplHead=lplSearch->lplNext;

			if(lplSearch->lplNext) lplSearch->lplNext->lplPrev=lplSearch->lplPrev;
			else lplTail=lplSearch->lplPrev;

			lplSearch=NULL;
		}

		else lplSearch=lplSearch->lplNext;
	}

	return 0;
}

UI32 CDisplayList::GetFirstLayer(LAYER *lplDest)
{
	if(!lplHead) return 0;
	memcpy(lplDest,lplHead,sizeof(LAYER));
	return 1;
}

UI32 CDisplayList::GetNextLayer(LAYER *lplDest)
{
	if(!lplRead) lplRead=lplHead;
	else lplRead=lplRead->lplNext;
	if(!lplRead) return 0;
	memcpy(lplDest,lplRead,sizeof(LAYER));
	return 1;
}

void CDisplayList::Clear()
{
	while(lplHead)
	{
		lplTail=lplHead->lplNext;
		delete lplHead;
		lplHead=lplTail;
	}

	lplHead=lplTail=lplRead=NULL;
}
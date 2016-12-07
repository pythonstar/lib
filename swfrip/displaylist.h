#ifndef _DISPLAYLIST_H_
#define _DISPLAYLIST_H_

#include "swf.h"

struct LAYER
{
	PLACEOBJECT2 po2Place;
	LAYER *lplPrev;
	LAYER *lplNext;
};

class CDisplayList
{
public:
	CDisplayList() {lplHead=lplTail=lplRead=NULL;}
	~CDisplayList() {Clear();}

	UI32 Place(void*);
	UI32 Remove(void*);

	UI32 GetFirstLayer(LAYER*);
	UI32 GetNextLayer(LAYER*);

	void Clear();

private:
	LAYER *lplHead, *lplTail, *lplRead;
};



#endif
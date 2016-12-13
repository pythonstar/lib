#ifndef _STACK_H_
#define _STACK_H_

#include "swf.h"
#include "pool.h"

#define MAX_STRING	2048

/*property index*/

#define _x				0
#define _y				1
#define _xscale			2
#define _yscale			3
#define _currentframe	4
#define _totalframes	5
#define _alpha			6
#define _visible		7
#define _width			8
#define _height			9
#define _rotation		10
#define _target			11
#define _framesloaded	12
#define _name			13
#define _droptarget		14
#define _url			15
#define _highquality	16
#define _focusrect		17
#define _soundbuftime	18
#define _quality		19
#define _xmouse			20
#define _ymouse			21

#define STACK_STRING	0
#define STACK_FLOAT		1
#define STACK_NULL		2
#define STACK_UNDEF		3
#define STACK_REG		4
#define STACK_BOOL		5
#define STACK_DOUBLE	6
#define STACK_INT		7
#define STACK_CONSTANT	8
		

struct STACK_ENTRY
{
	UI8 ubType;		
	char szString[4096];	//0
	UI32 ulDoubleHigh;		//1
	UI32 ulDoubleLow;		//6	
	UI8 ubRegister;			//4
	UI8 ubBoolean;			//5
	SI32 slInteger;			//7
	UI16 usConstant;		//8/9

	STACK_ENTRY *lpsePrev;
	STACK_ENTRY *lpseNext;
};

UI32 SetString(STACK_ENTRY *lpseStackEntry, char*szString);
UI32 AddQuotes(char *szString);
UI32 RemoveQuotes(char *szString);
UI32 AddParens(char *szString);
UI32 RemoveParens(char *szString);
UI32 Negate(char *szString);
double IEEE32TODOUBLE(UI32 ulFloat);
double IEEE64TODOUBLE(UI32 ulHigh, UI32 ulLow);

class CStack
{
public:
	CStack() {lpseBottom=lpseTop=NULL; memset(lprgseRegisters,0,256*sizeof(STACK_ENTRY));}
	~CStack() {Clear();}

	CPool cpoolConstantPool;

	UI32 Push(STACK_ENTRY*);
	UI32 Pop(STACK_ENTRY*);
	UI32 StoreRegister(UI8,STACK_ENTRY*);
	UI32 GetRegister(UI8,STACK_ENTRY*);

	UI32 ConvertToString(STACK_ENTRY *lpseStackEntry, UI8 fProperty=false);
	UI32 GetPropertyName(UI8 ubIndex, char *szName);
	
	void Clear() {while(Pop(NULL)) true;}
	
private:
	STACK_ENTRY *lpseBottom, *lpseTop;
	STACK_ENTRY lprgseRegisters[256];
};

#endif
/* idea.h */ 
 

/*
µ÷ÓÃÊ¾Àý:

unsigned int userkey[8]={1,2,3,4,5,6,7,8}; 
char szDataIn[17]="1234567890123456";
char szDataOut1[17]={0};
char szDataOut2[17]={0};

Idea::Encode(szDataIn,szDataOut1,16,userkey);
Idea::Decode(szDataOut1,szDataOut2,16,userkey);
*/


#pragma once

namespace Idea
{
	//------------------------------------------------------------------------
	#define IDEAKEYSIZE 16 
	#define IDEABLOCKSIZE 8 
	#define word16 unsigned int 
	#define word32 unsigned long int 
	#define ROUNDS    8 
	#define KEYLEN    (6*ROUNDS+4) 
	#define low16(x) ((x) & 0xffff) 

	typedef unsigned int uint16; 
	typedef word16 IDEAkey[KEYLEN]; 

	#define MUL(x,y) (x=mul(low16(x),y)) 
	//------------------------------------------------------------------------

	//------------------------------------------------------------------------
	void Encode(char*in ,char*out,int nSize,word16 *key);
	void Decode(char*in ,char*out,int nSize,word16 *key);
	//------------------------------------------------------------------------
}



#include "stdafx.h"
#include "TEA.h"
using namespace Star::Encrypt;

/*能够正常加解密,调用示例:

char szBuff[]="123456789123456789123456789123456789";
char szOut1[100]={0};
char szOut2[100]={0};

Encrypt::Tea::Encode(szBuff,szOut1,sizeof(szBuff));
Encrypt::Tea::Decode(szOut1,szOut2,sizeof(szBuff));
*/


//每次操作8字节
void _Encode(unsigned long *v, unsigned long *k) 
{
	unsigned long y=v[0], z=v[1], sum=0, i;         /* set up */
	unsigned long delta=0x9e3779b9;                 /* a key schedule constant */
	unsigned long a=k[0], b=k[1], c=a, d=b;   /* cache key */
	for (i=0; i < 32; i++) {                        /* basic cycle start */
		sum += delta;
		y += ((z<<4) + a) ^ (z + sum) ^ ((z>>5) + b);
		z += ((y<<4) + c) ^ (y + sum) ^ ((y>>5) + d);/* end cycle */
	}
	k[0]=y;
	k[1]=z;
}

//每次操作8字节
void _Decode(unsigned long *v, unsigned long *k) 
{
	unsigned long y=v[0], z=v[1], sum=0xC6EF3720, i; /* set up */
	unsigned long delta=0x9e3779b9;                  /* a key schedule constant */
	unsigned long a=k[0], b=k[1], c=a, d=b;    /* cache key */
	for(i=0; i<32; i++) {                            /* basic cycle start */
		z -= ((y<<4) + c) ^ (y + sum) ^ ((y>>5) + d);
		y -= ((z<<4) + a) ^ (z + sum) ^ ((z>>5) + b);
		sum -= delta;                                /* end cycle */
	}
	k[0]=y;
	k[1]=z;
}

void Tea::Encode(char*in ,char*out,int nSize)
{
	int nMulti=nSize/(sizeof(unsigned long)*2);
	int nRest=nSize%(sizeof(unsigned long)*2);
	//每次操作8字节
	for (int i=0;i<nMulti;i++ )
	{
		_Encode( (unsigned long *)(&in[i*sizeof(unsigned long)*2]), (unsigned long *)(&out[i*sizeof(unsigned long)*2]) );
	}
	char*pIn=&in[nMulti*(sizeof(unsigned long)*2)];
	char*pOut=&out[nMulti*(sizeof(unsigned long)*2)];
	for (int i=0;i<nRest;i++)
	{
		pOut[i]=pIn[i]^0x9e;
	}
}

void Tea::Decode(char*in ,char*out,int nSize)
{
	int nMulti=nSize/(sizeof(unsigned long)*2);
	int nRest=nSize%(sizeof(unsigned long)*2);
	//每次操作8字节
	for (int i=0;i<nMulti;i++ )
	{
		_Decode( (unsigned long *)(&in[i*sizeof(unsigned long)*2]), (unsigned long *)(&out[i*sizeof(unsigned long)*2]) );
	}
	char*pIn=&in[nMulti*(sizeof(unsigned long)*2)];
	char*pOut=&out[nMulti*(sizeof(unsigned long)*2)];
	for (int i=0;i<nRest;i++)
	{
		pOut[i]=pIn[i]^0x9e;
	}
}
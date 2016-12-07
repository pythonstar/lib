#ifndef BITSTREAM_H
#define BITSTREAM_H

#include <stdio.h>
#include <string.h>

/*Type Definitions*/

#define UI8		unsigned char
#define UI16	unsigned short
#define UI32	unsigned long
#define SI8		signed char
#define SI16	signed short
#define SI32	signed long
#define FP32	unsigned long
#define FP16	unsigned short
#define SFP32	unsigned long
#define SFP16	unsigned short

inline SI8 SGN(SI32 slValue)
{
	if(!slValue) return 0;
	else if(slValue>0) return 1;
	else return -1;
}

inline UI8 BITSFORUNSIGNEDVALUE(UI32 ulValue)
{
	UI8 ubBits=31;

	if(!ulValue) return 0;
	
	while(!(ulValue & 0x01<<ubBits)) ubBits--;
	
	return ubBits+1;
}

inline UI8 BITSFORSIGNEDVALUE(SI32 slValue)
{
	UI8 ubBits=31;

	if(!slValue) return 0;
	if(slValue==-1) return 2;

	if(!(slValue & 0x80000000)) return BITSFORUNSIGNEDVALUE(slValue)+1;
	
	while(slValue & 0x01<<ubBits) ubBits--;

	return ubBits+2;
}

inline UI32 TO32BITNEGATIVE(UI32 slValue, UI8 ubBits)
{
	UI32 ulValueMask=1;
	if(ubBits>1 && (slValue & 0x00000001<<(ubBits-1)))
	{
		for(UI32 iCurBit=0; iCurBit<ubBits; iCurBit++)
			ulValueMask |= 1<<iCurBit;
		
		slValue=~slValue & ulValueMask;
		slValue++;
		slValue*=-1;
	}

	return slValue;
}


/*Macros*/

#define ABS(X)					((X)*SGN((X)))
#define MIN(X,Y)				((X)<(Y)? (X):(Y))
#define MAX(X,Y)				((X)>(Y)? (X):(Y))
#define ROUND(X)				((X-UI32(X))>=.5? X+1:X) 
#define ROUNDTOUI32(X)			(UI32(ROUND(X)))
#define ROUNDUP(X)				UI32(X>(UI32)X? ((UI32)(X+1)):X)
#define ROUNDTOUI8(X)			(ROUNDTOINT(X)>255? 255:ROUNDTOINT(X))

#define BYTESFORSTREAM(B)		ROUNDUP(float(B)/8)

inline float FP32TOFLOAT(FP32 lfpFloat)			
{
	if(lfpFloat & 0x80000000)
		return -1*(float(~lfpFloat+1)/0x10000);		

	else return float(lfpFloat)/0x10000;

}

inline FP32 FLOATTOFP32(float sfFloat)			
{
	return sfFloat*0x10000;
}

inline float FP16TOFLOAT(FP16 lfpFloat)			
{
	if(lfpFloat & 0x8000)
		return -1*(float(~lfpFloat+1)/0x100);		

	else return float(lfpFloat)/0x100;

}

inline FP16 FLOATTOFP16(float sfFloat)			
{
	return sfFloat*0x100;
}


class CBitstream
{
public:
	CBitstream() {prgbData=NULL; Clear();}
	CBitstream(UI32 nLength, void *pvData=NULL) {prgbData=NULL; Create(nLength,pvData);}
	~CBitstream() {Clear();}
	UI32 Create(UI32 nLength, void *pvData=NULL);
	void Clear();
	UI32 ReadBytes(UI8*,UI32); 
	UI32 WriteBytes(UI8*,UI32);
	
	/*Regular Values*/

	UI8  GetUI8()				{SeekNextByte(); return *(UI8*)(prgbData+BYTESFORSTREAM(iBitOffset));}
	SI8  GetSI8()				{SeekNextByte(); return *(SI8*)(prgbData+BYTESFORSTREAM(iBitOffset));}
	UI16 GetUI16()				{SeekNextByte(); return *(UI16*)(prgbData+BYTESFORSTREAM(iBitOffset));}
	SI16 GetSI16()				{SeekNextByte(); return *(SI16*)(prgbData+BYTESFORSTREAM(iBitOffset));}
	UI32 GetUI32()				{SeekNextByte(); return *(UI32*)(prgbData+BYTESFORSTREAM(iBitOffset));}
	SI32 GetSI32()				{SeekNextByte(); return *(SI32*)(prgbData+BYTESFORSTREAM(iBitOffset));}
	UI32 PutUI8(UI8 ubValue)	{SeekNextByte(); *(UI8*)(prgbData+BYTESFORSTREAM(iBitOffset))=ubValue; return iBitOffset/8;}
	UI32 PutSI8(SI8 sbValue)	{SeekNextByte(); *(SI8*)(prgbData+BYTESFORSTREAM(iBitOffset))=sbValue; return iBitOffset/8;}
	UI32 PutUI16(UI16 usValue)  {SeekNextByte(); *(UI16*)(prgbData+BYTESFORSTREAM(iBitOffset))=usValue; return iBitOffset/8;}
	UI32 PutSI16(SI16 ssValue)  {SeekNextByte(); *(SI16*)(prgbData+BYTESFORSTREAM(iBitOffset))=ssValue; return iBitOffset/8;}
	UI32 PutUI32(UI32 ulValue)  {SeekNextByte(); *(UI32*)(prgbData+BYTESFORSTREAM(iBitOffset))=ulValue; return iBitOffset/8;}
	UI32 PutSI32(SI32 slValue)  {SeekNextByte(); *(SI32*)(prgbData+BYTESFORSTREAM(iBitOffset))=slValue; return iBitOffset/8;}

	UI8  ReadUI8()				{UI8  ubRet=GetUI8();  iBitOffset+=8*sizeof(UI8);  return ubRet;}
	SI8  ReadSI8()				{SI8  sbRet=GetSI8();  iBitOffset+=8*sizeof(SI8);  return sbRet;}
	UI16 ReadUI16()				{UI16 usRet=GetUI16(); iBitOffset+=8*sizeof(UI16); return usRet;}
	SI16 ReadSI16()				{SI16 ssRet=GetSI16(); iBitOffset+=8*sizeof(SI16); return ssRet;}
	UI32 ReadUI32()				{UI32 ulRet=GetUI32(); iBitOffset+=8*sizeof(UI32); return ulRet;}
	SI32 ReadSI32()				{SI32 slRet=GetSI32(); iBitOffset+=8*sizeof(SI32); return slRet;}
	UI32 WriteUI8(UI8 ubValue)	{PutUI8(ubValue);  iBitOffset+=8*sizeof(UI8);  return iBitOffset/8;}
	UI32 WriteSI8(SI8 sbValue)	{PutSI8(sbValue);  iBitOffset+=8*sizeof(SI8);  return iBitOffset/8;}
	UI32 WriteUI16(UI16 usValue){PutUI16(usValue); iBitOffset+=8*sizeof(UI16); return iBitOffset/8;}
	UI32 WriteSI16(SI16 ssValue){PutSI16(ssValue); iBitOffset+=8*sizeof(SI16); return iBitOffset/8;}
	UI32 WriteUI32(UI32 ulValue){PutUI32(ulValue); iBitOffset+=8*sizeof(UI32); return iBitOffset/8;}
	UI32 WriteSI32(SI32 slValue){PutSI32(slValue); iBitOffset+=8*sizeof(SI32); return iBitOffset/8;}


	/*Bit Values*/

	UI32 GetUB(UI8);
	SI32 GetSB(UI8);
	FP32 GetFB(UI8);
	UI32 PutUB(UI32,UI8);
	UI32 PutSB(SI32,UI8);
	UI32 PutFB(FP32,UI8);
	UI32 ReadUB(UI8 ubBits)	{UI32 ulValue=GetUB(ubBits); iBitOffset+=ubBits; return ulValue;}
	SI32 ReadSB(UI8 ubBits) {UI32 ulValue=GetSB(ubBits); iBitOffset+=ubBits; return ulValue;}
	FP32 ReadFB(UI8 ubBits) {FP32 lfpValue=GetFB(ubBits); iBitOffset+=ubBits; return lfpValue;}
	UI32 WriteUB(UI32 ulValue, UI8 ubBits) {PutUB(ulValue,ubBits); return iBitOffset+=ubBits;}
	UI32 WriteSB(SI32 slValue, UI8 ubBits) {PutSB(slValue,ubBits); return iBitOffset+=ubBits;}
	UI32 WriteFB(FP32 slValue, UI8 ubBits) {PutFB(slValue,ubBits); return iBitOffset+=ubBits;}
	

	/*String Values*/
	UI32 ReadString(char *&szString, UI32 ulLength=0);
	UI32 WriteString(const char*, UI32 ulLength=0);


	UI32 GetData(UI8 *&prgbCopy)  
	{
		UI32 nSize=BYTESFORSTREAM(iBitOffset);
		prgbCopy=new UI8[nSize]; 
		memcpy(prgbCopy,prgbData,nSize);
		return nSize;
	}

	UI8 *GetAddress()		{return prgbData+iBitOffset/8;}
	UI32 GetBitOffset()		{return iBitOffset;}
	UI32 GetByteOffset()	{return iBitOffset/8;}
	UI32 Seek(UI32 ulOffset) {return iBitOffset=ulOffset*8;}
	UI32 SeekNextByte()		{while(iBitOffset%8) iBitOffset++; return iBitOffset/8;}
	UI32 SeekBytes(SI32);
	UI32 SeekBits(SI32);
	void Rewind() {iBitOffset=0;}
	UI32 WriteToFile(FILE *pfileFile) {return fwrite(prgbData,1,BYTESFORSTREAM(iBitOffset),pfileFile);}
	UI32 WriteBufferToFile(FILE *pfileFile) {return fwrite(prgbData,1,ulDataLength,pfileFile);}

private:
	UI8 *prgbData;					//Buffer for file data
	UI32 ulDataLength;				//Including FrameHeader, Not including FileHeader, 
	UI32 iBitOffset;
};

#endif
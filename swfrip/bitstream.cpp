#include "bitstream.h"

UI32 CBitstream::Create(UI32 nLength, void *pvData)
{
	Clear();

	prgbData=new UI8[nLength];

	if(pvData) memcpy(prgbData,pvData,nLength);
	else memset(prgbData,0,nLength);

	ulDataLength=nLength;
	iBitOffset=0;
		
	return nLength;
}

void CBitstream::Clear()
{
	if(prgbData) delete prgbData; 
	prgbData=NULL; 
	iBitOffset=ulDataLength=0;
}

UI32 CBitstream::SeekBits(SI32 slBits)
{
	iBitOffset+=slBits; 
	
	/*Check if past buffer end*/
	if(iBitOffset>=ulDataLength*8) 
		iBitOffset=ulDataLength*8-1; 
	
	return iBitOffset;
}

UI32 CBitstream::SeekBytes(SI32 slBytes)
{
	SeekNextByte();

	while(slBytes)
	{
		iBitOffset+=8*SGN(slBytes);
		slBytes-=SGN(slBytes);
	}

	return iBitOffset;
}

UI32 CBitstream::ReadBytes(UI8 *prgbBuffer, UI32 ulBytes)
{
	SeekNextByte(); 
	memcpy(prgbBuffer,prgbData+iBitOffset/8,ulBytes); 
	iBitOffset+=ulBytes*8; 
	return iBitOffset/8;
}

UI32 CBitstream::WriteBytes(UI8 *prgbBuffer, UI32 ulBytes)
{
	SeekNextByte(); 
	memcpy(prgbData+iBitOffset/8,prgbBuffer,ulBytes); 
	iBitOffset+=ulBytes*8; 
	return iBitOffset/8;
}

SI32 CBitstream::GetSB(UI8 ubBits)
{
	return TO32BITNEGATIVE(GetUB(ubBits),ubBits);
}

UI32 CBitstream::GetUB(UI8 ubBits)
{
	if(!ubBits) return 0;

	UI32 iByte=(iBitOffset+ubBits-1)/8;
	UI32 iBit=(iBitOffset+ubBits-1)%8;
	UI32 ulValueMask=1;
	UI32 ulValue=0;

	for(UI32 iCurBit=0; iCurBit<ubBits; iCurBit++)
	{
		/*If the current stream bit is on, set the coresponding value bit*/
		if(prgbData[iByte] & (0x80>>iBit))
			ulValue |= ulValueMask;
		

		if(iBit==0)
		{
			iBit=7;
			iByte--;
		}
		else iBit--;

		ulValueMask<<=1;		
	}

	return ulValue;
}

FP32 CBitstream::GetFB(UI8 ubBits)
{
	FP32 lfpValue=0;

	lfpValue=GetUB(ubBits);

	if(lfpValue & 1<<ubBits-1)
		lfpValue |= 0xFFFFFFFF<<ubBits;

	return lfpValue;
}

UI32 CBitstream::PutFB(FP32 lfpValue, UI8 ubBits)
{
	PutUB(lfpValue,ubBits);

	return iBitOffset;
}

UI32 CBitstream::PutSB(SI32 slValue, UI8 ubBits)
{
	PutUB(slValue,ubBits);

	return iBitOffset;
}

UI32 CBitstream::PutUB(UI32 ulValue, UI8 ubBits)
{
	if(!ubBits) return 0;
	
	UI32 iByte=(iBitOffset+ubBits-1)/8;
	UI32 iBit=(iBitOffset+ubBits-1)%8;
	UI32 ulValueMask=1;

	for(UI32 iCurBit=0; iCurBit<ubBits; iCurBit++)
	{
		/*If the current value bit is on, set the coresponding stream bit*/
		if(ulValue & ulValueMask)
		prgbData[iByte] |= 0x80>>iBit; 

		if(iBit==0)
		{
			iBit=7;
			iByte--;
		}
		else iBit--;

		ulValueMask<<=1;		
	}

	return iBitOffset;
}

UI32 CBitstream::ReadString(char *&szString, UI32 ulLength)
{
	SeekNextByte();
	if(!ulLength) ulLength=strlen((const char*)prgbData+iBitOffset/8)+1;
	szString=new char[ulLength];
	memcpy(szString,prgbData+iBitOffset/8,ulLength);
	return SeekBytes(ulLength);
}

UI32 CBitstream::WriteString(const char *szString, UI32 ulLength)
{
	if(!szString) return SeekBytes(1);

	if(!ulLength) ulLength=strlen(szString)+1;
	memcpy(prgbData+iBitOffset/8,szString,ulLength);	
	return SeekBytes(ulLength);
}
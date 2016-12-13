#include "swf.h"

/*Headers*/

UI32 CSWF::ReadFrameHeader(SWFFRAMEHEADER *lpfhFrameHeader)
{
	UI32 ulHeaderSize=bsInFileStream.GetByteOffset();

	ReadRect(&fhFrameHeader.srFrameSize);
	lpfhFrameHeader->sfpFrameRate=bsInFileStream.ReadUI16();
	lpfhFrameHeader->usFrameCount=bsInFileStream.ReadUI16();

	bsInFileStream.SeekNextByte();
	ulHeaderSize=bsInFileStream.GetByteOffset()-ulHeaderSize;

	return ulHeaderSize;
}

UI32 CSWF::WriteFrameHeader(SWFFRAMEHEADER *lpfhFrameHeader, FILE *pfileSWF)
{
	bsOutFileStream.Create(sizeof(SWFFRAMEHEADER));

	WriteRect(&lpfhFrameHeader->srFrameSize);
	bsOutFileStream.WriteUI16(lpfhFrameHeader->sfpFrameRate);
	bsOutFileStream.WriteUI16(lpfhFrameHeader->usFrameCount);

	bsOutFileStream.WriteToFile(pfileSWF);

	return bsOutFileStream.GetByteOffset();
}

UI32 CSWF::ReadTagHeader(RECORDHEADER *lprhTagHeader)
{
	UI32 ulHeaderSize=bsInFileStream.GetByteOffset();
	
	lprhTagHeader->usTagCodeAndLength=bsInFileStream.ReadUI16();
	lprhTagHeader->usTagCode=lprhTagHeader->usTagCodeAndLength>>6;
	lprhTagHeader->ulLength=lprhTagHeader->usTagCodeAndLength & 0x003F;

	/*This is a long header*/
	if(lprhTagHeader->ulLength==0x3F)
		lprhTagHeader->ulLength=bsInFileStream.ReadUI32();
		
	bsOutFileStream.SeekNextByte();
	ulHeaderSize=bsInFileStream.GetByteOffset()-ulHeaderSize;
		
	return ulHeaderSize;
}

UI32 CSWF::WriteTagHeader(RECORDHEADER *lprhTagHeader, FILE *pfileSWF)
{
	UI32 ulHeaderSize=0;
	UI8 ubLongHeader=(lprhTagHeader->ulLength>=0x003F || lprhTagHeader->usTagCode==TAGCODE_DEFINESPRITE);
	
	if(ubLongHeader) lprhTagHeader->usTagCodeAndLength=lprhTagHeader->usTagCode<<6 | 0x003F;
	else lprhTagHeader->usTagCodeAndLength=UI16(lprhTagHeader->usTagCode<<6 | lprhTagHeader->ulLength);

	ulHeaderSize+=fwrite(&lprhTagHeader->usTagCodeAndLength,1,sizeof(UI16),pfileSWF);
	if(ubLongHeader) ulHeaderSize+=fwrite(&lprhTagHeader->ulLength,1,sizeof(UI32),pfileSWF);
	
	return ulHeaderSize;
}
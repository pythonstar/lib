#include "swf.h"

UI32 CSWF::ReadDefineVideoStream(DEFINEVIDEOSTREAM *lpdvsVideoStream)
{
	UI32 ulTagLength=bsInFileStream.GetByteOffset();

	memset(lpdvsVideoStream,0,sizeof(DEFINEVIDEOSTREAM));
	memcpy(&lpdvsVideoStream->rhTagHeader,&rhTagHeader,sizeof(RECORDHEADER));

	lpdvsVideoStream->usCharacterID=bsInFileStream.ReadUI16();
	lpdvsVideoStream->usNumFrames=bsInFileStream.ReadUI16();
	lpdvsVideoStream->usWidth=bsInFileStream.ReadUI16();
	lpdvsVideoStream->usHeight=bsInFileStream.ReadUI16();

	lpdvsVideoStream->ubVideoFlagsReserved=bsInFileStream.ReadUB(5);
	lpdvsVideoStream->ubVideoFlagsDeblocking=bsInFileStream.ReadUB(2);
	lpdvsVideoStream->fVideoFlagsSmoothing=bsInFileStream.ReadUB(1);

	lpdvsVideoStream->ubCodecID=bsInFileStream.ReadUI8();

	ulTagLength=bsInFileStream.GetByteOffset()-ulTagLength;

	return ulTagLength;
}

UI32 CSWF::WriteDefineVideoStream(DEFINEVIDEOSTREAM *lpdvsVideoStream, FILE *pfileSWF)
{
	bsOutFileStream.Create(sizeof(DEFINEVIDEOSTREAM));

	bsOutFileStream.WriteUI16(lpdvsVideoStream->usCharacterID);
	bsOutFileStream.WriteUI16(lpdvsVideoStream->usNumFrames);
	bsOutFileStream.WriteUI16(lpdvsVideoStream->usWidth);
	bsOutFileStream.WriteUI16(lpdvsVideoStream->usHeight);

	bsOutFileStream.WriteUB(lpdvsVideoStream->ubVideoFlagsReserved,5);
	bsOutFileStream.WriteUB(lpdvsVideoStream->ubVideoFlagsDeblocking,2);
	bsOutFileStream.WriteUB(lpdvsVideoStream->fVideoFlagsSmoothing,1);

	bsOutFileStream.WriteUI8(lpdvsVideoStream->ubCodecID);

	lpdvsVideoStream->rhTagHeader.usTagCode=TAGCODE_DEFINEVIDEOSTREAM;
	lpdvsVideoStream->rhTagHeader.ulLength=bsOutFileStream.GetByteOffset();
	WriteTagHeader(&lpdvsVideoStream->rhTagHeader,pfileSWF);
	bsOutFileStream.WriteToFile(pfileSWF);

	return bsOutFileStream.GetByteOffset();
}

UI32 CSWF::ReadVideoFrame(VIDEOFRAME *lpvfVideoFrame)
{
	UI32 ulTagLength=bsInFileStream.GetByteOffset();

	memset(lpvfVideoFrame,0,sizeof(VIDEOFRAME));
	memcpy(&lpvfVideoFrame->rhTagHeader,&rhTagHeader,sizeof(RECORDHEADER));

	lpvfVideoFrame->usStreamID=bsInFileStream.ReadUI16();
	lpvfVideoFrame->usFrameNum=bsInFileStream.ReadUI16();

	lpvfVideoFrame->ulVideoDataSize=lpvfVideoFrame->rhTagHeader.ulLength-4;
	lpvfVideoFrame->lprgbVideoData=new UI8[lpvfVideoFrame->ulVideoDataSize];
	bsInFileStream.ReadBytes(lpvfVideoFrame->lprgbVideoData,lpvfVideoFrame->ulVideoDataSize);

	ulTagLength=bsInFileStream.GetByteOffset()-ulTagLength;

	return ulTagLength;
}

UI32 CSWF::WriteVideoFrame(VIDEOFRAME *lpvfVideoFrame, FILE *pfileSWF)
{
	bsOutFileStream.Create(sizeof(VIDEOFRAME)+lpvfVideoFrame->ulVideoDataSize);

	bsOutFileStream.WriteUI16(lpvfVideoFrame->usStreamID);
	bsOutFileStream.WriteUI16(lpvfVideoFrame->usFrameNum);
	bsOutFileStream.WriteBytes(lpvfVideoFrame->lprgbVideoData,lpvfVideoFrame->ulVideoDataSize);

	lpvfVideoFrame->rhTagHeader.usTagCode=TAGCODE_VIDEOFRAME;
	lpvfVideoFrame->rhTagHeader.ulLength=bsOutFileStream.GetByteOffset();
	WriteTagHeader(&lpvfVideoFrame->rhTagHeader,pfileSWF);
	bsOutFileStream.WriteToFile(pfileSWF);

	return bsOutFileStream.GetByteOffset();
}

UI32 CSWF::SaveDefineVideoStream(DEFINEVIDEOSTREAM* lpdvsVideoStream)
{
	UI32 ulLength;
	UI16 usFrame=0;
	char szVideoFileName[4096];
	CSWF cswfVideoFlash;
	
	/*SWFFILEHEADER*/
	SWFFILEHEADER fhVideoFileHeader;
	fhVideoFileHeader.bSignature1='F';
	fhVideoFileHeader.bSignature2='W';
	fhVideoFileHeader.bSignature3='S';
	fhVideoFileHeader.bVersion=6;
	fhVideoFileHeader.ulFileLength=0;
	cswfVideoFlash.SetFileHeader(&fhVideoFileHeader);
	
	/*SWFFRAMEHEADER*/
	SWFFRAMEHEADER fhShapeVideoHeader;
	fhShapeVideoHeader.sfpFrameRate=fhFrameHeader.sfpFrameRate;
	fhShapeVideoHeader.usFrameCount=lpdvsVideoStream->usNumFrames;
	fhShapeVideoHeader.srFrameSize.bNbits=31;
	fhShapeVideoHeader.srFrameSize.slXMin=0;
	fhShapeVideoHeader.srFrameSize.slYMin=0;
	fhShapeVideoHeader.srFrameSize.slXMax=lpdvsVideoStream->usWidth*20+100;
	fhShapeVideoHeader.srFrameSize.slYMax=lpdvsVideoStream->usHeight*20+100;
	cswfVideoFlash.SetFrameHeader(&fhShapeVideoHeader);

	/*SETBACKGROUNDCOLOR*/
	SETBACKGROUNDCOLOR sbcVideoBackground;
	sbcVideoBackground.rhTagHeader.usTagCode=TAGCODE_SETBACKGROUNDCOLOR;
	sbcVideoBackground.rhTagHeader.ulLength=3;
	sbcVideoBackground.rgbBackgroundColor.ubRed=128;
	sbcVideoBackground.rgbBackgroundColor.ubGreen=128;
	sbcVideoBackground.rgbBackgroundColor.ubBlue=128;
	cswfVideoFlash.AddTag(&sbcVideoBackground);

	/*Video*/
	cswfVideoFlash.AddTag(lpdvsVideoStream);
	
	/*PLACEOBJECT(2)*/
	PLACEOBJECT2 *lprgpo2Place=new PLACEOBJECT2[lpdvsVideoStream->usNumFrames];

	MATRIX mMatrix;
	mMatrix.fHasScale=0;
	mMatrix.lfpScaleX=1;
	mMatrix.lfpScaleY=1;
	mMatrix.fHasRotate=0;
	mMatrix.ubNTranslateBits=31;
	mMatrix.slTranslateX=50;
	mMatrix.slTranslateY=50;
	
	memset(&lprgpo2Place[usFrame],0,sizeof(PLACEOBJECT2));
	lprgpo2Place[0].rhTagHeader.usTagCode=TAGCODE_PLACEOBJECT2;
	lprgpo2Place[0].fPlaceFlagHasClipDepth=true;
	lprgpo2Place[0].fPlaceFlagHasRatio=true;
	lprgpo2Place[0].fPlaceFlagHasMatrix=true;
	lprgpo2Place[0].fPlaceFlagHasCharacter=true;
	lprgpo2Place[0].usDepth=1;
	lprgpo2Place[0].usRatio=1;
	memcpy(&lprgpo2Place[0].mMatrix,&mMatrix,sizeof(MATRIX));
	lprgpo2Place[0].usCharacterID=lpdvsVideoStream->usCharacterID;
		
	UNKNOWNTAG utShowFrame;
	memset(&utShowFrame,0,sizeof(UNKNOWNTAG));
	utShowFrame.rhTagHeader.usTagCode=TAGCODE_SHOWFRAME;
		
	for(UI32 iTag=0; iTag<nTags; iTag++)
	{
		if(((UNKNOWNTAG*)lprgvpTags[iTag])->rhTagHeader.usTagCode==TAGCODE_VIDEOFRAME &&
			((VIDEOFRAME*)lprgvpTags[iTag])->usStreamID==lpdvsVideoStream->usCharacterID)
		{
			if(usFrame)
			{
				memset(&lprgpo2Place[usFrame],0,sizeof(PLACEOBJECT2));
				lprgpo2Place[usFrame].rhTagHeader.usTagCode=TAGCODE_PLACEOBJECT2;
				lprgpo2Place[usFrame].fPlaceFlagHasClipDepth=true;
				lprgpo2Place[usFrame].fPlaceFlagHasRatio=true;
				lprgpo2Place[usFrame].fPlaceFlagMove=true;
				lprgpo2Place[usFrame].usDepth=1;
				lprgpo2Place[usFrame].usRatio=usFrame+1;
			}

			cswfVideoFlash.AddTag(&lprgpo2Place[usFrame]);
			cswfVideoFlash.AddTag((VIDEOFRAME*)lprgvpTags[iTag]);
			cswfVideoFlash.AddTag(&utShowFrame);
			usFrame++;
			
			if(((VIDEOFRAME*)lprgvpTags[iTag])->usFrameNum==lpdvsVideoStream->usNumFrames)
				break;
		}
	}

	/*END*/
	UNKNOWNTAG utEnd;
	memset(&utEnd,0,sizeof(UNKNOWNTAG));
	utEnd.rhTagHeader.usTagCode=TAGCODE_END;
	cswfVideoFlash.AddTag(&utEnd);
	
	/*Save*/
	CreateFileName(szVideoFileName,"video",lpdvsVideoStream->usCharacterID,"swf");
	cswfVideoFlash.Save(szVideoFileName);

	delete lprgpo2Place;

	return 0;
}
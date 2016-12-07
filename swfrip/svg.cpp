#include "svg.h"

extern CSWF cswfLoaded;

void WriteTransform(MATRIX *lpmMatrix, FILE *pfileSVG, UI8 fBitmap)
{
	if(!lpmMatrix) return;

	fprintf(pfileSVG," transform=\"matrix(%.3f,%.3f,%.3f,%.3f,%.3f,%.3f)\"",
		FP32TOFLOAT(lpmMatrix->lfpScaleX),
		FP32TOFLOAT(lpmMatrix->lfpRotateSkew0), 
		FP32TOFLOAT(lpmMatrix->lfpRotateSkew1),
		FP32TOFLOAT(lpmMatrix->lfpScaleY),
		float(lpmMatrix->slTranslateX)/20,
		float(lpmMatrix->slTranslateY)/20);
}

void WriteColorMatrixFilter(CXFORM *lpcxfwaColorTransform, char *szID, FILE *pfileSVG)
{
	char szMatrix[128];
	
	sprintf(szMatrix,"%.3f,0,0,0,%.3f 0,%.3f,0,0,%.3f 0,0,%.3f,0,%.3f 0,0,0,%.3f,%.3f",
			FP16TOFLOAT(lpcxfwaColorTransform->sfpRedMultTerm),
			sqrt((double)lpcxfwaColorTransform->sbRedAddTerm)/255,
			FP16TOFLOAT(lpcxfwaColorTransform->sfpGreenMultTerm),
			sqrt((double)lpcxfwaColorTransform->sbGreenAddTerm)/255,
			FP16TOFLOAT(lpcxfwaColorTransform->sfpBlueMultTerm),
			sqrt((double)lpcxfwaColorTransform->sbBlueAddTerm)/255,
			FP16TOFLOAT(lpcxfwaColorTransform->sfpAlphaMultTerm),
			sqrt((double)lpcxfwaColorTransform->sbAlphaAddTerm)/255.0);
	
	fprintf(pfileSVG,"<filter id=\"%s\">\n",szID);
	fprintf(pfileSVG,"<feColorMatrix type=\"matrix\" result=\"SourceGraphic\" ");
	fprintf(pfileSVG,"values=\"%s\" />\n</filter>\n\n",szMatrix);
}

void TransformColor(RGBA *lprgbaColor, CXFORM *lpcxfwaColorTransform) 
{
	double sdRed, sdGreen, sdBlue, sdAlpha;

	if(!lprgbaColor || !lpcxfwaColorTransform) return;

	sdRed=lprgbaColor->ubRed;
	sdGreen=lprgbaColor->ubGreen;
	sdBlue=lprgbaColor->ubBlue;
	sdAlpha=lprgbaColor->ubAlpha;

	if(lpcxfwaColorTransform->fHasMultTerms)
	{
		sdRed*=FP16TOFLOAT(lpcxfwaColorTransform->sfpRedMultTerm);
		sdGreen*=FP16TOFLOAT(lpcxfwaColorTransform->sfpGreenMultTerm);
		sdBlue*=FP16TOFLOAT(lpcxfwaColorTransform->sfpBlueMultTerm);
		sdAlpha*=FP16TOFLOAT(lpcxfwaColorTransform->sfpAlphaMultTerm);
	}
	
	if(lpcxfwaColorTransform->fHasAddTerms)
	{
		sdRed+=FP16TOFLOAT(lpcxfwaColorTransform->sbRedAddTerm);
		sdGreen+=FP16TOFLOAT(lpcxfwaColorTransform->sbGreenAddTerm);
		sdBlue+=FP16TOFLOAT(lpcxfwaColorTransform->sbBlueAddTerm);
		sdAlpha+=FP16TOFLOAT(lpcxfwaColorTransform->sbAlphaAddTerm);
	}
	
	if(sdRed<0) sdRed=0;
	if(sdRed>255) sdRed=255;
	if(sdGreen<0) sdGreen=0;
	if(sdGreen>255) sdGreen=255;
	if(sdBlue<0) sdBlue=0;
	if(sdBlue>255) sdBlue=255;
	if(sdAlpha<0) sdAlpha=0;
	if(sdAlpha>255) sdAlpha=255;

	lprgbaColor->ubRed=sdRed;
	lprgbaColor->ubGreen=sdGreen;
	lprgbaColor->ubBlue=sdBlue;
	lprgbaColor->ubAlpha=sdAlpha;
}

UI32 WriteSVGFillGradient(FILLSTYLE *lpfsFillStyle, char *szID, FILE *pfileSVG)
{
	GRADIENT *lpgGradient=&lpfsFillStyle->gGradient;
	MATRIX *lpmMatrix=&lpfsFillStyle->mGradientMatrix;
	GRADIENTRECORD *lpgrGradRecord;
	double sdWidth, sdHeight;
	double sdX1, sdY1, sdX2, sdY2, sdCX, sdCY, sdR, sdFX, sdFY;
	double sdScaleX, sdScaleY, sdRotate0, sdRotate1, sdTranslateX, sdTranslateY;

	switch(lpfsFillStyle->ubFillStyleType)
	{
		case FILLSTYLE_LINEARGRADIENT:
			sdX1=0;
			sdY1=0;
			sdX2=1;
			sdY2=1;
			fprintf(pfileSVG,"<linearGradient x1=\"%0.3f\" y1=\"%0.3f\" x2=\"%0.3f\" y2=\"%0.3f\" ",
					sdX1,sdY1,sdX2,sdY2);
			
			break;

		case FILLSTYLE_RADIALGRADIENT:
			sdCX=.5;
			sdCY=.5;
			sdR=1;
			sdFX=0;
			sdFY=0;
			fprintf(pfileSVG,"<radialGradient cx=\"%0.3f\" cy=\"%0.3f\" r=\"%0.3f\" ",sdCX,sdCY,sdR);
			fprintf(pfileSVG,"fx=\"%0.3f\" fy=\"%0.3f\" ",sdFX,sdFY);
			break;

		default: return 0;
	}

	lpmMatrix->fHasScale;
	lpmMatrix->fHasRotate;

	sdScaleX=FP32TOFLOAT(lpmMatrix->lfpScaleX);
	sdScaleY=FP32TOFLOAT(lpmMatrix->lfpScaleY);
	sdRotate0=FP32TOFLOAT(lpmMatrix->lfpRotateSkew0);
	sdRotate1=FP32TOFLOAT(lpmMatrix->lfpRotateSkew1);
	sdTranslateX=double(lpmMatrix->slTranslateX)/20;
	sdTranslateY=double(lpmMatrix->slTranslateY)/20;

	//sdWidth=double(rRect.slXMax-rRect.slXMin)/20;
	//sdHeight=double(rRect.slYMax-rRect.slYMin)/20;
	
	sdScaleX=1;//(16384*sdScaleX)/sdWidth;
	sdScaleY=1;//(16384*sdScaleY)/sdHeight;

	sdRotate0=0;
	sdRotate1=0;
	
	sdTranslateX=0;//sdTranslateX-(rRect.slXMin-16384);
	sdTranslateY=0;//sdTranslateY-(rRect.slYMin-16384);

	fprintf(pfileSVG,"id=\"%s\" ",szID);
	fprintf(pfileSVG,"gradientTransform=\"matrix(%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f)\">\n",
			sdScaleX,sdRotate0,sdRotate1,sdScaleY,sdTranslateX,sdTranslateY);
						
	for(UI32 iGradRecord=0; iGradRecord<lpgGradient->ubNumGradients; iGradRecord++)
	{
		lpgrGradRecord=&lpgGradient->lprggrGradientRecords[iGradRecord];
		fprintf(pfileSVG,"<stop style=\"stop-color: #%01X%01X%01X; ",
				UI32(float(lpgrGradRecord->rgbaColor.ubRed)/16),
				UI32(float(lpgrGradRecord->rgbaColor.ubGreen)/16),
				UI32(float(lpgrGradRecord->rgbaColor.ubBlue)/16));
		fprintf(pfileSVG,"stop-opacity: %0.3f;\" ",float(lpgrGradRecord->rgbaColor.ubAlpha)/255);
		fprintf(pfileSVG,"offset=\"%lu\" />\n",iGradRecord);
	}

	switch(lpfsFillStyle->ubFillStyleType)
	{
		case FILLSTYLE_LINEARGRADIENT:
			fprintf(pfileSVG,"</linearGradient>\n\n");
			break;

		case FILLSTYLE_RADIALGRADIENT:
			fprintf(pfileSVG,"</radialGradient>\n\n");
			break;

		default: return 0;
	}
	
	return 0;
}

UI32 WriteImage(DEFINESHAPE *lpdsShape, EDGE *lpeEdge, FILE *pfileSVG)
{
	char szResource[MAX_FILEPATH];
	UI8 ubFormat;
	MATRIX mBitmapMatrix;

	if(!lpdsShape) return 0;
	if(!lpeEdge) return 0;
	if(!pfileSVG) return 0;

	/*set the image type to png, since ASV doesn't support BMP*/
	ubFormat=cswfLoaded.GetExportFormat(SWF_IMAGE);
	cswfLoaded.SetExportFormat(SWF_IMAGE,2);
	cswfLoaded.SaveCharacter(lpeEdge->lpfsFillStyle0->usBitmapID);
	cswfLoaded.GetLastRes(szResource);
			
	/*image position*/
	memcpy(&mBitmapMatrix,&lpeEdge->lpfsFillStyle0->mBitmapMatrix,sizeof(MATRIX));

	fprintf(pfileSVG,"<image x=\"%.2f\" y=\"%.2f\" width=\"100%%\" height=\"100%%\"",
			//double(lpdsShape->rShapeBounds.slXMin)/20,
			//double(lpdsShape->rShapeBounds.slYMin)/20);
			double(lpeEdge->lpfsFillStyle0->mBitmapMatrix.slTranslateX)/20,
			double(lpeEdge->lpfsFillStyle0->mBitmapMatrix.slTranslateY)/20);

				
	/*include url and clip path*/
	fprintf(pfileSVG," xlink:href=\"%s\" clip-path=\"url(#clip_%p)\"",
			szResource,lpeEdge->lpfsFillStyle0);
		
	/*write tranform matrix*/
	mBitmapMatrix.lfpScaleX=FLOATTOFP32(FP32TOFLOAT(mBitmapMatrix.lfpScaleX)/20);
	mBitmapMatrix.lfpScaleY=FLOATTOFP32(FP32TOFLOAT(mBitmapMatrix.lfpScaleY)/20);
	mBitmapMatrix.lfpRotateSkew0=FLOATTOFP32(FP32TOFLOAT(mBitmapMatrix.lfpRotateSkew0)/20);
	mBitmapMatrix.lfpRotateSkew1=FLOATTOFP32(FP32TOFLOAT(mBitmapMatrix.lfpRotateSkew1)/20);
	mBitmapMatrix.slTranslateX=lpdsShape->rShapeBounds.slXMin-mBitmapMatrix.slTranslateX;
	mBitmapMatrix.slTranslateY=lpdsShape->rShapeBounds.slYMin-mBitmapMatrix.slTranslateY;
	
	WriteTransform(&mBitmapMatrix,pfileSVG,false);
	
	/*end image tag*/
	fprintf(pfileSVG," />\n\n");

	/*reset image export format*/
	cswfLoaded.SetExportFormat(SWF_IMAGE,ubFormat);

	return 1;
}

UI32 WriteSVGPaths(DEFINESHAPE *lpdsShape, CEdgeList *lpelEdgeList, UI8 fLines, FILE *pfileSVG, PLACEOBJECT2 *lppo2Place)
{
	EDGE eCurEdge, ePrevEdge;
	char szStroke[32], szFill[32];
	double sdFillOpacity, sdStrokeOpacity, sdStrokeWidth;
	UI32 iGradRecord;
	UI8 fFirst=true, fInClip=false;
	RGBA rgbaColor;

	if(fLines) fprintf(pfileSVG,"<!-- Path Lines -->\n\n");
	else fprintf(pfileSVG,"<!-- Path Fills -->\n\n");

	/*render svg*/
	while(lpelEdgeList->GetNextEdge(&eCurEdge))
	{
		if(fFirst || !MatchStyles(&eCurEdge,&ePrevEdge,false,fLines))
		{
			if(!fFirst) fprintf(pfileSVG,"\" />\n\n");

			/*end clip path and include image*/
			if(fInClip)
			{
				fprintf(pfileSVG,"</clipPath>\n\n");
				WriteImage(lpdsShape,&eCurEdge,pfileSVG);
				fInClip=false;
			}

			/*fillstyle*/
			if(fLines || !eCurEdge.lpfsFillStyle0) 
			{
				sprintf(szFill,"none");
				sdFillOpacity=0;
			}
			
			else 
			{
				switch(eCurEdge.lpfsFillStyle0->ubFillStyleType)
				{
					case FILLSTYLE_SOLID:
						
						memcpy(&rgbaColor,&eCurEdge.lpfsFillStyle0->rgbaColor,sizeof(RGBA));

						//if(lppo2Place && lppo2Place->fPlaceFlagHasColorTransform)
						//	TransformColor(&rgbaColor,&lppo2Place->cxfwaColorTransform);

						sprintf(szFill,"#%02X%02X%02X",
								rgbaColor.ubRed,rgbaColor.ubGreen,rgbaColor.ubBlue);
						
						sdFillOpacity=float(rgbaColor.ubAlpha)/255;
						break;

					case FILLSTYLE_LINEARGRADIENT:
					case FILLSTYLE_RADIALGRADIENT:
						//break;
						sprintf(szFill,"gradient_%p",&eCurEdge.lpfsFillStyle0->gGradient);
						WriteSVGFillGradient(eCurEdge.lpfsFillStyle0,szFill,pfileSVG);
						sprintf(szFill,"url(#gradient_%p)",&eCurEdge.lpfsFillStyle0->gGradient);
						sdFillOpacity=1;
						break;

					case FILLSTYLE_REPEATINGBITMAP:
					case FILLSTYLE_CLIPPEDBITMAP:
					case FILLSTYLE_REPEATINGBITMAPNS:
					case FILLSTYLE_CLIPPEDBITMAPNS:
						sprintf(szFill,"#000000");
						sdFillOpacity=1;
						fInClip=true;

						/*start the clip path*/
						fprintf(pfileSVG,"<clipPath id=\"clip_%p\">\n\n",eCurEdge.lpfsFillStyle0);
						break;
						
					default: sprintf(szFill,"none");
				}
			}

			/*linestyle*/
			if(!fLines || !eCurEdge.lplsLineStyle) 
			{
				sprintf(szStroke,"none");
				sdStrokeOpacity=0;
				sdStrokeWidth=0;
			}

			else
			{
				memcpy(&rgbaColor,&eCurEdge.lplsLineStyle->rgbaColor,sizeof(RGBA));

				//if(lppo2Place && lppo2Place->fPlaceFlagHasColorTransform)
				//	TransformColor(&rgbaColor,&lppo2Place->cxfwaColorTransform);

				sprintf(szStroke,"#%02X%02X%02X",
						rgbaColor.ubRed,rgbaColor.ubGreen,rgbaColor.ubBlue);

				sdStrokeOpacity=float(rgbaColor.ubAlpha)/255;
	
				sdStrokeWidth=float(eCurEdge.lplsLineStyle->usWidth)/20;
			}
			
			fprintf(pfileSVG,"<path style=\"");
			
			fprintf(pfileSVG,"stroke: %s; ",szStroke);
			fprintf(pfileSVG,"stroke-opacity: %0.3f; ",sdStrokeOpacity);
			fprintf(pfileSVG,"stroke-width: %0.2f; ",sdStrokeWidth);

			fprintf(pfileSVG,"fill: %s; ",szFill);
			fprintf(pfileSVG,"fill-opacity: %0.3f; ",sdFillOpacity);
			
			fprintf(pfileSVG,"\" d=\"M %0.2f,%0.2f ",eCurEdge.sdStartX,eCurEdge.sdStartY);
		}

		/*if it's drawing lines and there is no stroke, don't bother drawing it*/
		if(fLines && !eCurEdge.lplsLineStyle) 
		{
			memcpy(&ePrevEdge,&eCurEdge,sizeof(EDGE));
			fFirst=false;
			continue;
		}

		/*if it's drawing fills and there is no fill, don't bother drawing it*/
		if(!fLines && !eCurEdge.lpfsFillStyle0) 
		{
			memcpy(&ePrevEdge,&eCurEdge,sizeof(EDGE));
			fFirst=false;
			continue;
		}

		/*if the edges are not adjecent, move to the current position*/
		else if(!AreAdjecentEdges(&ePrevEdge,&eCurEdge))
			fprintf(pfileSVG,"M %0.2f,%0.2f ",eCurEdge.sdStartX,eCurEdge.sdStartY);

		/*a quadratic bezier*/
		if(eCurEdge.ubType)
		{
			fprintf(pfileSVG,"q %0.2f,%0.2f %0.2f,%0.2f ",
					eCurEdge.sdControlX-eCurEdge.sdStartX,
					eCurEdge.sdControlY-eCurEdge.sdStartY,
					eCurEdge.sdEndX-eCurEdge.sdStartX,
					eCurEdge.sdEndY-eCurEdge.sdStartY);
		}

		/*a strait line*/
		else 
		{
			fprintf(pfileSVG,"l %0.2f,%0.2f ",
					eCurEdge.sdEndX-eCurEdge.sdStartX,
					eCurEdge.sdEndY-eCurEdge.sdStartY);
		}

		memcpy(&ePrevEdge,&eCurEdge,sizeof(EDGE));
		fFirst=false;
	}

	/*end path element*/
	fprintf(pfileSVG,"\"/>\n\n");

	/*end clip path and include image*/
	if(fInClip)
	{
		fprintf(pfileSVG,"</clipPath>\n\n");
		WriteImage(lpdsShape,&eCurEdge,pfileSVG);
		fInClip=false;
	}


	return 0;
}
#include "swf.h"

UI32 CSWF::WriteDefineShapeSSWF(DEFINESHAPE *lpdsShape, UI8 fRGBA, FILE *pfileSSWF)
{
	UI32 iFillStyle0=0, iFillStyle1=0, iLineStyle=0;
	double sdDeltaX, sdDeltaY, sdXPosition=0, sdYPosition=0, sdControlX, sdControlY, sdLineWidth;
	RGBA rgbaFill0, rgbaFill1, rgbaLine;
	FILLSTYLE *lprgfsFillStyles;
	LINESTYLE *lprglsLineStyles;
	UI8 fInEdges=false;

	/*init line styles*/
	lprgfsFillStyles=lpdsShape->swsShapeWithStyle.fsaFillStyleArray.lprgfsFillStyles;
	lprglsLineStyles=lpdsShape->swsShapeWithStyle.lsaLineStyleArray.lprglsLineStyles;

	/*id and rect*/
	fprintf(pfileSSWF,"\nshape \"shape_%05lu\" {\n",lpdsShape->usShapeID);
	fprintf(pfileSSWF,"\trect { %.3f, %.3f, %.3f, %.3f };\n",
						double(lpdsShape->rShapeBounds.slXMin)/20,
						double(lpdsShape->rShapeBounds.slYMin)/20,
						double(lpdsShape->rShapeBounds.slXMax)/20,
						double(lpdsShape->rShapeBounds.slYMax)/20);

	//fprintf(pfileSSWF,"\tfill0: no_fill;\n");
	//fprintf(pfileSSWF,"\tfill1: no_fill;\n");

	/*shape records*/
	for(UI32 iCurRecord=0; iCurRecord<lpdsShape->swsShapeWithStyle.ulNumShapeRecords; iCurRecord++)
	{
		void *lpvRecord=lpdsShape->swsShapeWithStyle.lprgvpShapeRecords[iCurRecord];

		/*EDGERECORD*/
		if(((STRAIGHTEDGERECORD*)(lpvRecord))->fTypeFlag)
		{
			if(!fInEdges)/*start edges*/
			{
				fprintf(pfileSSWF,"\n\tedges {\n");
				fInEdges=true;
			}

			/*STRAIGHTEDGERECORD*/
			if(((STRAIGHTEDGERECORD*)lpvRecord)->fStraightFlag)
			{
				if(((STRAIGHTEDGERECORD*)lpvRecord)->fGeneralLineFlag)
				{		
					sdDeltaX=double(((STRAIGHTEDGERECORD*)lpvRecord)->slGeneralDeltaX)/20;
					sdDeltaY=double(((STRAIGHTEDGERECORD*)lpvRecord)->slGeneralDeltaY)/20;
					sdXPosition+=sdDeltaX;
					sdYPosition+=sdDeltaY;
				}

				else
				{	/*vertical line*/
					if(((STRAIGHTEDGERECORD*)lpvRecord)->fVertLineFlag)
					{						
						sdDeltaX=0;
						sdDeltaY=double(((STRAIGHTEDGERECORD*)lpvRecord)->slVertDeltaY)/20;
						sdYPosition+=sdDeltaY;
					}
					
					else /*horizontal line*/
					{
						sdDeltaX=double(((STRAIGHTEDGERECORD*)lpvRecord)->slVertDeltaX)/20;
						sdDeltaY=0;
						sdXPosition+=sdDeltaX;
					}
				}
				
				fprintf(pfileSSWF,"\t\t %0.3f, %0.3f;\n",sdDeltaX,sdDeltaY);
			}

			else /*CURVEDEDGERECORD*/
			{
				if(!fInEdges)/*start edges*/
				{
					fprintf(pfileSSWF,"\n\tedges {\n");
					fInEdges=true;
				}

				sdControlX=double(((CURVEDEDGERECORD*)lpvRecord)->slControlDeltaX)/20;
				sdControlY=double(((CURVEDEDGERECORD*)lpvRecord)->slControlDeltaY)/20;

				sdDeltaX=double(((CURVEDEDGERECORD*)lpvRecord)->slAnchorDeltaX)/20;
				sdDeltaY=double(((CURVEDEDGERECORD*)lpvRecord)->slAnchorDeltaY)/20;

				sdXPosition=sdXPosition+sdControlX+sdDeltaX;
				sdYPosition=sdYPosition+sdControlY+sdDeltaY;

				fprintf(pfileSSWF,"\t\t%0.3f, %0.3f, %0.3f, %0.3f;\n",sdControlX,sdControlY,sdDeltaX,sdDeltaY);
			}
		}

		else /*STYLECHANGERECORD*/
		{
			if(((STYLECHANGERECORD*)lpvRecord)->fStateMoveTo)
			{
				/*close and end*/
				if(fInEdges) fprintf(pfileSSWF,"\t\tclose: 0,0;\n\t};\n\n");
				fInEdges=false;

				sdDeltaX=double(((STYLECHANGERECORD*)lpvRecord)->slMoveDeltaX)/20;
				sdDeltaY=double(((STYLECHANGERECORD*)lpvRecord)->slMoveDeltaY)/20;
				
				sdXPosition=sdDeltaX;
				sdYPosition=sdDeltaY;

				fprintf(pfileSSWF,"\t//fStateMoveTo\n");
				//fprintf(pfileSSWF,"\t//move: %.3f, %.3f;\n",sdXPosition,sdYPosition);
			}

			if(((STYLECHANGERECORD*)lpvRecord)->fStateNewStyles)
			{
				if(((STYLECHANGERECORD*)lpvRecord)->fsaFillStyleArray.lprgfsFillStyles)
					lprgfsFillStyles=((STYLECHANGERECORD*)lpvRecord)->fsaFillStyleArray.lprgfsFillStyles;
				
				if(((STYLECHANGERECORD*)lpvRecord)->lsaLineStyleArray.lprglsLineStyles)
					lprglsLineStyles=((STYLECHANGERECORD*)lpvRecord)->lsaLineStyleArray.lprglsLineStyles;
			
				fprintf(pfileSSWF,"\t//fStateNewStyles\n");
				//fprintf(pfileSSWF,"\tfill0: no_fill;\n");
				//fprintf(pfileSSWF,"\tfill1: no_fill;\n");
			}

			if(((STYLECHANGERECORD*)lpvRecord)->fStateFillStyle0)
			{
				/*close and end*/
				if(fInEdges) fprintf(pfileSSWF,"\t\tclose: 0,0;\n\t};\n\n");
				fInEdges=false;

				fprintf(pfileSSWF,"\t//fStateFillStyle0\n");

				if(iFillStyle0=((STYLECHANGERECORD*)lpvRecord)->ulFillStyle0)
					switch(lprgfsFillStyles[iFillStyle0-1].ubFillStyleType)
					{
						case FILLSTYLE_SOLID:
							memcpy(&rgbaFill0,&lprgfsFillStyles[iFillStyle0-1].rgbaColor,sizeof(RGBA));
							fprintf(pfileSSWF,"\tfill0: fill style { color { %.5f, %.5f, %.5f }; };\n",
									float(rgbaFill0.ubRed)/255,float(rgbaFill0.ubGreen)/255,float(rgbaFill0.ubBlue)/255);
							break;

						default:
							fprintf(pfileSSWF,"\t//fill type: %lu\n",lprgfsFillStyles[iFillStyle0-1].ubFillStyleType);
					}

				else fprintf(pfileSSWF,"\tfill0: no_fill;\n");
			}
			
			if(((STYLECHANGERECORD*)lpvRecord)->fStateFillStyle1)
			{
				/*close and end*/
				if(fInEdges) fprintf(pfileSSWF,"\t\tclose: 0,0;\n\t};\n\n");
				fInEdges=false;

				fprintf(pfileSSWF,"\t//fStateFillStyle1\n");

				if(iFillStyle1=((STYLECHANGERECORD*)lpvRecord)->ulFillStyle1)
					switch(lprgfsFillStyles[iFillStyle1-1].ubFillStyleType)
					{
						case FILLSTYLE_SOLID:
							memcpy(&rgbaFill1,&lprgfsFillStyles[iFillStyle1-1].rgbaColor,sizeof(RGBA));
							fprintf(pfileSSWF,"\tfill1: fill style { color { %.5f, %.5f, %.5f }; };\n",
									float(rgbaFill1.ubRed)/255,float(rgbaFill1.ubGreen)/255,float(rgbaFill1.ubBlue)/255);
							break;

						default:
							fprintf(pfileSSWF,"\t//fill type: %lu\n",lprgfsFillStyles[iFillStyle1-1].ubFillStyleType);

					}

				else fprintf(pfileSSWF,"\tfill1: no_fill;\n");
			}
			
			if(((STYLECHANGERECORD*)lpvRecord)->fStateLineStyle)
			{
				fprintf(pfileSSWF,"\t//fStateLineStyle (%lu)\n",((STYLECHANGERECORD*)lpvRecord)->ulLineStyle);

				if(!lprglsLineStyles) continue;

				if(iLineStyle=((STYLECHANGERECORD*)lpvRecord)->ulLineStyle)
				{
					memcpy(&rgbaLine,&lprglsLineStyles[iLineStyle-1].rgbaColor,sizeof(RGBA));
					sdLineWidth=double(lprglsLineStyles[iLineStyle-1].usWidth)/20;
				
					fprintf(pfileSSWF,"\tline style { %.3f; color { %.5f, %.5f, %.5f }; };\n",
							sdLineWidth,float(rgbaLine.ubRed)/255,float(rgbaLine.ubGreen)/255,float(rgbaLine.ubBlue)/255);
				}

				else fprintf(pfileSSWF,"\tline style { };\n");
			}

			/*set position*/
			fprintf(pfileSSWF,"\tmove: %.3f, %.3f;\n",sdXPosition,sdYPosition);
		}
	}

	if(fInEdges)/*end edges*/
	{
		fprintf(pfileSSWF,"\t\tclose: 0,0;\n\t};\n");
		fInEdges=false;
	}

	fprintf(pfileSSWF,"};\n\n");

	return 0;
}

UI32 CSWF::SaveSSWF(char *szFileName)
{
	FILE *pfileSSWF=fopen(szFileName,"w");

	char szName[128];
	UI8 ubRed, ubGreen, ubBlue;
	float sfFrameRate, sfWidth, sfHeight;

	sfWidth=double(fhFrameHeader.srFrameSize.slXMax-fhFrameHeader.srFrameSize.slXMin)/20;
	sfHeight=double(fhFrameHeader.srFrameSize.slYMax-fhFrameHeader.srFrameSize.slYMin)/20;
	sfFrameRate=FP16TOFLOAT(fhFrameHeader.sfpFrameRate);

	/*frame rect*/
	fprintf(pfileSSWF,"screen_width=%.3f;\nscreen_height=%.3f;\n",sfWidth,sfHeight);
	fprintf(pfileSSWF,"rectangle \"screen\" { 0, 0, screen_width, screen_height };\n");
	fprintf(pfileSSWF,"fill style \"no_fill\" { };\n");


	for(UI32 iTag=0; iTag<nTags; iTag++)
	{
		switch(((UNKNOWNTAG*)lprgvpTags[iTag])->rhTagHeader.usTagCode)
		{
			case TAGCODE_DEFINESHAPE:
			case TAGCODE_DEFINESHAPE2:
				WriteDefineShapeSSWF((DEFINESHAPE*)lprgvpTags[iTag],false,pfileSSWF);
				break;

			case TAGCODE_DEFINESHAPE3:
				WriteDefineShapeSSWF((DEFINESHAPE*)lprgvpTags[iTag],true,pfileSSWF);
				break;
		}
	}

	/*main sequence*/
	fprintf(pfileSSWF,"\nsequence \"main\" {\n");
	fprintf(pfileSSWF,"\tframe_rate=%.3f;\n",sfFrameRate);
	fprintf(pfileSSWF,"\tcompress=false;\n");
	fprintf(pfileSSWF,"\tscreen;\n");

	ubRed=lpbcBackground->rgbBackgroundColor.ubRed;
	ubGreen=lpbcBackground->rgbBackgroundColor.ubGreen;
	ubBlue=lpbcBackground->rgbBackgroundColor.ubBlue;
	fprintf(pfileSSWF,"\tset background color { %i, %i, %i };\n",ubRed,ubGreen,ubBlue);

	fprintf(pfileSSWF,"\tshape_00004;\n\tplace object {id: shape_00004; depth: 1; matrix {translate: 100,100;};};\n\tshow frame;\n");
	
	
	fprintf(pfileSSWF,"};");
	

	fclose(pfileSSWF);

	return 0;
}
#include "swf.h"
#include "edge.h"

/*
UI32 ulEdgeTolerance=0;

UI8 EqualRange(SI32 slValue1, SI32 slValue2, SI32 slTolerance)
{
	if(ABS(slValue1-slValue2)>slTolerance) return 0;
	return 1;
}*/

UI8 MatchRGBA(RGBA *lprgbaRGBA1, RGBA *lprgbaRGBA2)
{
	if(!lprgbaRGBA1 && !lprgbaRGBA2) return 1;
	if(lprgbaRGBA1 && !lprgbaRGBA2) return 0;
	if(!lprgbaRGBA1 && lprgbaRGBA2) return 0;

	if(lprgbaRGBA1->ubRed!=lprgbaRGBA2->ubRed) return 0;
	if(lprgbaRGBA1->ubGreen!=lprgbaRGBA2->ubGreen) return 0;
	if(lprgbaRGBA1->ubBlue!=lprgbaRGBA2->ubBlue) return 0;
	if(lprgbaRGBA1->ubAlpha!=lprgbaRGBA2->ubAlpha) return 0;

	return 1;
}

UI8 MatchGradient(GRADIENT *lpgGradient1, GRADIENT *lpgGradient2)
{
	GRADIENTRECORD *lpgrGradRecord;

	if(!lpgGradient1 && !lpgGradient2) return 1;
	if(lpgGradient1 && !lpgGradient2) return 0;
	if(!lpgGradient1 && lpgGradient2) return 0;

	if(lpgGradient1->ubNumGradients!=lpgGradient2->ubNumGradients) return 0;

	for(UI32 iGradRecord=0; iGradRecord<lpgGradient1->ubNumGradients; iGradRecord++)
	{
		lpgrGradRecord=&lpgGradient1->lprggrGradientRecords[iGradRecord];
		if(lpgrGradRecord->ubRatio!=lpgrGradRecord->ubRatio) return 0;
		if(!MatchRGBA(&lpgrGradRecord->rgbaColor,&lpgrGradRecord->rgbaColor)) return 0;
	}

	return 1;
}

UI8 MatchMatrix(MATRIX *lpmMatrix1, MATRIX *lpmMatrix2)
{
	if(!lpmMatrix1 && !lpmMatrix2) return 1;
	if(lpmMatrix1 && !lpmMatrix2) return 0;
	if(!lpmMatrix1 && lpmMatrix2) return 0;

	if(lpmMatrix1->fHasScale!=lpmMatrix2->fHasScale) return 0;
	
	if(lpmMatrix1->fHasScale)
	{
		if(lpmMatrix1->lfpScaleX!=lpmMatrix2->lfpScaleX) return 0;
		if(lpmMatrix1->lfpScaleY!=lpmMatrix2->lfpScaleY) return 0;
	}

	if(lpmMatrix1->fHasRotate!=lpmMatrix2->fHasRotate) return 0;
	
	if(lpmMatrix1->fHasRotate)
	{
		if(lpmMatrix1->lfpRotateSkew0!=lpmMatrix2->lfpRotateSkew0) return 0;
		if(lpmMatrix1->lfpRotateSkew1!=lpmMatrix2->lfpRotateSkew1) return 0;
	}
	
	if(lpmMatrix1->slTranslateX!=lpmMatrix2->slTranslateX) return 0;
	if(lpmMatrix1->slTranslateX!=lpmMatrix2->slTranslateX) return 0;

	return 1;
}

UI8 MatchFillStyle(FILLSTYLE *lpfsFillStyle1, FILLSTYLE *lpfsFillStyle2)
{
	if(!lpfsFillStyle1 && !lpfsFillStyle2) return 1;
	if(lpfsFillStyle1 && !lpfsFillStyle2) return 0;
	if(!lpfsFillStyle1 && lpfsFillStyle2) return 0;
	
	if(lpfsFillStyle1->ubFillStyleType!=lpfsFillStyle2->ubFillStyleType) return 0;
	
	switch(lpfsFillStyle1->ubFillStyleType)
	{
		case FILLSTYLE_SOLID:
			if(!MatchRGBA(&lpfsFillStyle1->rgbaColor,&lpfsFillStyle2->rgbaColor)) return 0;
			break;		
	
		case FILLSTYLE_LINEARGRADIENT:
		case FILLSTYLE_RADIALGRADIENT:
			if(!MatchMatrix(&lpfsFillStyle1->mGradientMatrix,&lpfsFillStyle2->mGradientMatrix)) return 0;
			if(!MatchGradient(&lpfsFillStyle1->gGradient,&lpfsFillStyle2->gGradient)) return 0;
			break;

		case FILLSTYLE_REPEATINGBITMAP:
		case FILLSTYLE_CLIPPEDBITMAP:
		case FILLSTYLE_REPEATINGBITMAPNS:
		case FILLSTYLE_CLIPPEDBITMAPNS:
			if(lpfsFillStyle1->usBitmapID!=lpfsFillStyle2->usBitmapID) return 0;
			if(!MatchMatrix(&lpfsFillStyle1->mBitmapMatrix,&lpfsFillStyle2->mBitmapMatrix)) return 0;
			break;
	}

	return 1;
}

UI8 MatchStyles(EDGE *lpeEdge1, EDGE *lpeEdge2, UI8 fFillStyle1, UI8 fLineStyle)
{
	if(!MatchFillStyle(lpeEdge1->lpfsFillStyle0,lpeEdge2->lpfsFillStyle0)) return 0;
	
	if(fFillStyle1)
		if(!MatchFillStyle(lpeEdge1->lpfsFillStyle1,lpeEdge2->lpfsFillStyle1)) return 0;
	
	if(fLineStyle)
	{
		if(lpeEdge1->lplsLineStyle && lpeEdge2->lplsLineStyle)
		{
			if(lpeEdge1->lplsLineStyle->usWidth!=lpeEdge2->lplsLineStyle->usWidth) return 0;
			if(!MatchRGBA(&lpeEdge1->lplsLineStyle->rgbaColor,&lpeEdge2->lplsLineStyle->rgbaColor)) return 0;
		}
		else if(lpeEdge1->lplsLineStyle && !lpeEdge2->lplsLineStyle) return 0;
		else if(!lpeEdge1->lplsLineStyle && lpeEdge2->lplsLineStyle) return 0;
	}
	
	return 1;
}
/*
UI32 Intersect(EDGE *lpeEdge1, EDGE *lpeEdge2)
{
	SI32 slSlope1, slSlope2, slIntercept1, slIntercept2, slIntersectX, slIntersectY;

	slSlope1=(lpeEdge1->slEndY-lpeEdge1->slStartY)/(lpeEdge1->slEndX-lpeEdge1->slStartX);
	slSlope2=(lpeEdge2->slEndY-lpeEdge2->slStartY)/(lpeEdge2->slEndX-lpeEdge2->slStartX);

	slIntercept1=lpeEdge1->slStartY-slSlope1*lpeEdge1->slStartX; 
	slIntercept2=lpeEdge2->slStartY-slSlope2*lpeEdge2->slStartX; 

	slIntersectX=-1*(slIntercept1-slIntercept2)/(slSlope1-slSlope2);
	slIntersectY=slSlope1*slIntersectX+slIntercept1; 
	
	if((lpeEdge1->slStartX-slIntersectX)*(slIntersectX-lpeEdge1->slEndX)<0) return 0;
	if((lpeEdge2->slStartX-slIntersectX)*(slIntersectX-lpeEdge2->slEndX)<0) return 0; 
	if((lpeEdge1->slStartY-slIntersectY)*(slIntersectY-lpeEdge1->slEndY)<0) return 0;
	if((lpeEdge2->slStartY-slIntersectY)*(slIntersectY-lpeEdge2->slEndY)<0) return 0;
	
	return 1;
}

UI32 InPath(EDGE *lpeEdge1, EDGE *lpeEdge2)
{
	EDGE eBezier1a, eBezier1b, eBezier2a, eBezier2b;

	if(!MatchStyles(lpeEdge1,lpeEdge2,false,false)) return 0;

	eBezier1a.slStartX=lpeEdge1->slStartX;
	eBezier1a.slStartY=lpeEdge1->slStartY;
	eBezier1a.slEndX=lpeEdge1->slControlX;
	eBezier1a.slEndY=lpeEdge1->slControlY;

	eBezier1b.slStartX=lpeEdge1->slControlX;
	eBezier1b.slStartY=lpeEdge1->slControlY;
	eBezier1b.slEndX=lpeEdge1->slEndX;
	eBezier1b.slEndY=lpeEdge1->slEndY;
	
	eBezier2a.slStartX=lpeEdge2->slStartX;
	eBezier2a.slStartY=lpeEdge2->slStartY;
	eBezier2a.slEndX=lpeEdge2->slControlX;
	eBezier2a.slEndY=lpeEdge2->slControlY;

	eBezier2b.slStartX=lpeEdge2->slControlX;
	eBezier2b.slStartY=lpeEdge2->slControlY;
	eBezier2b.slEndX=lpeEdge2->slEndX;
	eBezier2b.slEndY=lpeEdge2->slEndY;

	//both are bezier
	if(lpeEdge1->ubType && lpeEdge2->ubType)
	{
		if(Intersect(&eBezier1a,&eBezier2a)) return 1;
		if(Intersect(&eBezier1a,&eBezier2b)) return 1;
		if(Intersect(&eBezier1b,&eBezier2a)) return 1;
		if(Intersect(&eBezier1b,&eBezier2b)) return 1;
	}

	//edge1 is bezier
	else if(lpeEdge1->ubType)
	{
		if(Intersect(&eBezier1a,lpeEdge2)) return 1;
		if(Intersect(&eBezier1b,lpeEdge2)) return 1;
	}

	//edge2 is bezier
	else if(lpeEdge2->ubType)
	{
		if(Intersect(lpeEdge1,&eBezier2a)) return 1;
		if(Intersect(lpeEdge1,&eBezier2b)) return 1;
	}

	//none are bezier
	else if(Intersect(lpeEdge1,lpeEdge2)) return 1;

	return 0;
}
*/

/*Is lpeEdge2 the edge after lpeEdge1*/
UI8 AreAdjecentEdges(EDGE *lpeEdge1, EDGE *lpeEdge2)
{
	SI32 sdAverage;

	/*compare endpoint of edge1 and startpoint of edge2, and their styles*/	
	if(lpeEdge1->slEndX!=lpeEdge2->slStartX) return 0;
	if(lpeEdge1->slEndY!=lpeEdge2->slStartY) return 0;
	if(!MatchStyles(lpeEdge1,lpeEdge2,false,false)) return 0;

	/*average endpoint of edge1 and startpoint of edge2*/
	/*sdAverage=(lpeEdge1->slEndX+lpeEdge2->slStartX)>>1;
	lpeEdge1->slEndX=sdAverage;
	lpeEdge2->slStartX=sdAverage;

	sdAverage=(lpeEdge1->slEndY+lpeEdge2->slStartY)>>1;
	lpeEdge1->slEndY=sdAverage;
	lpeEdge2->slStartY=sdAverage;*/

	return 1;
}

void ReverseEdge(EDGE *lpeEdge)
{
	UI32 slTemp;
	FILLSTYLE *lpfsTemp;

	/*swap start and end points*/
	slTemp=lpeEdge->slStartX;
	lpeEdge->slStartX=lpeEdge->slEndX;
	lpeEdge->slEndX=slTemp;

	slTemp=lpeEdge->slStartY;
	lpeEdge->slStartY=lpeEdge->slEndY;
	lpeEdge->slEndY=slTemp;

	/*swap fills*/
	lpfsTemp=lpeEdge->lpfsFillStyle0;
	lpeEdge->lpfsFillStyle0=lpeEdge->lpfsFillStyle1;
	lpeEdge->lpfsFillStyle1=lpfsTemp;
}

CEdgeList::CEdgeList() 
{
	lpeHead=lpeCurAdd=lpeCurRead=NULL;
	ulNumEdges=0;
}

void CEdgeList::Clear()
{
	EDGE *lpePrevEdge, *lpeCurEdge=lpeHead;
	
	while(lpeCurEdge)
	{
		lpePrevEdge=lpeCurEdge;
		lpeCurEdge=lpeCurEdge->lpeNext;
		delete lpePrevEdge;
	}

	lpeHead=lpeCurAdd=lpeCurRead=NULL;
	ulNumEdges=0;
}

void CEdgeList::DefineShapeToEdges(DEFINESHAPE *lpdsShape)
{
	FILLSTYLE *lprgfsFillStyles;
	LINESTYLE *lprglsLineStyles;
	UI32 slXPosition, slYPosition, slDeltaX, slDeltaY, slControlDeltaX, slControlDeltaY;
	UI32 iFillStyle0=0, iFillStyle1=0, iLineStyle=0;

	/*create edgelist*/
	EDGE eNewEdge;

	slXPosition=lpdsShape->rShapeBounds.slXMin;
	slYPosition=lpdsShape->rShapeBounds.slYMin;

	lprgfsFillStyles=lpdsShape->swsShapeWithStyle.fsaFillStyleArray.lprgfsFillStyles;
	lprglsLineStyles=lpdsShape->swsShapeWithStyle.lsaLineStyleArray.lprglsLineStyles;

	/*read edge records*/
	for(UI32 iCurRecord=0; iCurRecord<lpdsShape->swsShapeWithStyle.ulNumShapeRecords; iCurRecord++)
	{
		void *lpvRecord=lpdsShape->swsShapeWithStyle.lprgvpShapeRecords[iCurRecord];

		/*EDGERECORD*/
		if(((STRAIGHTEDGERECORD*)(lpvRecord))->fTypeFlag)
		{
			/*set start position*/
			memset(&eNewEdge,0,sizeof(EDGE));
			eNewEdge.slStartX=slXPosition;
			eNewEdge.slStartY=slYPosition;
			
			/*CURVEDEDGERECORD*/
			if(((STRAIGHTEDGERECORD*)lpvRecord)->fStraightFlag)
			{
				eNewEdge.ubType=0;
				
				if(((STRAIGHTEDGERECORD*)lpvRecord)->fGeneralLineFlag)
				{		
					slDeltaX=((STRAIGHTEDGERECORD*)lpvRecord)->slGeneralDeltaX;
					slDeltaY=((STRAIGHTEDGERECORD*)lpvRecord)->slGeneralDeltaY;			
					slXPosition+=slDeltaX;
					slYPosition+=slDeltaY;
				}

				else
				{
					if(((STRAIGHTEDGERECORD*)lpvRecord)->fVertLineFlag)
					{						
						slDeltaY=((STRAIGHTEDGERECORD*)lpvRecord)->slVertDeltaY;
						slYPosition+=slDeltaY;
					}
					
					else 
					{
						slDeltaX=((STRAIGHTEDGERECORD*)lpvRecord)->slVertDeltaX;
						slXPosition+=slDeltaX;
					}
				}
			}

			else /*CURVEDEDGERECORD*/
			{
				eNewEdge.ubType=1;

				slControlDeltaX=((CURVEDEDGERECORD*)lpvRecord)->slControlDeltaX;
				slControlDeltaY=((CURVEDEDGERECORD*)lpvRecord)->slControlDeltaY;

				slXPosition+=slControlDeltaX;
				slYPosition+=slControlDeltaY;

				/*set control position*/
				eNewEdge.slControlX=slXPosition;
				eNewEdge.slControlY=slYPosition;

				slDeltaX=((CURVEDEDGERECORD*)lpvRecord)->slAnchorDeltaX;
				slDeltaY=((CURVEDEDGERECORD*)lpvRecord)->slAnchorDeltaY;

				slXPosition+=slDeltaX;
				slYPosition+=slDeltaY;
			}

			/*set end position*/
			eNewEdge.slEndX=slXPosition;
			eNewEdge.slEndY=slYPosition;

			if(iFillStyle0) eNewEdge.lpfsFillStyle0=&lprgfsFillStyles[iFillStyle0-1];
			if(iFillStyle1) eNewEdge.lpfsFillStyle1=&lprgfsFillStyles[iFillStyle1-1];
			if(iLineStyle && lprglsLineStyles) eNewEdge.lplsLineStyle=&lprglsLineStyles[iLineStyle-1];

			AddEdge(&eNewEdge);
		}

		else /*STYLECHANGERECORD*/
		{
			if(((STYLECHANGERECORD*)lpvRecord)->fStateMoveTo)
			{
				slDeltaX=((STYLECHANGERECORD*)lpvRecord)->slMoveDeltaX;
				slDeltaY=((STYLECHANGERECORD*)lpvRecord)->slMoveDeltaY;
				
				slXPosition=slDeltaX;
				slYPosition=slDeltaY;
			}

			if(((STYLECHANGERECORD*)lpvRecord)->fStateNewStyles)
			{
				if(((STYLECHANGERECORD*)lpvRecord)->fsaFillStyleArray.lprgfsFillStyles)
					lprgfsFillStyles=((STYLECHANGERECORD*)lpvRecord)->fsaFillStyleArray.lprgfsFillStyles;
				
				if(((STYLECHANGERECORD*)lpvRecord)->lsaLineStyleArray.lprglsLineStyles)
					lprglsLineStyles=((STYLECHANGERECORD*)lpvRecord)->lsaLineStyleArray.lprglsLineStyles;
			}

			if(((STYLECHANGERECORD*)lpvRecord)->fStateFillStyle0)
				iFillStyle0=((STYLECHANGERECORD*)lpvRecord)->ulFillStyle0;
			
			if(((STYLECHANGERECORD*)lpvRecord)->fStateFillStyle1)
				iFillStyle1=((STYLECHANGERECORD*)lpvRecord)->ulFillStyle1;
							
			if(((STYLECHANGERECORD*)lpvRecord)->fStateLineStyle)
				iLineStyle=((STYLECHANGERECORD*)lpvRecord)->ulLineStyle;
		}
	}
}

void CEdgeList::AddEdge(EDGE *lpeTemplate)
{
	EDGE *lpeNewEdge=new EDGE;
	memcpy(lpeNewEdge,lpeTemplate,sizeof(EDGE));

	lpeNewEdge->sdStartX=double(lpeNewEdge->slStartX)/20;
	lpeNewEdge->sdStartY=double(lpeNewEdge->slStartY)/20;
	lpeNewEdge->sdEndX=double(lpeNewEdge->slEndX)/20;
	lpeNewEdge->sdEndY=double(lpeNewEdge->slEndY)/20;
	lpeNewEdge->sdControlX=double(lpeNewEdge->slControlX)/20;
	lpeNewEdge->sdControlY=double(lpeNewEdge->slControlY)/20;

	if(!lpeHead) 
	{
		lpeHead=lpeNewEdge;
		lpeHead->lpeNext=lpeHead->lpePrev=NULL;
	}

	else 
	{
		lpeCurAdd->lpeNext=lpeNewEdge;
		lpeNewEdge->lpePrev=lpeCurAdd;
		lpeNewEdge->lpeNext=NULL;
	}
			
	lpeCurAdd=lpeNewEdge;
	ulNumEdges++;
}

int CEdgeList::GetFirstEdge(EDGE *lpeDest)
{
	if(!lpeHead) return 0;
	memcpy(lpeDest,lpeHead,sizeof(EDGE));
	return 1;
}

int CEdgeList::GetNextEdge(EDGE *lpeDest)
{
	if(!lpeCurRead) lpeCurRead=lpeHead;
	else lpeCurRead=lpeCurRead->lpeNext;
	if(!lpeCurRead) return 0;
	memcpy(lpeDest,lpeCurRead,sizeof(EDGE));
	return 1;
}

void CEdgeList::MoveEdge(EDGE *lpeMove, EDGE *lpePrev)
{
	if(lpePrev && lpeMove==lpePrev->lpeNext) 
	{
		lpeMove->fMoved=true;
		return;
	}

	/*cut out current edge*/
	if(lpeMove->lpePrev) lpeMove->lpePrev->lpeNext=lpeMove->lpeNext;
	else lpeHead=lpeMove->lpeNext;

	if(lpeMove->lpeNext) lpeMove->lpeNext->lpePrev=lpeMove->lpePrev;
	else lpeCurAdd=lpeMove->lpePrev;

	/*if not to head*/
	if(lpePrev)
	{
		lpeMove->lpePrev=lpePrev;
		lpeMove->lpeNext=lpePrev->lpeNext;

		if(lpePrev->lpeNext) lpePrev->lpeNext->lpePrev=lpeMove;
		lpePrev->lpeNext=lpeMove;
	}

	/*move to head*/
	else
	{
		lpeHead->lpePrev=lpeMove;
		lpeMove->lpePrev=NULL;
		lpeMove->lpeNext=lpeHead;
		lpeHead=lpeMove;
	}

	lpeMove->fMoved=true;
}

void CEdgeList::ConvertToSingleFill()
{
	EDGE *lpeNewEdge, *lpeCurEdge;

	if(!ulNumEdges) return;

	lpeCurEdge=lpeHead;

	while(lpeCurEdge)
	{
		if(lpeCurEdge->lpfsFillStyle1)
		{
			//make a duplicate of this edge and make it single fill
			if(lpeCurEdge->lpfsFillStyle0)
			{
				lpeNewEdge=new EDGE;
				memcpy(lpeNewEdge,lpeCurEdge,sizeof(EDGE));
				lpeNewEdge->lpfsFillStyle1=NULL;
				AddEdge(lpeNewEdge);
			}

			//reverse this edge and make it single fill
			ReverseEdge(lpeCurEdge);
			lpeCurEdge->lpfsFillStyle1=NULL;
		}

		lpeCurEdge=lpeCurEdge->lpeNext;
	}
}

/*
Orgainize the edges by fill style, 
putting them into individual edge lists.
Each list is then sorted.
*/

UI32 CEdgeList::Enumerate(CEdgeList **lplprgelPaths)
{
	EDGE *lpeCur, eTest;
	UI32 ulNumLists=0;

	/*eventually count the # of fill styles to allocate this*/
	CEdgeList *lprgelEdgeLists=new CEdgeList[4096];

	lpeCur=lpeHead;

	UI32 iList;
	while(lpeCur)
	{
		for( iList=0; iList<ulNumLists; iList++)
		{
			lprgelEdgeLists[iList].GetFirstEdge(&eTest);

			if(MatchStyles(lpeCur,&eTest,false,false))
			{
				lprgelEdgeLists[iList].AddEdge(lpeCur);
				break;
			}
		}

		if(iList==ulNumLists)
		{
			lprgelEdgeLists[ulNumLists].AddEdge(lpeCur);
			ulNumLists++;
		}

		lpeCur=lpeCur->lpeNext;
	}

	//Clear();

	for(UI32 iList=0; iList<ulNumLists; iList++)
	{
		lprgelEdgeLists[iList].Sort();
		//while(lprgelEdgeLists[iList].GetNextEdge(&eTest)) AddEdge(&eTest);
	}

	*lplprgelPaths=lprgelEdgeLists;

	return ulNumLists;
}

/*
Sort edges into the proper order to make a single path (fug 63,177)
(bs47 422,307) (fa3 3201) freeze  (mrpg 1052) fug 34= grad
(robocop 14,27,30,33) (sue 325) (sb 380)
seems that one or the other sorts is enough
*/

void CEdgeList::Sort()
{
	EDGE *lpeStart, *lpeEnd, *lpeCur, *lpeTemp;

	lpeStart=lpeHead;
	
	while(lpeStart)
	{
		lpeStart->fMoved=true;
		lpeEnd=lpeStart;

		/*move all edges after end*/
		//lpeCur=lpeHead;
		lpeCur=lpeStart;

		while(lpeCur)
		{
			if(!lpeCur->fMoved && AreAdjecentEdges(lpeEnd,lpeCur))
			{
				MoveEdge(lpeCur,lpeEnd);
				lpeEnd=lpeCur;

				//lpeCur=lpeHead;
			}

			lpeCur=lpeCur->lpeNext;
		}

		/*move all edges before start*/
		/*lpeCur=lpeHead;
		
		while(lpeCur)
		{
			if(!lpeCur->fMoved && AreAdjecentEdges(lpeCur,lpeStart))
			{
				MoveEdge(lpeCur,lpeStart->lpePrev);
				lpeStart=lpeCur;

				lpeCur=lpeHead;
			}

			lpeCur=lpeCur->lpeNext;
		}*/

		lpeStart=lpeEnd->lpeNext;
	}

	return;
}
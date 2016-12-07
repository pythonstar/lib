#ifndef _SVG_H_
#define _SVG_H_

#include "swf.h"
#include "edge.h"

void WriteTransform(MATRIX*,FILE*,UI8 fBitmap=false);
void WriteColorMatrixFilter(CXFORM*,char*,FILE*);
void TransformColor(RGBA*,CXFORM*);
UI32 WriteSVGFillGradient(FILLSTYLE*,char*,FILE*);
UI32 WriteSVGPaths(DEFINESHAPE*,CEdgeList*,UI8,FILE*,PLACEOBJECT2 *lppo2Place=NULL);

#endif
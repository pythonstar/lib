#ifndef _EDGE_H_
#define _EDGE_H_

/*edge*/

struct EDGE
{
	UI8 ubType;			//0=line, 1=curve
	UI8 fMoved;			//has it already been moved
	
	SI32 slStartX;
	SI32 slStartY;
	SI32 slControlX;
	SI32 slControlY;
	SI32 slEndX;
	SI32 slEndY;

	double sdStartX;
	double sdStartY;
	double sdControlX;
	double sdControlY;
	double sdEndX;
	double sdEndY;

	FILLSTYLE *lpfsFillStyle0;
	FILLSTYLE *lpfsFillStyle1;
	LINESTYLE *lplsLineStyle;
	EDGE *lpePrev;
	EDGE *lpeNext;
};


UI8 MatchStyles(EDGE*,EDGE*,UI8,UI8);
UI32 Intersect(EDGE*,EDGE*);
UI32 InPath(EDGE*,EDGE*);
UI8 AreAdjecentEdges(EDGE*,EDGE*);

class CEdgeList
{
public:
	CEdgeList();
	~CEdgeList() {Clear();}

	void DefineShapeToEdges(DEFINESHAPE*);
	void AddEdge(EDGE*);
	int GetFirstEdge(EDGE*);
	int GetNextEdge(EDGE*);
	void ConvertToSingleFill();
	UI32 Enumerate(CEdgeList**);
	void MoveEdge(EDGE*,EDGE*);
	void Sort();
	void Clear();

private:
	EDGE *lpeHead, *lpeCurAdd, *lpeCurRead;
	UI32 ulNumEdges;
};

#endif
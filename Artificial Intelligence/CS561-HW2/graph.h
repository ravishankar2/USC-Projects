/*----------------------------------------------------------*/
/*	Author:			Yu Sun									*/
/*															*/
/*	Date:			02/22/2013								*/
/*															*/
/*	Description:	Graph (Adjacency List)					*/
/*					Header File								*/
/*----------------------------------------------------------*/
#ifndef _GRAPH_H_
#define _GRAPH_H_

#define MAX_VERTEX_NUM 100

class ArcNode
{
public:
	ArcNode(int v, int t, int r) : adjvex(v), _time(t), risk(r) {}
	/* data */
	int adjvex; // indicate the index of vertex
	int _time;
	int risk;
	class ArcNode * nextArc;
};

template <class DataType>
class VexNode
{
public:
	/* data */
	DataType data;
	class ArcNode * firstArc;
};

template <class DataType>
class Graph
{
public:
	void Initial();
	int LocateVex(DataType);
	DataType GetValue(int);
	bool InsertVex(DataType);
	bool InsertArc(DataType, DataType, int ,int );
	ArcNode * FirstArc(DataType);
	ArcNode * FirstArc(int);
	ArcNode * NextArc(ArcNode *);
	/* data */
	VexNode <DataType> vertices[MAX_VERTEX_NUM];
	int vexNum; // number of vertexs
	int arcNum; // number of arc
};

#endif /*_GRAPH_H_*/

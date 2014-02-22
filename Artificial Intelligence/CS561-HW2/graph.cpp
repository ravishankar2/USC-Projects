/*----------------------------------------------------------*/
/*	Author:			Yu Sun									*/
/*															*/
/*	Date:			02/22/2013								*/
/*															*/
/*	Description:	Graph (Adjacency List)					*/
/*					CPP File								*/
/*----------------------------------------------------------*/
#include "graph.h"

template <class DataType>
void Graph <DataType> :: Initial()
{
	vexNum = 0;
	arcNum = 0;
	for(int i = 0; i < MAX_VERTEX_NUM; i++)
		vertices[i].firstArc = 0;
}

template <class DataType>
int Graph <DataType> :: LocateVex(DataType data)
{
	for(int i = 0; i < vexNum; i++)
		if(vertices[i].data == data)
			return i;
	return -1;
}

template <class DataType>
DataType Graph <DataType> :: GetValue(int index)
{
	if(index >= vexNum)
		return 0;
	return vertices[index].data;
}

template <class DataType>
bool Graph <DataType> :: InsertVex(DataType data)
{
	if(LocateVex(data) != -1)
		return false; // the vertex already exists
	vertices[vexNum++].data = data;
	return true;
}

template <class DataType>
bool Graph <DataType> :: InsertArc(DataType data1, DataType data2, int _time, int risk)
{
	ArcNode * arc = new ArcNode(LocateVex(data2), _time, risk);
	if(!arc)
		return false;
	int vertexIndex = LocateVex(data1);
	ArcNode * nextArc = vertices[vertexIndex].firstArc;
	vertices[vertexIndex].firstArc = arc;
	arc -> nextArc = nextArc;
	arcNum++;
	return true;
}

template <class DataType>
ArcNode * Graph <DataType> :: FirstArc(DataType data)
{
	int index = LocateVex(data);
	if(index == -1)
		return 0;
	return vertices[index].firstArc;
}

template <class DataType>
ArcNode * Graph <DataType> :: FirstArc(int vex)
{
	if(vex >= vexNum)
		return 0;
	return vertices[vex].firstArc;
}

template <class DataType>
ArcNode * Graph <DataType> :: NextArc(ArcNode * arc)
{
	if(!arc)
		return 0;
	return arc -> nextArc;
}





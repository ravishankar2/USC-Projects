#include "graph.h"

template <class T>
bool graph <T> :: connected(T a, T b, int & t, int & r)
{
	int _v = find_node(a);
	graph_arc <T> * _arc = _nodes[_v]._first_arc;
	while(_arc)
	{
		if(b == _arc -> _data)
		{
			t = _arc -> _time;
			r = _arc -> _risk;
			return true;
		}
		_arc = _arc -> _next_arc;
	}
	return false;
}

template <class T>
graph_arc <T> * graph <T> :: first_arc(int n)
{
	return _nodes[n]._first_arc;
}

template <class T>
graph_arc <T> * graph <T> :: next_arc(graph_arc <T> * a)
{
	return a -> _next_arc;
}

template <class T>
bool graph <T> :: insert_arc(T v, T a, int t, int r)
{
	graph_arc <T> * _arc_new = new graph_arc <T> ();
	_arc_new -> _data = a;
	_arc_new -> _time = t;
	_arc_new -> _risk = r;
	_arc_new -> _next_arc = 0;
	if(!_arc_new)
		return false;
	int _num = find_node(v);
	graph_arc <T> * _arc = _nodes[_num]._first_arc;
	if(!_arc)
		_nodes[_num]._first_arc = _arc_new;
	else
	{
		while(_arc -> _next_arc)
			_arc = _arc -> _next_arc;
		_arc -> _next_arc = _arc_new;
	}
	return true;
}

template <class T>
bool graph <T> :: append_node(T v)
{
	if(find_node(v) != -1)
		return false;
	_nodes[_sum]._data = v;
	_sum++;
	return true;
}

template <class T>
void graph <T> :: initial()
{
	_sum = 0;
	for(int i = 0; i < GRAPH_MAX_NODE; i++)
		_nodes[i]._first_arc = 0;
}

template <class T>
int graph <T> :: find_node(T v)
{
	for(int i = 0; i < _sum; i++)
		if(_nodes[i]._data == v)
			return i;
	return -1;
}
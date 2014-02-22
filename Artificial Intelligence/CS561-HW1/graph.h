// adjacency list graph
#define GRAPH_MAX_NODE 1000

template <class T>
class graph_arc
{
public:
	T _data;
	int _time;
	int _risk;
	class graph_arc <T> * _next_arc;
};

template <class T>
class graph_node
{
public:
	T _data;
	class graph_arc <T> * _first_arc;
};

template <class T>
class graph
{
public:
	int _sum;
	graph_node <T> _nodes[GRAPH_MAX_NODE];
	void initial();
	bool append_node(T v);
	bool insert_arc(T v, T a, int t, int r);
	int find_node(T v);
	void clean();
	bool connected(T a, T b, int & t, int & r);
	graph_arc <T> * first_arc(int n);
	graph_arc <T> * next_arc(graph_arc <T> * a);
};



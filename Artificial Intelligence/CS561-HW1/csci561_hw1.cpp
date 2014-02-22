#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <queue>
#include <stack>
#include <list>

#include "friends.h";
#include "graph.cpp"

using namespace std;

int debug = 0;

void error(string error)
{
	cerr << "Error: " << error << ", program abort!" << endl;
	exit(1);
}

void traversal(graph <string> & _graph)
{
	cout << "Graph:" << endl;
	for(int i = 0; i < _graph._sum; i++)
	{
		cout << _graph._nodes[i]._data;
		graph_arc <string> * _arc = _graph._nodes[i]._first_arc;
		while(_arc)
		{
			cout << " -> " << _arc -> _data;
			_arc = _arc -> _next_arc;
		}
		cout << endl;
	}
}

template <class T>
void clean(stack <T *> & _clean)
{
	while(!_clean.empty())
	{
		T * _temp = _clean.top();
		_clean.pop();
		delete _temp;
	}
}

void initial_visited(graph <string> & _graph, map <string, int> & _visited)
{
	for(int i = 0; i < _graph._sum; i++)
		_visited[_graph._nodes[i]._data] = 0;
}

void print_visited(graph <string> & _graph, map <string, int> & _visited)
{
	cout << "Visited:" << endl;
	for(int i = 0; i < _graph._sum; i++)
		cout << _graph._nodes[i]._data << ":" << _visited[_graph._nodes[i]._data] << " | ";
	cout << endl;
}

void print_path(fpath * _fpath, int _type)
{
	string _file_name = _type == BFS ? "breadth-first" : "depth-first";
	_file_name += ".result.txt";
	ofstream _ofstream;
    _ofstream.open(_file_name.c_str());
	stack <string> _stack;
	while(_fpath)
	{
		_stack.push(_fpath -> _name);
		_fpath = _fpath -> _last;
	}
	cout << _stack.top();
	_ofstream << _stack.top();
	_stack.pop();
	while(!_stack.empty())
	{
		cout << "-" << _stack.top();
		_ofstream << "-" << _stack.top();
		_stack.pop();
	}
	cout << endl;
	_ofstream << endl;
	_ofstream.close();
}

void breadth_first_search(graph <string> & _graph, string _start, string _target)
{
	int _start_num = _graph.find_node(_start);
	if(_start_num == -1)
		return;
	// set the visited table, avoid visiting repeat
	map <string, int> _visited;
	initial_visited(_graph, _visited);
	_visited[_start]++;
	// using stack to clean memory
	stack <fpath *> _clean = stack <fpath *> ();
	// using fpath to trace the path through friend
	fpath * _fpath = new fpath(_start);
	_clean.push(_fpath);
	// using queue to implement BFS
	queue <fpath *> _queue = queue <fpath *> ();
	_queue.push(_fpath);
	while(!_queue.empty())
	{
		fpath * _fcur = _queue.front();
		_queue.pop();
		string _cur = _fcur -> _name;
		if(_cur == _target)
		{
			cout << "Find " << _cur << " in BFS:" << endl;
			print_path(_fcur, BFS);
			return;
		}
		int _cur_num = _graph.find_node(_cur);
		graph_arc <string> * _arc = _graph._nodes[_cur_num]._first_arc;
		while(_arc)
		{
			string _cur_sub = _arc -> _data;
			if(_visited[_cur_sub] == 0)
			{
				_visited[_cur_sub]++;
				fpath * _fpath_sub = new fpath(_cur_sub);
				_clean.push(_fpath_sub);
				_fpath_sub -> _last = _fcur;
				_queue.push(_fpath_sub);
				if(debug)
					print_visited(_graph, _visited);
			}
			_arc = _arc -> _next_arc;
		}
	}
	clean(_clean);
	error("Cannot find " + _target);
}

void depth_first_search(graph <string> & _graph, string _start, string _target)
{
	int _start_num = _graph.find_node(_start);
	if(_start_num == -1)
		return;
	// set the visited table, avoid visiting repeat
	map <string, int> _visited;
	initial_visited(_graph, _visited);
	_visited[_start]++;
	if(_start == _target)
	{
		cout << "Find " << _target << " in DFS:" << endl;
		return;
	}
	// using stack to clean memory
	stack <fpath *> _clean = stack <fpath *> ();
	// using fpath to trace the path through friend
	fpath * _fpath = new fpath(_start);
	_clean.push(_fpath);
	// using stack to implement DFS
	stack <fpath *> _stack = stack <fpath *> ();
	_stack.push(_fpath);
	while(!_stack.empty())
	{
		fpath * _fcur = _stack.top();
		_stack.pop();
		string _cur = _fcur -> _name;
		int _cur_num = _graph.find_node(_cur);
		graph_arc <string> * _arc = _graph._nodes[_cur_num]._first_arc;
		while(_arc)
		{
			string _cur_sub = _arc -> _data;
			if(_visited[_cur_sub] == 0)
			{
				_visited[_cur_sub]++;
				fpath * _fpath_sub = new fpath(_cur_sub);
				_stack.push(_fpath_sub);
				_fpath_sub -> _last = _fcur;
				if(_cur_sub == _target)
				{
					cout << "Find " << _cur_sub << " in DFS:" << endl;
					print_path(_fpath_sub, DFS);
					return;
				}
				_stack.push(_fpath_sub);
				if(debug)
					print_visited(_graph, _visited);
			}
			_arc = _arc -> _next_arc;
		}
	}
	clean(_clean);
	error("Cannot find " + _target);
}

bool exist_list(list <fcost *> & _list, string _target)
{
	list <fcost *> :: iterator _itr;
	for(_itr = _list.begin(); _itr != _list.end(); _itr++)
		if((*_itr) -> _name == _target)
			return true;
	return false;
}

void sort_insert_list(list <fcost *> & _list, string _child, int _cost, stack <fcost *> & _clean)
{
	list <fcost *> :: iterator _itr;
	fcost * _fcost = new fcost(_child, _cost);
	_clean.push(_fcost);
	for(_itr = _list.begin(); _itr != _list.end(); _itr++)
		if(_fcost -> _cost < (*_itr) -> _cost)
			break;
	_list.insert(_itr, _fcost);
}

int cost(graph_arc <string> * _arc, int _cost_type)
{
	if(_cost_type == Time)
		return _arc -> _time;
	if(_cost_type == Risk)
		return _arc -> _risk;
	return 0;
}

void replace_insert_list(list <fcost *> & _list, string _child, int _cost, stack <fcost *> & _clean)
{
	list <fcost *> :: iterator _itr;
	for(_itr = _list.begin(); _itr != _list.end(); _itr++)
		if((*_itr) -> _name == _child)
		{
			if((*_itr) -> _cost > _cost)
			{
				_list.erase(_itr);
				sort_insert_list(_list, _child, _cost, _clean);
			}
			break;
		}
}

void replace_insert_list(list <fcost *> & _open, list <fcost *> & _closed, string _child, int _cost, stack <fcost *> & _clean)
{
	list <fcost *> :: iterator _itr;
	for(_itr = _closed.begin(); _itr != _closed.end(); _itr++)
		if((*_itr) -> _name == _child)
		{
			if((*_itr) -> _cost > _cost)
			{
				_closed.erase(_itr);
				sort_insert_list(_open, _child, _cost, _clean);
			}
			break;
		}
}

void print_list(list <fcost *> & _list, string _list_type)
{
	cout << _list_type << ':' << endl;
	cout << "State\t" << "Cost" << endl;
	list <fcost *> :: iterator _itr;
	for(_itr = _list.begin(); _itr != _list.end(); _itr++)
		cout << (*_itr) -> _name << '\t' << (*_itr) -> _cost << endl;
}

void print_cost_path(graph <string> & _graph, list <fcost *> & _closed, string _target, int _cost_type)
{
	stack <string> _stack = stack <string> ();
	_stack.push(_target);
	list <fcost *> :: iterator _itr_search = _closed.end();
	list <fcost *> :: iterator _itr_anchor = _itr_search;
	while(_itr_search != _closed.begin())
	{
		if((*_itr_search) -> _name == _target)
			break;
		_itr_search--;
		_itr_anchor = _itr_search;
	}
	if(_itr_search != _closed.begin())
		_itr_search--;
	while(_itr_search != _closed.begin())
	{
		int _time, _risk;
		if(_graph.connected((*_itr_anchor) -> _name, (*_itr_search) -> _name, _time, _risk))
		{
			int _cost = (_cost_type == 0 ? _time : _risk);
			if(((*_itr_anchor) -> _cost - _cost) == (*_itr_search) -> _cost)
			{
				_stack.push((*_itr_search) -> _name);
				_itr_anchor = _itr_search;
			}
		}
		_itr_search--;
	}

	string _file_name = "uniform-cost.";
	_file_name += _cost_type == Time ? "time" : "risk";
	_file_name += ".result.txt";
	ifstream _ifstream;
	ofstream _ofstream;
	_ofstream.open(_file_name.c_str());
	cout << (*_closed.begin()) -> _name;
	_ofstream << (*_closed.begin()) -> _name;
	while(!_stack.empty())
	{
		cout << "-" << _stack.top();
		_ofstream << "-" << _stack.top();
		_stack.pop();
	}
	cout << endl;
	_ofstream << endl;
	_ofstream.close();
}

void uniform_cost_search(graph <string> & _graph, string _start, string _target, int _cost_type)
{
	int _start_num = _graph.find_node(_start);
	if(_start_num == -1)
		return;
	if(_start == _target)
	{
		cout << "Find " << _target << " in UCS:" << endl;
		return;
	}
	// using stack to clean memory
	stack <fcost *> _clean = stack <fcost *> ();
	fcost * _fcost = new fcost(_start, 0);
	_clean.push(_fcost);
	// using open list and closed list to implement Uniform-Cost Search
	list <fcost *> _open = list <fcost *> ();
	_open.push_back(_fcost);
	list <fcost *> _closed = list <fcost *> ();
	while(!_open.empty())
	{
		fcost * _currnode = _open.front();
		_open.pop_front();
		if(_currnode -> _name == _target)
			cout << "Find " << _currnode -> _name << " in UCS:(" << (_cost_type == 0 ? "Time" : "Risk") << ")" << endl;
		int _currnode_num = _graph.find_node(_currnode -> _name);
		int _currnode_cost = _currnode -> _cost;
		graph_arc <string> * _arc_child = _graph.first_arc(_currnode_num);
		queue <graph_arc <string> *> _children = queue <graph_arc <string> *> ();
		while(_arc_child)
		{
			_children.push(_arc_child);
			_arc_child = _graph.next_arc(_arc_child);
		}
		while(!_children.empty())
		{
			_arc_child = _children.front();
			_children.pop();
			bool _in_open = exist_list(_open, _arc_child -> _data);
			bool _in_closed = exist_list(_closed, _arc_child -> _data);
			int _cost_child = _currnode_cost + cost(_arc_child, _cost_type);
			if(!_in_open && !_in_closed)
				sort_insert_list(_open, _arc_child -> _data, _cost_child, _clean);
			else if(_in_open)
				replace_insert_list(_open, _arc_child -> _data, _cost_child, _clean);
			else if(_in_closed) // really need this step?
				replace_insert_list(_open, _closed, _arc_child -> _data, _cost_child, _clean);
		}
		_closed.push_back(_currnode);
		if(debug)
		{
			print_list(_open, "Open");
			print_list(_closed, "Closed");
		}
	}
	print_cost_path(_graph, _closed, _target, _cost_type);
	clean(_clean);
}

void read_text(string & _text, string & _friend1, string & _friend2, int & _time, int & _risk)
{
	stringstream str(_text);
	string _sub_str;
	getline(str, _sub_str, ' ');
	_friend1 = _sub_str;
	getline(str, _sub_str, ' ');
	_friend2 = _sub_str;
	getline(str, _sub_str, ' ');
	_time = atoi(_sub_str.c_str());
	getline(str, _sub_str, ' ');
	_risk = atoi(_sub_str.c_str());
}

void read_file(string & _file_name, vector <friends> & _friends)
{
	string _friend1, _friend2;
	int _time, _risk;
	ifstream infile(_file_name.c_str());
	if(!infile)
		error("cannot open file");
	string _text;
	while(getline(infile, _text, '\n'))
	{
		read_text(_text, _friend1, _friend2, _time, _risk);
		friends _friends_temp = friends(_friend1, _friend2, _time, _risk);
		_friends.push_back(_friends_temp);
	}
	infile.close();
}

void fill_graph(graph <string> & _graph, vector <friends> & _friends)
{
	for(unsigned int i = 0; i < _friends.size(); i++)
	{
		_graph.append_node(_friends[i]._friend1);
		_graph.append_node(_friends[i]._friend2);
		_graph.insert_arc(_friends[i]._friend1, _friends[i]._friend2, _friends[i]._time, _friends[i]._risk);
		_graph.insert_arc(_friends[i]._friend2, _friends[i]._friend1, _friends[i]._time, _friends[i]._risk);
	}
}

void process(string _file_name, string _start, string _target)
{
	// read data from txt file and store them to vector
	vector <friends> _friends;
	read_file(_file_name, _friends);
	// using graph to represent the data
	graph <string> _graph = graph <string> ();
	_graph.initial();
	fill_graph(_graph, _friends);
	if(debug)
		traversal(_graph);
	breadth_first_search(_graph, _start, _target);
	depth_first_search(_graph, _start, _target);
	uniform_cost_search(_graph, _start, _target, Time);
	uniform_cost_search(_graph, _start, _target, Risk);
}

void command(string & _start, string & _target, string & _file, int _argc, char * _argv[])
{
	if(_argc >= 3 && _argc <= 4)
	{
		_start = string(_argv[1]);
		_target = string(_argv[2]);
		_file = "social-network.txt";
		if(_argc == 4)
			_file = string(_argv[3]);
	}
	else
	{
		_start = "Alice";
		_target = "Noah";
		_file = "social-network.txt";
	}
}

int main(int argc, char * argv[])
{
	string _file, _start, _target;
	command(_start, _target, _file, argc, argv);
	process(_file, _start, _target);
	return 0;
}



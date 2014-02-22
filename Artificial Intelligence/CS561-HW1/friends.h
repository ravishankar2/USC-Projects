using namespace std;

enum Cost
{
	Time,
	Risk
};

enum SearchType
{
	BFS,
	DFS
};

class friends
{
public:
	friends(string f1, string f2, int t, int r) : _friend1(f1), _friend2(f2), _time(t), _risk(r) {}
	string _friend1;
	string _friend2;
	int _time;
	int _risk;
};

class fcost
{
public:
	fcost(string f, int c) : _name(f), _cost(c) {}
	string _name;
	int _cost;
};

class fpath
{
public:
	fpath(string f) : _name(f), _last(0) {}
	string _name;
	fpath * _last;
};



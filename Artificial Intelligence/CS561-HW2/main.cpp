/*----------------------------------------------------------*/
/*	Author:			Yu Sun									*/
/*															*/
/*	Date:			02/22/2013								*/
/*															*/
/*	Description:	CSCI561 HW2								*/
/*					CPP File                                */
/*----------------------------------------------------------*/
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <stack>
#include <list>

#include "csci561_hw2.h"
#include "graph.cpp"

using namespace std;
/*----------------------------------------------------------*/
/*						Global 								*/
/*----------------------------------------------------------*/
string file_network;// =	"social-network-updated.txt";
string file_heuristic;// =	"direct-time-risk.txt";
string person_start;// =	"Alice";
string person_target;// =	"Noah";

map <string, int> heuristic_risk_map;
map <string, int> heuristic_time_map;

// map <string, int> cost_time_map;
// map <string, int> cost_risk_map;

int DEBUG = 0;

/*----------------------------------------------------------*/
/*						Utilities 							*/
/*----------------------------------------------------------*/
void errorMessage(string message)
{
	cerr << "Error: " << message << ", program abort!" << endl;
	exit(1);
}

void printTitle(string message)
{
	cout << "------------------------------------------" << endl;
	cout << message  << endl;
	cout << "------------------------------------------" << endl;
}

void traversalGraph(Graph <string> & graph)
{
	printTitle("Graph");
	for(int i = 0; i < graph.vexNum; i++)
	{
		cout << graph.vertices[i].data;
		ArcNode * arc = graph.vertices[i].firstArc;
		while(arc)
		{
			cout << " -> " << graph.GetValue(arc -> adjvex);
			arc = arc -> nextArc;
		}
		cout << endl;
	}
}

void printSocialNet(vector <SocialNet> & social_net)
{
	printTitle("Social Network");
	for(vector <SocialNet> :: iterator it = social_net.begin(); it != social_net.end(); it++)
		cout << it -> name1 << " " << it -> name2 << " " << it -> _time << " " << it -> risk << endl;
}

void printSocialHeuristic(vector <SocialHeuristic> & social_heuristic)
{
	printTitle("Social Heuristic");
	for(vector <SocialHeuristic> :: iterator it = social_heuristic.begin(); it != social_heuristic.end(); it++)
		cout << it -> name << " " << it -> _time << " " << it -> risk << endl;
}

void printOpenList(list <SocialHeuristic *> & open_list)
{
	printTitle("Open List");
	for(list <SocialHeuristic *> :: iterator it = open_list.begin(); it != open_list.end(); it++)
		cout << (*it) -> name << " ";
	cout << endl << endl;
}

/*----------------------------------------------------------*/
/*						Commandline 						*/
/*----------------------------------------------------------*/
void commandline(char * argv[]) {
	file_network = string(argv[1]);
	file_heuristic = string(argv[2]);
	person_start = string(argv[3]);
	person_target = string(argv[4]);
}

/*----------------------------------------------------------*/
/*						Read File 							*/
/*----------------------------------------------------------*/
void readLine(string & line, string & name1, string & name2, int & _time, int & risk)
{
	stringstream str(line);
	string value;
	getline(str, value, ' ');
	name1 = value;
	getline(str, value, ' ');
	name2 = value;
	getline(str, value, ' ');
	_time = atoi(value.c_str());
	getline(str, value, ' ');
	risk = atoi(value.c_str());
}

void readLine(string & line, string & name, int & _time, int & risk)
{
	stringstream str(line);
	string value;
	getline(str, value, ' ');
	name = value;
	getline(str, value, ' ');
	_time = atoi(value.c_str());
	getline(str, value, ' ');
	risk = atoi(value.c_str());
}

void readNetFile(string & file_name, vector <SocialNet> & social_net)
{
	string name1, name2;
	int _time, risk;
	ifstream infile(file_name.c_str());
	if(!infile)
		errorMessage("cannot open file");
	string line;
	while(getline(infile, line, '\n'))
	{
		readLine(line, name1, name2, _time, risk);
		SocialNet relation = SocialNet(name1, name2, _time, risk);
		social_net.push_back(relation);
	}
	infile.close();
}

void readHeuristicFile(string & file_name, vector <SocialHeuristic> & social_heuristic)
{
	string name;
	int _time, risk;
	ifstream infile(file_name.c_str());
	if(!infile)
		errorMessage("cannot open file");
	string line;
	while(getline(infile, line, '\n'))
	{
		readLine(line, name, _time, risk);
		SocialHeuristic heuristic = SocialHeuristic(name, _time, risk);
		social_heuristic.push_back(heuristic);
	}
	infile.close();
}

/*----------------------------------------------------------*/
/*						Graph 								*/
/*----------------------------------------------------------*/
void createGraph(Graph <string> & graph, vector <SocialNet> & social_net)
{
	for(vector <SocialNet> :: iterator it = social_net.begin(); it != social_net.end(); it++)
	{
		graph.InsertVex(it -> name1);
		graph.InsertVex(it -> name2);
		graph.InsertArc(it -> name1, it -> name2, it -> _time, it -> risk);
		graph.InsertArc(it -> name2, it -> name1, it -> _time, it -> risk);
	}
}

/*----------------------------------------------------------*/
/*						Map 		 						*/
/*----------------------------------------------------------*/
// create the heuristic map
void createMap(vector <SocialHeuristic> & social_heuristic)
{
	for(vector<SocialHeuristic> :: iterator it = social_heuristic.begin(); it != social_heuristic.end(); it++)
	{
		string name = it -> name;
		heuristic_risk_map[name] = it -> risk;
		heuristic_time_map[name] = it -> _time;
	}
}
// update the cost map
// void updateCostMap(string name, string pname, int _time, int risk)
// {
// 	cost_time_map[name] = cost_time_map[pname] + _time;
// 	cost_risk_map[name] = cost_risk_map[pname] + risk;
// }

int getHeurTime(string name) {return heuristic_time_map[name];}

int getHeurRisk(string name) {return heuristic_risk_map[name];}

// int getCostTime(string name) {return cost_time_map[name];}

// int getCostRisk(string name) {return cost_risk_map[name];}

/*----------------------------------------------------------*/
/*						List 		 						*/
/*----------------------------------------------------------*/
void insertHeurList(list <SocialHeuristic *> & open_list, SocialHeuristic * heuristic, int cost_type)
{
	list <SocialHeuristic *> :: iterator it;
	for(it = open_list.begin(); it != open_list.end(); it++)
	{
		if(cost_type == Time)
			if(heuristic -> _time < (*it) -> _time)
				break;
		if(cost_type == Risk)
			if(heuristic -> risk < (*it) -> risk)
				break;
	}
	open_list.insert(it, heuristic);
	if(DEBUG)
		printOpenList(open_list);
}

bool checkIsRetrieved(list <SocialHeuristic *> & close_list, string name)
{
	list <SocialHeuristic *> :: iterator it;
	for(it = close_list.begin(); it != close_list.end(); it++)
		if((*it) -> name == name)
			return true;
	return false;
}

void cleanList(list <SocialHeuristic *> & clean_list)
{
	list <SocialHeuristic *> :: iterator it;
	for(it = clean_list.begin(); it != clean_list.end(); it++)
		free(*it);
}

void printListPath(SocialHeuristic * heuristic, string search, int cost_type)
{
	search += cost_type == Time ? ".time.result.txt" : ".risk.result.txt";
	string result = "";
	printTitle(search);
	stack <string> path_stack;
	while(heuristic != 0)
	{
		path_stack.push(heuristic -> name);
		heuristic = heuristic -> lastNode;
	}
	if(!path_stack.empty())
	{
		string name = path_stack.top();
		path_stack.pop();
		result += name;
		cout << name;
	}
	while(!path_stack.empty())
	{
		string name = path_stack.top();
		path_stack.pop();
		result += "-";
		result += name;
		cout << "-" << name;
	}
	cout << endl << endl;

	string file_name = search;
	ifstream _ifstream;
	ofstream _ofstream;
	_ofstream.open(file_name.c_str());
	_ofstream << result;
	_ofstream << endl;
	_ofstream.close();
}

/*----------------------------------------------------------*/
/*						Greedy Search  						*/
/*----------------------------------------------------------*/
void greedySearch(Graph <string> & graph, int cost_type)
{
	// retrieval current nodes
	list <SocialHeuristic *> open_list;
	// store retrieved nodes
	list <SocialHeuristic *> close_list;
	// store all nodes to clean up memory
	list <SocialHeuristic *> clean_list;
	SocialHeuristic * start = new SocialHeuristic(person_start, getHeurTime(person_start), getHeurRisk(person_start));
	open_list.push_back(start);
	clean_list.push_back(start);
	while(!open_list.empty())
	{
		// get the first node in the list (has the smallest cost)
		SocialHeuristic * heuristic = open_list.front();
		open_list.pop_front();
		// get name and heuristic cost
		string name = heuristic -> name;
		// push the node into close to avoid retrieval again
		close_list.push_back(heuristic);
		// push all the child node (in order)
		ArcNode * arc = graph.FirstArc(name);
		// check whether find the target
		bool found = false;
		while(arc != 0)
		{
			name = graph.GetValue(arc -> adjvex);
			// if not retrieved, push into open list
			if(!checkIsRetrieved(close_list, name))
			{
				SocialHeuristic * heuristic_child = new SocialHeuristic(name, getHeurTime(name), getHeurRisk(name));
				heuristic_child -> lastNode = heuristic;
				insertHeurList(open_list, heuristic_child, cost_type);
				clean_list.push_back(heuristic_child);
				// find the target
				if(name == person_target)
				{
					found = true;
					printListPath(heuristic_child, "Greedy", cost_type);
					break;
				}
			}
			arc = arc -> nextArc;
		}
		if(found)
			break;
	}
	cleanList(clean_list);
}

/*----------------------------------------------------------*/
/*						A* Search 	 						*/
/*----------------------------------------------------------*/
void aStarSearch(Graph <string> & graph, int cost_type)
{
	// retrieval current nodes
	list <SocialHeuristic *> open_list;
	// store retrieved nodes
	list <SocialHeuristic *> close_list;
	// store all nodes to clean up memory
	list <SocialHeuristic *> clean_list;
	// record the cost have paid

	// cost_time_map[person_start] = 0;
	// cost_risk_map[person_start] = 0;

	SocialHeuristic * start = new SocialHeuristic(person_start, 0, 0);
	open_list.push_back(start);
	clean_list.push_back(start);
	while(!open_list.empty())
	{
		// get the first node in the list (has the smallest cost)
		SocialHeuristic * heuristic = open_list.front();
		open_list.pop_front();
		// get name and heuristic cost
		string pname = heuristic -> name;
		// push the node into close to avoid retrieval again
		close_list.push_back(heuristic);
		// push all the child node (in order)
		ArcNode * arc = graph.FirstArc(pname);
		// check whether find the target
		bool found = false;
		while(arc != 0)
		{
			string name = graph.GetValue(arc -> adjvex);
			// if not retrieved, push into open list
			if(!checkIsRetrieved(close_list, name))
			{
				int time_cost = heuristic -> time_cost + arc -> _time;
				int risk_cost = heuristic -> risk_cost + arc -> risk;
				SocialHeuristic * heuristic_child = new SocialHeuristic(name, getHeurTime(name) + time_cost, getHeurRisk(name) + risk_cost);
				heuristic_child -> time_cost = time_cost;
				heuristic_child -> risk_cost = risk_cost;
				heuristic_child -> lastNode = heuristic;
				insertHeurList(open_list, heuristic_child, cost_type);
				clean_list.push_back(heuristic_child);
				// find the target
				if(name == person_target)
				{
					found = true;
					printListPath(heuristic_child, "A-star", cost_type);
					break;
				}
			}
			arc = arc -> nextArc;
		}
		if(found)
			break;
	}
	cleanList(clean_list);
}

/*----------------------------------------------------------*/
/*						Process 							*/
/*----------------------------------------------------------*/
void process()
{
	vector <SocialNet> social_net;
	vector <SocialHeuristic> social_heuristic;
	// read data from file and store to vector
	readNetFile(file_network, social_net);
	readHeuristicFile(file_heuristic, social_heuristic);
	if(DEBUG)
		printSocialNet(social_net);
	if(DEBUG)
		printSocialHeuristic(social_heuristic);

	// using graph to represent the whole social network
	Graph <string> graph = Graph <string> ();
	graph.Initial();
	createGraph(graph, social_net);
	if(DEBUG)
		traversalGraph(graph);
	// create heuristic map which can find the heuristic in O(1)
	createMap(social_heuristic);

	greedySearch(graph, Time);
	greedySearch(graph, Risk);
	aStarSearch(graph, Time);
	aStarSearch(graph, Risk);
}

/*----------------------------------------------------------*/
/*						Main 								*/
/*----------------------------------------------------------*/
int main(int argc, char * argv[])
{
	if(argc == 5) {
		commandline(argv);
	}
	else {
		errorMessage("Please input the arguments as following guideline:\
			\n\t1.Filename for thenetwork, e.g, social-network-updated.txt;\
			\n\t2.File name for the time and risk estimates, e.g, direct-time-risk.txt;\
			\n\t3.Name of the start node, e.g, Alice;\n\t4.Name of the goal node, e.g, Noah");
	}
	process();
	return 0;
}




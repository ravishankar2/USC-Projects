/*----------------------------------------------------------*/
/*	Author:			Yu Sun									*/
/*															*/
/*	Date:			02/22/2013								*/
/*															*/
/*	Description:	CSCI561 HW2								*/
/*					Header File                             */
/*----------------------------------------------------------*/
#ifndef _CSCI561_HW2_H_
#define _CSCI561_HW2_H_

using namespace std;

class SocialNet
{
public:
	SocialNet(string n1, string n2, int t, int r) : name1(n1), name2(n2), _time(t), risk(r) {}
	/* data */
	string name1;
	string name2;
	int _time;
	int risk;
};

class SocialHeuristic
{
public:
	SocialHeuristic(string n, int t, int r) : name(n), _time(t), risk(r), time_cost(0), risk_cost(0), lastNode(0) {}
	/* data */
	string name;
	int _time;
	int risk;
	int time_cost;
	int risk_cost;
	class SocialHeuristic * lastNode;
};

enum CostType {Time, Risk};

#endif /*_CSCI561_HW2_H_*/




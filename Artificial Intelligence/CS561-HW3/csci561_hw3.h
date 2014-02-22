/*----------------------------------------------------------*/
/*	Author:			Yu Sun									*/
/*															*/
/*	Date:			04/6/2013								*/
/*															*/
/*	Description:	CSCI561 HW3								*/
/*					Header File                             */
/*----------------------------------------------------------*/
#ifndef _CSCI561_HW3_H_
#define _CSCI561_HW3_H_
#include <vector>

using namespace std;

enum Mode {EXP1, EXP2, EXP3, TEST};

Mode MODE;

/*
 For Experiment 1
*/
static const int M_EXP1 = 16;
static const int N_EXP1 = 2;

static int R_EXP1[M_EXP1 * M_EXP1];
static bool Model_EXP1[M_EXP1 * N_EXP1];
/*
 For Experiment 2
*/
static const int M_EXP2 = 16;
static const int N_EXP2 = 2;

static int R_EXP2[M_EXP2 * M_EXP2];
static bool Model_EXP2[M_EXP2 * N_EXP2];
/*
 For Experiment 3
*/
static const int M_EXP3_1 = 16;
static const int N_EXP3_1 = 2;
static int R_EXP3_1[M_EXP3_1 * M_EXP3_1];
static bool Model_EXP3_1[M_EXP3_1 * N_EXP3_1];

static const int M_EXP3_2 = 24;
static const int N_EXP3_2 = 3;
static int R_EXP3_2[M_EXP3_2 * M_EXP3_2];
static bool Model_EXP3_2[M_EXP3_2 * N_EXP3_2];

static const int M_EXP3_3 = 32;
static const int N_EXP3_3 = 4;
static int R_EXP3_3[M_EXP3_3 * M_EXP3_3];
static bool Model_EXP3_3[M_EXP3_3 * N_EXP3_3];

static const int M_EXP3_4 = 40;
static const int N_EXP3_4 = 5;
static int R_EXP3_4[M_EXP3_4 * M_EXP3_4];
static bool Model_EXP3_4[M_EXP3_4 * N_EXP3_4];

static const int M_EXP3_5 = 48;
static const int N_EXP3_5 = 6;
static int R_EXP3_5[M_EXP3_5 * M_EXP3_5];
static bool Model_EXP3_5[M_EXP3_5 * N_EXP3_5];

static const int M_EXP3_6 = 56;
static const int N_EXP3_6 = 7;
static int R_EXP3_6[M_EXP3_6 * M_EXP3_6];
static bool Model_EXP3_6[M_EXP3_6 * N_EXP3_6];
/*
 For Execute
*/
static int M;
static int N;
static int *R;
static bool *Model;
/*
 For Debug
*/
static const int M_Debug = 16;						// Number of guests
static const int N_Debug = 3;						// Number of tables
static int R_Debug[M_Debug * M_Debug];				// M by M matrix R
static bool Model_Debug[M_Debug * N_Debug];			// Randomly true/false symbol
static int Debug = 0;								// Turn on the debug mode
static int Experiment_Debug = 0;					// Turn on the debug mode in experiment
static int Test_PL_RESOLUTION = 0;					// Turn on to check PL_RESOLUTION
static int Test_PL_WALK_SAT = 1;					// Turn on to check WALK_SAT

class Literal {
public:
	Literal(bool n, int g, int s) : negative(n), guest(g), seat(s) {}
	bool Negative() {return negative;}
	int Guest() {return guest;}
	int Seat() {return seat;}
private:
	/* data */
	bool	negative;
	int		guest;
	int		seat;
};

static vector <vector <Literal> > CNF;
#endif /*_CSCI561_HW3_H_*/
/*----------------------------------------------------------*/
/*	Author:			Yu Sun									*/
/*															*/
/*	Date:			05/27/2013								*/
/*															*/
/*	Description:	CSCI561 HW4								*/
/*					Header File                             */
/*----------------------------------------------------------*/
#ifndef _CSCI561_HW4_H_
#define _CSCI561_HW4_H_
#include <vector>

using namespace std;

#define DEBUG 1
#define PROCESS 1
#define POSITIVE_INFINITY 9999
#define NEGATIVE_INFINITY -9999
#define GAME_GRID_NUM 8
#define ALICE_MOVE 1
#define BILL_MOVE 2
#define JUMP_MOVE 1
#define NORMAL_MOVE 2
#define MAX_SEARCH_DEPTH 4
#define KING_VALUE 2
// ‘O’ denotes an empty dark square where a piece can be placed.
#define EMPTY_DARK_SQUARE 0
// ‘+’ denotes a light-shaded square where no piece can be placed.
#define LIGHT_SHADED_SQUARE -1
// ‘A’ denotes Alice’s piece.
#define ALICE_PIECE 1
// ‘B’ denotes Bill’s piece.
#define BILL_PIECE 2
// ‘k’ denotes Alice’s king piece.
#define ALICE_KING 3
// ‘K’ denotes Bill’s king piece
#define BILL_KING 4

class Point {
public:
	Point() {}
	Point(int x, int y) : i(x), j(y) {}

	int i;
	int j;
};

Point mid_point(Point p1, Point p2) {
	Point mid = Point();
	mid.i = (p1.i + p2.i) / 2;
	mid.j = (p1.j + p2.j) / 2;
	return mid;
}

class Operation {
public:
	Operation() {}
	Operation(Point p1, Point p2, int m, int t) {
		from.i = p1.i;
		from.j = p1.j;
		to.i = p2.i;
		to.j = p2.j;
		move = m;
		type = t;
	}
	void set(Point p1, Point p2, int m, int t) {
		from.i = p1.i;
		from.j = p1.j;
		to.i = p2.i;
		to.j = p2.j;
		move = m;
		type = t;
	}

	int move;
	int type;
	Point from;
	Point to;
};

class Game {
public:
	Game() { next = 0; }
	void init(Game g) {
		for(int i = 0; i < GAME_GRID_NUM; i++) {
			for(int j = 0; j < GAME_GRID_NUM; j++) {
				config[i][j] = g.config[i][j];
			}
		}
	}
	int get(Point p) { return config[p.i][p.j]; }
	void set(Point p, int s) {
		config[p.i][p.j] = s;
	}

	int config[GAME_GRID_NUM][GAME_GRID_NUM];
	class Game *next;
};

class Game *First_Game;

#endif /*_CSCI561_HW4_H_*/





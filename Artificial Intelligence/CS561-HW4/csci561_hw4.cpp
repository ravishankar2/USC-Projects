/*
 Author:			Yu Sun

 Date:				05/27~28/2013

 Description:		CSCI561 HW4
 					CPP File
*/

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <stdio.h>

#include "csci561_hw4.h"

using namespace std;
/********************************************************************************************************
 Debug & Utility
********************************************************************************************************/
char convert_symbol(int val);
static string output_file;
static string output_result;

void print_point(Point &p) {
	cout << "(" << p.i << ", " << p.j << ")" << flush;
	stringstream s1, s2;
	s1 << p.i;
	s2 << p.j;
	output_result = output_result + "(" + s1.str() + ", " + s2.str() + ")";
}

void print_operation(Operation &op) {
	print_point(op.from);
	cout << "=>";
	output_result += "=>";
	print_point(op.to);
}

void print_game_config() {
	cout << endl;
	Game *game = First_Game;
	int no = 0;
	while(game != 0) {
		cout << "case " << ++no << endl;
		for(int i = 0; i < GAME_GRID_NUM; i++) {
			for(int j = 0; j < GAME_GRID_NUM; j++) {
				cout << convert_symbol(game->config[i][j]) << " ";
			}
			cout << endl;
		}
		cout << endl;
		game = game->next;
	}
}

void print_game_config(int case_no) {
	cout << endl;
	Game *game = First_Game;
	int no = 0;
	while(game != 0) {
		++no;
		if(case_no != no) {
			game = game->next;
			continue;
		}
		cout << "case " << no << endl;
		for(int i = 0; i < GAME_GRID_NUM; i++) {
			for(int j = 0; j < GAME_GRID_NUM; j++) {
				cout << convert_symbol(game->config[i][j]) << " ";
			}
			cout << endl;
		}
		cout << endl;
		game = game->next;
	}
}

void error(string message) {
	cerr << "Error: " << message << ", program abort!" << endl;
	exit(1);
}

void print_first_move(Operation &decision) {
	output_result += "first move: ";
	cout << "first move: ";
	print_operation(decision);
	output_result += ".\n\n";
	cout << "." << endl;
}

void print_process(Operation &op, int move, int it) {
	int i = it;
	while(--i > 0) { 
		cout << "|-";
		output_result += "|-";
	}
	if(move == ALICE_MOVE) {
		cout << "A" << it << ": "; 
		stringstream ss;
		ss << it;
		output_result = output_result + "A" + ss.str() + ": ";
	}
	if(move == BILL_MOVE) { 
		cout << "B" << it << ": ";
		stringstream ss;
		ss << it;
		output_result = output_result + "B" + ss.str() + ": ";
	}

	print_operation(op);
	output_result += ".\n";
	cout << "." << endl;
}

void print_process(Operation &op, int move, int it, int utility) {
	int i = it;
	while(--i > 0) { 
		cout << "|-"; 
		output_result += "|-";
	}
	if(move == ALICE_MOVE) { 
		cout << "A" << it << ": "; 
		stringstream ss;
		ss << it;
		output_result = output_result + "A" + ss.str() + ": ";
	}
	if(move == BILL_MOVE) { 
		cout << "B" << it << ": "; 
		stringstream ss;
		ss << it;
		output_result = output_result + "B" + ss.str() + ": ";
	}
	print_operation(op);
	if(utility < POSITIVE_INFINITY && utility > NEGATIVE_INFINITY) {
		cout << "; h=" << utility << "." << endl;
		stringstream ss;
		ss << utility;
		output_result = output_result + "; h=" + ss.str() + ".\n";
	}
	if(utility == POSITIVE_INFINITY) {
		cout << "; h=+INFINITY." << endl;
		output_result = output_result + "; h=+INFINITY.\n";
	}
	if(utility == NEGATIVE_INFINITY) {
		cout << "; h=-INFINITY." << endl;
		output_result = output_result + "; h=-INFINITY.\n";
	}
}

void print_pruning(vector <Operation> &op_list, vector <Operation>::iterator op, int move, int it, int alpha, int beta) {
	if(op == op_list.end())
		return;
	int i = it;
	while(--i > 0) { 
		cout << "|-";
		output_result += "|-";
	}
	if(move == ALICE_MOVE) { 
		cout << "A" << it << ": "; 
		stringstream ss;
		ss << it;
		output_result = output_result + "A" + ss.str() + ": ";
	}
	if(move == BILL_MOVE) { 
		cout << "B" << it << ": "; 
		stringstream ss;
		ss << it;
		output_result = output_result + "B" + ss.str() + ": ";
	}
	cout << "pruning ";
	output_result += "pruning ";
	while(op != op_list.end()) {
		print_operation(*op);
		++op;
		if(op == op_list.end()) {
			cout << "; ";
			output_result += "; ";
		}
		else {
			cout << ", ";
			output_result += ", ";
		}
	}
	if(alpha < POSITIVE_INFINITY && alpha > NEGATIVE_INFINITY) {
		cout << "alpha=" << alpha;
		stringstream ss;
		ss << alpha;
		output_result = output_result + "alpha=" + ss.str();
	}
	if(alpha == POSITIVE_INFINITY) {
		cout << "alpha=+INFINITY";
		output_result = output_result + "alpha=+INFINITY";
	}
	if(alpha == NEGATIVE_INFINITY) {
		cout << "alpha=-INFINITY";
		output_result = output_result + "alpha=-INFINITY";
	}

	if(beta < POSITIVE_INFINITY && beta > NEGATIVE_INFINITY) {
		cout << ", beta=" << beta << "." << endl;
		stringstream ss;
		ss << alpha;
		output_result = output_result + ", beta=" + ss.str() + ".\n";
	}
	if(beta == POSITIVE_INFINITY) {
		cout << ", beta=+INFINITY." << endl;
		output_result = output_result + ", beta=+INFINITY.\n";
	}
	if(beta == NEGATIVE_INFINITY) {
		cout << ", beta=-INFINITY." << endl;
		output_result = output_result + ", beta=-INFINITY.\n";
	}
}

/********************************************************************************************************
 File
********************************************************************************************************/
void output(string str) {
	ifstream _ifstream;
	ofstream _ofstream;
	_ofstream.open(output_file.c_str(), ios::app);
	_ofstream << str;
	_ofstream.close();
}

int convert_symbol(char ch) {
	int ret;
	switch(ch) {
		case 'O':
			ret = EMPTY_DARK_SQUARE;
			break;
		case '+':
			ret = LIGHT_SHADED_SQUARE;
			break;
		case 'A':
			ret = ALICE_PIECE;
			break;
		case 'B':
			ret = BILL_PIECE;
			break;
		case 'k':
			ret = ALICE_KING;
			break;
		case 'K':
			ret = BILL_KING;
			break;
		default:
			error("undefined symbol");
			break;
	}
	return ret;
}

char convert_symbol(int val) {
	char ret;
	switch(val) {
		case EMPTY_DARK_SQUARE:
			ret = 'O';
			break;
		case LIGHT_SHADED_SQUARE:
			ret = '+';
			break;
		case ALICE_PIECE:
			ret = 'A';
			break;
		case BILL_PIECE:
			ret = 'B';
			break;
		case ALICE_KING:
			ret = 'k';
			break;
		case BILL_KING:
			ret = 'K';
			break;
		default:
			error("undefined symbol");
			break;
	}
	return ret;
}

void read_line(string & line, Game *config, int i_no)
{
	for(int i = 0; i < GAME_GRID_NUM; i++) {
		char ch = line[i];
		int val = convert_symbol(ch);
		config->config[i_no][i] = val;
	}
}

void read_file(string & file) {
	ifstream infile(file.c_str());
	if(!infile)
		error("cannot open file");
	string line;
	int no = 0;
	Game *last_game = 0;
	while(getline(infile, line, '\n')) {
		if(line == "")
			continue;
		if(line.substr(0, 4) == "case") {
			Game *game = new Game();
			// link
			if(last_game != 0)
				last_game->next = game;
			// no = atoi(line.substr(5, 6).c_str());
			if(no++ == 0)
				First_Game = game;
			// read one game config
			for(int i = 0; i < GAME_GRID_NUM; i++) {
				getline(infile, line, '\n');
				read_line(line, game, i);
			}
			last_game = game;
		}
	}
	infile.close();
}

/********************************************************************************************************
 Command
********************************************************************************************************/
void command(char *argv[]) {
	string file = string(argv[1]);
	// read from file, and save to link list
	read_file(file);
	output_file = string(argv[2]);
}

/********************************************************************************************************
 Game Play
********************************************************************************************************/
bool check_top_right_jump(Game *game, Point &start, Operation &op);
bool check_top_left_jump(Game *game, Point &start, Operation &op);
bool check_bot_right_jump(Game *game, Point &start, Operation &op);
bool check_bot_left_jump(Game *game, Point &start, Operation &op);
bool check_top_right_move(Game *game, Point &start, Operation &op);
bool check_top_left_move(Game *game, Point &start, Operation &op);
bool check_bot_right_move(Game *game, Point &start, Operation &op);
bool check_bot_left_move(Game *game, Point &start, Operation &op);
/*
 Return utility value
*/
int utility(Game *game) {
	int alice_pieces = 0;
	int bill_pieces = 0;
	for(int i = 0; i < GAME_GRID_NUM; i++) {
		for(int j = 0; j < GAME_GRID_NUM; j++) {
			if(game->config[i][j] == ALICE_PIECE)
				alice_pieces++;
			if(game->config[i][j] == BILL_PIECE)
				bill_pieces++;
			if(game->config[i][j] == ALICE_KING)
				alice_pieces += KING_VALUE;
			if(game->config[i][j] == BILL_KING)
				bill_pieces += KING_VALUE;
		}
	}
	// Alice lose
	if(alice_pieces == 0) {
		return NEGATIVE_INFINITY;
	}
	// Alice Win
	if(bill_pieces == 0) {
		return POSITIVE_INFINITY;
	}
	return alice_pieces - bill_pieces;
}

/*
 Test terminal
*/
bool terminal_test(Game *game, int & utility, int move, int it) {
	int alice_pieces = 0;
	int bill_pieces = 0;
	for(int i = 0; i < GAME_GRID_NUM; i++) {
		for(int j = 0; j < GAME_GRID_NUM; j++) {
			if(game->config[i][j] == ALICE_PIECE)
				alice_pieces++;
			if(game->config[i][j] == BILL_PIECE)
				bill_pieces++;
			if(game->config[i][j] == ALICE_KING)
				alice_pieces += KING_VALUE;
			if(game->config[i][j] == BILL_KING)
				bill_pieces += KING_VALUE;
		}
	}
	// Alice lose
	if(alice_pieces == 0) {
		int i = it;
		while(--i > 0) { 
			cout << "|-"; 
			output_result += "|-";
		}
		if(move == ALICE_MOVE) { 
			cout << "A" << it << ": "; 
			stringstream ss;
			ss << it;
			output_result = output_result + "A" + ss.str() + ": ";
		}
		if(move == BILL_MOVE) { 
			cout << "B" << it << ": ";
			stringstream ss;
			ss << it;
			output_result = output_result + "B" + ss.str() + ": ";
		}
		cout << "h=-INFINITY." << endl;
		output_result += "h=-INFINITY.\n";
		utility = NEGATIVE_INFINITY;
		return true;
	}
	// Alice Win
	if(bill_pieces == 0) {
		int i = it;
		while(--i > 0) { 
			cout << "|-"; 
			output_result += "|-";
		}
		if(move == ALICE_MOVE) { 
			cout << "A" << it << ": "; 
			stringstream ss;
			ss << it;
			output_result = output_result + "A" + ss.str() + ": ";
		}
		if(move == BILL_MOVE) { 
			cout << "B" << it << ": "; 
			stringstream ss;
			ss << it;
			output_result = output_result + "B" + ss.str() + ": ";
		}
		cout << "h=+INFINITY." << endl;
		output_result += "h=+INFINITY.\n";
		utility = POSITIVE_INFINITY;
		return true;
	}
	// check eligible move
	if(move == ALICE_MOVE) {
		bool no_move = true;
		Operation op;
		for(int i = 0; i < GAME_GRID_NUM; i++) {
			for(int j = 0; j < GAME_GRID_NUM; j++) {
				Point start = Point(i, j);
				if(game->get(start) == ALICE_KING || game->get(start) == ALICE_PIECE) {
					if(!check_top_right_jump(game, start, op)
					 &&!check_top_left_jump(game, start, op)
					 &&!check_bot_right_jump(game, start, op)
					 &&!check_bot_left_jump(game, start, op)
					 &&!check_top_right_move(game, start, op)
					 &&!check_top_left_move(game, start, op)
					 &&!check_bot_right_move(game, start, op)
					 &&!check_bot_left_move(game, start, op)) {
					}
					else { no_move = false; }
				}
				if(no_move == false)
					break;
			}
			if(no_move == false)
				break;
		}
		if(no_move) {
			int i = it;
			while(--i > 0) { 
				cout << "|-"; 
				output_result += "|-";
			}
			cout << "A" << it << ": ";
			stringstream ss;
			ss << it;
			output_result = output_result + "A" + ss.str() + ": ";
			cout << "h=-INFINITY." << endl;
			output_result += "h=-INFINITY.\n";
			utility = NEGATIVE_INFINITY;
			return true;
		}
	}
	if(move == BILL_MOVE) {
		bool no_move = true;
		Operation op;
		for(int i = 0; i < GAME_GRID_NUM; i++) {
			for(int j = 0; j < GAME_GRID_NUM; j++) {
				Point start = Point(i, j);
				if(game->get(start) == BILL_KING || game->get(start) == BILL_PIECE) {
					if(!check_top_right_jump(game, start, op)
					 &&!check_top_left_jump(game, start, op)
					 &&!check_bot_right_jump(game, start, op)
					 &&!check_bot_left_jump(game, start, op)
					 &&!check_top_right_move(game, start, op)
					 &&!check_top_left_move(game, start, op)
					 &&!check_bot_right_move(game, start, op)
					 &&!check_bot_left_move(game, start, op)) {
					}
					else { no_move = false; }
				}
				if(no_move == false)
					break;
			}
			if(no_move == false)
				break;
		}
		if(no_move) {
			int i = it;
			while(--i > 0) { 
				cout << "|-"; 
				output_result += "|-";
			}
			cout << "B" << it << ": ";
			stringstream ss;
			ss << it;
			output_result = output_result + "B" + ss.str() + ": ";
			cout << "h=+INFINITY." << endl;
			output_result += "h=+INFINITY.\n";
			utility = POSITIVE_INFINITY;
			return true;
		}
	}
	utility = alice_pieces - bill_pieces;
	return false;
}

/*
 Top Right Jump
*/
bool check_top_right_jump(Game *game, Point &start, Operation &op) {
	// check eligible for jumping
	if(start.j <= (GAME_GRID_NUM - 3) && start.i >= 2) {
		Point jp = Point(start.i - 2, start.j + 2);
		// the jump piece can be placed
		if(game->get(jp) == EMPTY_DARK_SQUARE) {
			Point p = Point(start.i - 1, start.j + 1);
			// Alice jump
			if(game->get(start) == ALICE_PIECE || game->get(start) == ALICE_KING) {
				// eliminate Bill's piece
				if(game->get(p) == BILL_PIECE || game->get(p) == BILL_KING) {
					op.set(start, jp, ALICE_MOVE, JUMP_MOVE);
					return true;
				}
			}
			// Bill jump
			if(game->get(start) == BILL_KING) {
				// eliminate Alice's piece
				if(game->get(p) == ALICE_PIECE || game->get(p) == ALICE_KING) {
					op.set(start, jp, BILL_MOVE, JUMP_MOVE);
					return true;
				}
			}
		}
	}
	return false;
}

/*
 Top Left Jump
*/
bool check_top_left_jump(Game *game, Point &start, Operation &op) {
	// check eligible for jumping
	if(start.j >= 2 && start.i >= 2) {
		Point jp = Point(start.i - 2, start.j - 2);
		// the jump piece can be placed
		if(game->get(jp) == EMPTY_DARK_SQUARE) {
			Point p = Point(start.i - 1, start.j - 1);
			// Alice jump
			if(game->get(start) == ALICE_PIECE || game->get(start) == ALICE_KING) {
				// eliminate Bill's piece
				if(game->get(p) == BILL_PIECE || game->get(p) == BILL_KING) {
					op.set(start, jp, ALICE_MOVE, JUMP_MOVE);
					return true;
				}
			}
			// Bill jump
			if(game->get(start) == BILL_KING) {
				// eliminate Alice's piece
				if(game->get(p) == ALICE_PIECE || game->get(p) == ALICE_KING) {
					op.set(start, jp, BILL_MOVE, JUMP_MOVE);
					return true;
				}
			}
		}
	}
	return false;
}

/*
 Bottom Right Jump
*/
bool check_bot_right_jump(Game *game, Point &start, Operation &op) {
	// check eligible for jumping
	if(start.j <= (GAME_GRID_NUM - 3) && start.i <= (GAME_GRID_NUM - 3)) {
		Point jp = Point(start.i + 2, start.j + 2);
		// the jump piece can be placed
		if(game->get(jp) == EMPTY_DARK_SQUARE) {
			Point p = Point(start.i + 1, start.j + 1);
			// Alice jump
			if(game->get(start) == ALICE_KING) {
				// eliminate Bill's piece
				if(game->get(p) == BILL_PIECE || game->get(p) == BILL_KING) {
					op.set(start, jp, ALICE_MOVE, JUMP_MOVE);
					return true;
				}
			}
			// Bill jump
			if(game->get(start) == BILL_KING || game->get(start) == BILL_PIECE) {
				// eliminate Alice's piece
				if(game->get(p) == ALICE_PIECE || game->get(p) == ALICE_KING) {
					op.set(start, jp, BILL_MOVE, JUMP_MOVE);
					return true;
				}
			}
		}
	}
	return false;
}

/*
 Bottom Left Jump
*/
bool check_bot_left_jump(Game *game, Point &start, Operation &op) {
	// check eligible for jumping
	if(start.j >= 2 && start.i <= (GAME_GRID_NUM - 3)) {
		Point jp = Point(start.i + 2, start.j - 2);
		// the jump piece can be placed
		if(game->get(jp) == EMPTY_DARK_SQUARE) {
			Point p = Point(start.i + 1, start.j - 1);
			// Alice jump
			if(game->get(start) == ALICE_KING) {
				// eliminate Bill's piece
				if(game->get(p) == BILL_PIECE || game->get(p) == BILL_KING) {
					op.set(start, jp, ALICE_MOVE, JUMP_MOVE);
					return true;
				}
			}
			// Bill jump
			if(game->get(start) == BILL_KING || game->get(start) == BILL_PIECE) {
				// eliminate Alice's piece
				if(game->get(p) == ALICE_PIECE || game->get(p) == ALICE_KING) {
					op.set(start, jp, BILL_MOVE, JUMP_MOVE);
					return true;
				}
			}
		}
	}
	return false;
}

/*
 Check Jump, store the eligible jump movement to op_list
*/
bool check_jump(Game *game, int move, vector <Operation> &op_list) {
	bool ret = false;
	if(move == ALICE_MOVE) {
		for(int i = 0; i < GAME_GRID_NUM; i++) {
			for(int j = 0; j < GAME_GRID_NUM; j++) {
				Point start = Point(i, j);
				if(game->get(start) == ALICE_KING || game->get(start) == ALICE_PIECE) {
					Operation op = Operation();
					if(check_top_right_jump(game, start, op)) {
						op_list.push_back(op);
						ret = true;
					}
					if(check_top_left_jump(game, start, op)) {
						op_list.push_back(op);
						ret = true;
					}
					// king can go back
					if(game->get(start) == ALICE_KING) {
						if(check_bot_right_jump(game, start, op)) {
							op_list.push_back(op);
							ret = true;
						}
						if(check_bot_left_jump(game, start, op)) {
							op_list.push_back(op);
							ret = true;
						}
					}
				}
			}
		}
	}
	if(move == BILL_MOVE) {
		for(int i = 0; i < GAME_GRID_NUM; i++) {
			for(int j = 0; j < GAME_GRID_NUM; j++) {
				Point start = Point(i, j);
				if(game->get(start) == BILL_KING || game->get(start) == BILL_PIECE) {
					Operation op = Operation();
					if(check_bot_right_jump(game, start, op)) {
						op_list.push_back(op);
						ret = true;
					}
					if(check_bot_left_jump(game, start, op)) {
						op_list.push_back(op);
						ret = true;
					}
					// king can go back
					if(game->get(start) == BILL_KING) {
						if(check_top_right_jump(game, start, op)) {
							op_list.push_back(op);
							ret = true;
						}
						if(check_top_left_jump(game, start, op)) {
							op_list.push_back(op);
							ret = true;
						}
					}
				}
			}
		}
	}
	return ret;
}

/*
 Top Right Move
*/
bool check_top_right_move(Game *game, Point &start, Operation & op) {
	// check eligible for moving
	if(start.j <= (GAME_GRID_NUM - 2) && start.i >= 1) {
		Point mp = Point(start.i - 1, start.j + 1);
		// the move piece can be placed
		if(game->get(mp) == EMPTY_DARK_SQUARE) {
			// Alice move
			if(game->get(start) == ALICE_PIECE || game->get(start) == ALICE_KING) {
				op.set(start, mp, ALICE_MOVE, NORMAL_MOVE);
				return true;
			}
			// Bill move
			if(game->get(start) == BILL_KING) {
				op.set(start, mp, BILL_MOVE, NORMAL_MOVE);
				return true;
			}
		}
	}
	return false;
}

/*
 Top Left Move
*/
bool check_top_left_move(Game *game, Point &start, Operation & op) {
	// check eligible for moving
	if(start.j >= 1 && start.i >= 1) {
		Point mp = Point(start.i - 1, start.j - 1);
		// the move piece can be placed
		if(game->get(mp) == EMPTY_DARK_SQUARE) {
			// Alice move
			if(game->get(start) == ALICE_PIECE || game->get(start) == ALICE_KING) {
				op.set(start, mp, ALICE_MOVE, NORMAL_MOVE);
				return true;
			}
			// Bill move
			if(game->get(start) == BILL_KING) {
				op.set(start, mp, BILL_MOVE, NORMAL_MOVE);
				return true;
			}
		}
	}
	return false;
}

/*
 Bottom Right Move
*/
bool check_bot_right_move(Game *game, Point &start, Operation & op) {
	// check eligible for moving
	if(start.j <= (GAME_GRID_NUM - 2) && start.i <= (GAME_GRID_NUM - 2)) {
		Point mp = Point(start.i + 1, start.j + 1);
		// the move piece can be placed
		if(game->get(mp) == EMPTY_DARK_SQUARE) {
			// Alice move
			if(game->get(start) == ALICE_KING) {
				op.set(start, mp, ALICE_MOVE, NORMAL_MOVE);
				return true;
			}
			// Bill move
			if(game->get(start) == BILL_KING || game->get(start) == BILL_PIECE) {
				op.set(start, mp, BILL_MOVE, NORMAL_MOVE);
				return true;
			}
		}
	}
	return false;
}

/*
 Bottom Left Move
*/
bool check_bot_left_move(Game *game, Point &start, Operation & op) {
	// check eligible for moving
	if(start.j >= 1 && start.i <= (GAME_GRID_NUM - 2)) {
		Point mp = Point(start.i + 1, start.j - 1);
		// the move piece can be placed
		if(game->get(mp) == EMPTY_DARK_SQUARE) {
			// Alice move
			if(game->get(start) == ALICE_KING) {
				op.set(start, mp, ALICE_MOVE, NORMAL_MOVE);
				return true;
			}
			// Bill move
			if(game->get(start) == BILL_KING || game->get(start) == BILL_PIECE) {
				op.set(start, mp, BILL_MOVE, NORMAL_MOVE);
				return true;
			}
		}
	}
	return false;
}

/*
 Check Move, store the eligible move movement to op_list
*/
void check_move(Game *game, int move, vector <Operation> & op_list) {
	if(move == ALICE_MOVE) {
		for(int i = 0; i < GAME_GRID_NUM; i++) {
			for(int j = 0; j < GAME_GRID_NUM; j++) {
				Point start = Point(i, j);
				if(game->get(start) == ALICE_KING || game->get(start) == ALICE_PIECE) {
					Operation op = Operation();
					if(check_top_right_move(game, start, op)) {
						op_list.push_back(op);
					}
					if(check_top_left_move(game, start, op)) {
						op_list.push_back(op);
					}
					// king can go back
					if(game->get(start) == ALICE_KING) {
						if(check_bot_right_move(game, start, op)) {
							op_list.push_back(op);
						}
						if(check_bot_left_move(game, start, op)) {
							op_list.push_back(op);
						}
					}
				}
			}
		}
	}
	if(move == BILL_MOVE) {
		for(int i = 0; i < GAME_GRID_NUM; i++) {
			for(int j = 0; j < GAME_GRID_NUM; j++) {
				Point start = Point(i, j);
				if(game->get(start) == BILL_KING || game->get(start) == BILL_PIECE) {
					Operation op = Operation();
					if(check_bot_right_move(game, start, op)) {
						op_list.push_back(op);
					}
					if(check_bot_left_move(game, start, op)) {
						op_list.push_back(op);
					}
					// king can go back
					if(game->get(start) == BILL_KING) {
						if(check_top_right_move(game, start, op)) {
							op_list.push_back(op);
						}
						if(check_top_left_move(game, start, op)) {
							op_list.push_back(op);
						}
					}
				}
			}
		}
	}
}

/*
 Apply the eligible movement
*/
void apply_movement(Game *game, Operation &op) {
	if(op.type == NORMAL_MOVE) {
		int original = game->get(op.from);
		game->set(op.from, EMPTY_DARK_SQUARE);
		game->set(op.to, original);
	}
	if(op.type == JUMP_MOVE) {
		int original = game->get(op.from);
		game->set(op.from, EMPTY_DARK_SQUARE);
		game->set(mid_point(op.from, op.to), EMPTY_DARK_SQUARE);
		game->set(op.to, original);
	}
}

/*
 function MINMAX-VALUE(state, game) returns a utility value
     if TERMINAL-TEST[game](state) then
          return UTILITY[game](state)
     else if MAX is to move in state then
          return the highest MINMAX-VALUE of SUCCESSORS(state)
     else
          return the lowest MINMAX-VALUE of SUCCESSORS(state)

 it: the depth of iteration
 move: decide min or max
*/
int minmax_value(Game *game, int move, int it, int alpha, int beta) {
	int _utility;
	++it;
	if(terminal_test(game, _utility, move, it)) {
		return _utility;
	}
	if(it > MAX_SEARCH_DEPTH)
		return _utility;
	vector <Operation> op_list;
	// max
	if(move == ALICE_MOVE) {
		// int max = NEGATIVE_INFINITY;
		// mandatory jump
		if(!check_jump(game, ALICE_MOVE, op_list)) {
			// normal move
			check_move(game, ALICE_MOVE, op_list);
		}
		for(vector <Operation>::iterator op = op_list.begin(); op != op_list.end(); ++op) {
			Game temp_game = Game();
			temp_game.init(*game);
			apply_movement(&temp_game, *op);
			if(it == MAX_SEARCH_DEPTH) {
				if(PROCESS)
					print_process(*op, ALICE_MOVE, it, utility(&temp_game));
			}
			else {
				if(PROCESS)
					print_process(*op, ALICE_MOVE, it);
			}
			int value = minmax_value(&temp_game, BILL_MOVE, it, alpha, beta);
			if(value > alpha) {
				alpha = value;
			}
			if(alpha >= beta) {
				print_pruning(op_list, ++op, ALICE_MOVE, it, alpha, beta);
				return beta;
			}
		}
		_utility = alpha;
	}
	// min
	if(move == BILL_MOVE) {
		// int min = POSITIVE_INFINITY;
		// mandatory jump
		if(!check_jump(game, BILL_MOVE, op_list)) {
			// normal move
			check_move(game, BILL_MOVE, op_list);
		}
		for(vector <Operation>::iterator op = op_list.begin(); op != op_list.end(); ++op) {
			Game temp_game = Game();
			temp_game.init(*game);
			apply_movement(&temp_game, *op);
			if(it == MAX_SEARCH_DEPTH) {
				if(PROCESS)
					print_process(*op, BILL_MOVE, it, utility(&temp_game));
			}
			else {
				if(PROCESS)
					print_process(*op, BILL_MOVE, it);
			}
			int value = minmax_value(&temp_game, ALICE_MOVE, it, alpha, beta);
			if(value < beta) {
				beta = value;
			}
			if(alpha >= beta) {
				print_pruning(op_list, ++op, BILL_MOVE, it, alpha, beta);
				return alpha;
			}
		}
		_utility = beta;
	}
	return _utility;
}

/*
 function MINMAX-DECISION(game) return an operator
     for each op in OPERATORS[game] do
          VALUE[op] <- MINMAX-VALUE(APPLY(op, game), game)
     end
     return the op with the highest VALUE[op]
*/
Operation alpha_beta(Game *game) {
	// int max = NEGATIVE_INFINITY;
	vector <Operation> op_list;
	Operation decision;
	int alpha = NEGATIVE_INFINITY;
	int beta = POSITIVE_INFINITY;
	// mandatory jump
	if(!check_jump(game, ALICE_MOVE, op_list)) {
		// normal move
		check_move(game, ALICE_MOVE, op_list);
	}
	for(vector <Operation>::iterator op = op_list.begin(); op != op_list.end(); ++op) {
		Game temp_game = Game();
		temp_game.init(*game);
		apply_movement(&temp_game, *op);
		if(PROCESS)
			print_process(*op, ALICE_MOVE, 1);
		int value = minmax_value(&temp_game, BILL_MOVE, 1, alpha, beta);
		if(value > alpha) {
			alpha = value;
			decision = *op;
		}
		// no use here
		if(alpha > beta)
			return decision;
	}
	print_first_move(decision);
	return decision;
}

/********************************************************************************************************
 Process
 Your program should first check if there exists a jumping move starting from the current configuration 
 since jumping is mandatory. If there is none, then expand the simple moves. Your program should also do 
 alpha-beta pruning while expanding the search to reduce the complexity.
********************************************************************************************************/
void process() {
	// print_game_config();
	remove(output_file.c_str());
	// for each configuration
	Game *game = First_Game;
	int case_no = 0;
	while(game != 0) {
		++case_no;
		print_game_config(case_no);
		cout << "case " << case_no << endl;
		stringstream ss;
		ss << case_no;
		output_result = "case " + ss.str() + "\n";
		alpha_beta(game);
		cout << endl;
		game = game->next;
		output(output_result);
	}
}

/********************************************************************************************************
 Main
********************************************************************************************************/
int main(int argc, char *argv[]) {
	if(argc == 3)
		command(argv);
	else
		error("command number error");
	process();
	return 0;
}



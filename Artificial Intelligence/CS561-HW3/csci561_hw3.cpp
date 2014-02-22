/*
 Author:			Yu Sun

 Date:				04/6~8/2013

 Description:		CSCI561 HW3
 					CPP File
*/

#include <iostream>
#include <vector>
#include <string>
#include <new>

#include "csci561_hw3.h"

using namespace std;
/********************************************************************************************************
 Debug & Utility
********************************************************************************************************/
// Print Title
void printTitle(string str) {
	cout << str << ": " << endl;
	cout << "-----------------------------------------------" << endl;
}

// Print R[M][M]
void printMatrix() {
	for(int i = 0; i < M; i++) {
		for(int j = 0; j < M; j++) {
			int index = i * M + j;
			cout << R[index] << "\t";
		}
		cout << "\n\n";
	}
}

// Print Model[M][N]
void printModel() {
	for(int i = 0; i < M; i++) {
		for(int n = 0; n < N; n++) {
			int index = i * N + n;
			cout << (Model[index] == true ? "true" : "false") << "\t";
		}
		cout << "\n\n";
	}
}

// Print CNF
void printCNF() {
	for (vector <vector <Literal> >::iterator itClause = CNF.begin(); itClause != CNF.end(); ++itClause) {
		vector <Literal>::iterator itLiteral = (*itClause).begin();
		Literal literal = *itLiteral;
		cout << (literal.Negative() ? "¬ " : "") << "X" << "(" << literal.Guest() + 1 << "," << literal.Seat() + 1 << ")";
		++itLiteral;
		while(itLiteral != (*itClause).end()) {
			literal = *itLiteral;
			cout << " V " << (literal.Negative() ? "¬ " : "") << "X" << "(" << literal.Guest() + 1 << "," << literal.Seat() + 1 << ")";
			++itLiteral;
		}
		cout << endl;
	}
	cout << endl;
}

// Print Sentence
void printSentence(vector <vector <Literal> > & sentence) {
	for (vector <vector <Literal> >::iterator itClause = sentence.begin(); itClause != sentence.end(); ++itClause) {
		vector <Literal>::iterator itLiteral = (*itClause).begin();
		Literal literal = *itLiteral;
		cout << (literal.Negative() ? "¬ " : "") << "X" << literal.Guest() + 1 << literal.Seat() + 1;
		++itLiteral;
		while(itLiteral != (*itClause).end()) {
			literal = *itLiteral;
			cout << " V " << (literal.Negative() ? "¬ " : "") << "X" << literal.Guest() + 1 << literal.Seat() + 1;
			++itLiteral;
		}
		cout << endl;
	}
	cout << endl;
}

// Print Clause
void printClause(vector <Literal> & clause) {
	vector <Literal>::iterator it = clause.begin();
	Literal literal = *it;
	cout << (literal.Negative() ? "¬ " : "") << "X" << literal.Guest() + 1 << literal.Seat() + 1;
	++it;
	while(it != clause.end()) {
		literal = *it;
		cout << " V " << (literal.Negative() ? "¬ " : "") << "X" << literal.Guest() + 1 << literal.Seat() + 1;
		++it;
	}
	cout << endl << endl;
}

// Show error message
void error(string message) {
	cerr << "Error: " << message << ", please input -exp1 or -exp2 or -exp3 as the only parameter, program abort!" << endl;
	exit(1);
}

// Print Argument
void printArg(int M, int N, double f, double e, double p, int max_flips) {
	cout << "<M=" << M
	<< ", N=" << N 
	<< ", f=" << f * 100
	<< "%, e=" << e * 100 
	<< "%, p=" << p * 100 
	<< "%, max_flips=" << max_flips 
	<< ">" << endl;
}

/********************************************************************************************************
 Task 2: Instance Generator
 As the first part of this programming assignment, you will need to write a program to generate CNF sentences
 for random instances of wedding seating arrangements. Key parameters of the program should include the number
 of guests <M>, and the number of tables <N>. Moreover, you need to randomly assign each pair of guests as
 Friends (F), Enemies (E) or Indifferent (I). Suppose any two guests are Friends with probability <f>, or are
 Enemies with probability <e>. And any two guests are Indifferent to each other with probability <1-f-e>.
 Given these parameters <M, N, f, e>, the generator should firstly produce a relationship for each pair of guests.
 The internal output of the program should be a M by M matrix R with elements Rij = 1, -1 or 0 to represent
 whether guest i and j are Friends (F), Enemies (E) or Indifferent (I).

 Parameters:
 M:		Number of guests
 N:		Number of tables
 f:		Friends probability
 e:		Enemies probability
 1-f-e:	Indifferent probability
 Return:
 R:		M by M matrix R with elements
 		Rij = 1, guest i and j are Friends (F),
 			 -1, guest i and j are Enemies (E),
 			  0, guest i and j are Indifferent (I).
********************************************************************************************************/
void instanceGenerator(int M, int N, double f, double e) {
	int rangeF = f * 100;
	int rangeE = e * 100 + rangeF;
	for(int i = 0; i < M; i++) {
		for(int j = i; j < M; j++) {
			if(i == j)
				continue;
			int randN = rand() % 100 + 1;
			int index = i * M + j;
			int _index = j * M + i;
			if(randN > rangeE) { // Indifferent
				R[index] = 0;
				R[_index] = 0;
			}
			else if(randN > rangeF) { // Enemies
				R[index] = -1;
				R[_index] = -1;
			}
			else { // Friends
				R[index] = 1;
				R[_index] = 1;
			}
		}
	}
}

/********************************************************************************************************
 Task 2: cont'
 Then your program should convert any generated instance of wedding seating arrangement into a CNF sentence.
 You are free to use whatever internal representation of CNF sentences. You will NOT be asked to input or
 output sentences for the user for this assignment. In general, it is a good idea to use the most efficient
 representation possible, given the NP-complete nature of SAT. For instance, in C++, you can represent a CNF
 sentence as a vector of clauses, and represent each clause as a vector of literals.

 Xij means guest i seat at table j.
	 (a)Each guest i should be seated at one and only one table.
 	 CNF: Xi1 ∨ Xi2 … ∨ Xin … ∨ XiN 						(1 ≤ i ≤ M, 1 ≤ n ≤ N)
 	 FOL: Xik ⟹ ¬ Xin
 	 CNF: ¬ Xik ∨ ¬ Xin										(1 ≤ i ≤ M, 1 ≤ k ≠ n ≤ N)

	 (b) Any two guests i and j who are Friends (F) should be seated at the same table.
 	 FOL: Xin ⟹ Xjn
 	 CNF: ¬ Xin ∨ Xjn									(1 ≤ i, j ≤ M, 1 ≤ n ≤ N)
 	 FOL: Xjn ⟹ Xin
 	 CNF: ¬ Xjn ∨ Xin									(1 ≤ i, j ≤ M, 1 ≤ n ≤ N)

	 (c) Any two guests i and j who are Enemies (E) should be seated at different tables.
 	 FOL: Xin ⟹ ¬ Xjn ^ Xjn ⟹ ¬ Xin
 	 CNF: ¬ Xin ∨ ¬ Xjn				 					(1 ≤ i, j ≤ M, 1 ≤ n ≤ N)

********************************************************************************************************/
void CNF_CONVERSION() {
	/*
	 Create clause (a)
	 (a)Each guest i should be seated at one and only one table.
 	 CNF: Xi1 ∨ Xi2 … ∨ Xin … ∨ XiN 						(1 ≤ i ≤ M, 1 ≤ n ≤ N)
 	 FOL: Xik ⟹ ¬ Xin
 	 CNF: ¬ Xik ∨ ¬ Xin										(1 ≤ i ≤ M, 1 ≤ k ≠ n ≤ N)
	 */
	for(int i = 0; i < M; i++) {
		vector <Literal> clause;
		for(int n = 0; n < N; n++) {
			Literal literal = Literal(false, i, n);

			// Add literals to clause
			clause.push_back(literal);
		}
		// Add clause to CNF
		CNF.push_back(clause);
	}
	for(int i = 0; i < M; i++) {
		for(int n = 0; n < N; n++) {
			for(int k = n + 1; k < N; k++) {
				vector <Literal> clause;

				Literal literal1 = Literal(true, i, n);
				Literal literal2 = Literal(true, i, k);

				// Add literals to clause
				clause.push_back(literal1);
				clause.push_back(literal2);

				// Add clause to CNF
				CNF.push_back(clause);
			}
		}
	}
	/*
	 Create clause (b)
	 (b) Any two guests i and j who are Friends (F) should be seated at the same table.
 	 FOL: Xin ⟹ Xjn
 	 CNF: ¬ Xin ∨ Xjn									(1 ≤ i, j ≤ M, 1 ≤ n ≤ N)
 	 FOL: Xjn ⟹ Xin
 	 CNF: ¬ Xjn ∨ Xin									(1 ≤ i, j ≤ M, 1 ≤ n ≤ N)
	*/
	for(int i = 0; i < M; i++) {
		for(int j = i; j < M; j++) {
			// if i and j are friends
			int index = i * M + j;
			if(R[index] == 1) {
				for(int n = 0; n < N; n++) {
					vector <Literal> clause1;
					Literal literal11 = Literal(true, i, n);
					Literal literal12 = Literal(false, j, n);
					// Add literals to clause
					clause1.push_back(literal11);
					clause1.push_back(literal12);

					vector <Literal> clause2;
					Literal literal21 = Literal(true, j, n);
					Literal literal22 = Literal(false, i, n);
					// Add literals to clause
					clause2.push_back(literal21);
					clause2.push_back(literal22);

					// Add clause to CNF
					CNF.push_back(clause1);
					CNF.push_back(clause2);
				}
			}
			else
				continue;
		}
	}
	/*
	 Create clause (c)
	 (c) Any two guests i and j who are Enemies (E) should be seated at different tables.
 	 FOL: Xin ⟹ ¬ Xjn ^ Xjn ⟹ ¬ Xin
 	 CNF: ¬ Xin ∨ ¬ Xjn				 					(1 ≤ i, j ≤ M, 1 ≤ n ≤ N)
 	*/
	for(int i = 0; i < M; i++) {
		for(int j = i; j < M; j++) {
			// if i and j are enemies
			int index = i * M + j;
			if(R[index] == -1) {
				for(int n = 0; n < N; n++) {
					vector <Literal> clause;
					Literal literal1 = Literal(true, i, n);
					Literal literal2 = Literal(true, j, n);
					// Add literals to clause
					clause.push_back(literal1);
					clause.push_back(literal2);

					// Add clause to CNF
					CNF.push_back(clause);
				}
			}
			else
				continue;
		}
	}
}

/********************************************************************************************************
 Task 3: SAT Solvers
 You need to implement SAT solvers to find a satisfying assignment for any given CNF sentences.
 Firstly, you need to implement a modified version of the PL-Resolution algorithm (AIMA Figure 7.12).
 Modifications are necessary because we are using the algorithm for a slightly different purpose than
 is explained in AIMA. Here, we are not looking to prove entailment of a particular query.
 Rather, we hope to prove satisfiability. Thus, there is no need to add negated query clauses to the
 input clauses. In other words, the only input to the algorithm is the set of clauses that comprise a
 randomly generated sentence. As an additional consequence of our purpose, the outputs will be reversed
 compared to the outputs listed in AIMA’s pseudo code. That is to say, if the empty clause is derived at
 any point from the clauses of the input sentence, then the sentence is unsatisfiable. In this case,
 the function should return false and not true as the book specifies for this situation. In the opposite
 situation where the empty clause is never derived, the algorithm should return true, indicating that
 the sentence is satisfiable.
********************************************************************************************************/
/*
 Check whether literal1 and literal2 are resolvable, e.g. A and ¬ A are resolvable.
 return:
 	true, literal1 and literal2 are resolvable.
 	false, literal1 and literal2 are not resolvable.
*/
bool resolvable(Literal & literal1, Literal & literal2) {
	if(literal1.Guest() == literal2.Guest() && // Same guest
	literal1.Seat() == literal2.Seat() && // Same seat
	literal1.Negative() != literal2.Negative()) { // Different negative
		return true;
	}
	else
		return false;
}

/*
 Compare literal1 and literal2.
 return:
 	true, literal1 and literal2 are same.
 	false, literal1 and literal2 are different.
*/
bool sameLiteral(Literal & literal1, Literal & literal2) {
	if(literal1.Guest() == literal2.Guest() &&
	literal1.Seat() == literal2.Seat() &&
	literal1.Negative() == literal2.Negative()) {
		return true;
	}
	else
		return false;
}

/*
 Compare clause1 and clause2.
 return:
 	true, clause1 and clause2 are same.
 	false, clause1 and clause2 are different.
*/
bool sameClause(vector <Literal> & clause1, vector <Literal> & clause2) {
	bool isSame = false;
	// the number of same literals
	unsigned int num = 0;
	if(clause1.size() == clause2.size()) {
		for(vector <Literal>::iterator it1 = clause1.begin(); it1 != clause1.end(); ++it1) {
			for(vector <Literal>::iterator it2 = clause2.begin(); it2 != clause2.end(); ++it2) {
				Literal literal1 = *it1;
				Literal literal2 = *it2;
				if(sameLiteral(literal1, literal2)) {
					num++;
					break;
				}
			}
		}
		if(num == clause1.size())
			isSame = true;
	}
	return isSame;
}

/*
 Check whether clause contain literal.
 return:
 	true, clause contain literal.
 	false, clause do not contain literal.
*/
bool contain(vector <Literal> & clause, Literal & literal) {
	bool isContain = false;
	for(vector <Literal>::iterator it = clause.begin(); it != clause.end(); ++it) {
		if(sameLiteral(*it, literal)) {
			isContain = true;
			break;
		}
	}
	return isContain;
}

bool PL_RESOLVE(vector <Literal> & C1, vector <Literal> & C2, vector <Literal> & resolvents) {
	bool isResolvable = false;
	vector <Literal>::iterator re1;
	vector <Literal>::iterator re2;
	for(re1 = C1.begin(); re1 != C1.end(); ++re1) {
		for(re2 = C2.begin(); re2 != C2.end(); ++re2) {
			Literal literal1 = *re1;
			Literal literal2 = *re2;
			if(resolvable(literal1, literal2)) {
				// Find the resolvable literal
				isResolvable = true;
				break;
			}
		}
		if(isResolvable)
			break;
	}
	if(isResolvable) {
		// Add the rest literals of C1 to resolvents
		for(vector <Literal>::iterator it1 = C1.begin(); it1 != C1.end(); ++it1) {
			if(it1 != re1)
				resolvents.push_back(*it1);
		}
		// Add the rest literals of C2 to resolvents, and also need to check whether have repeat clause
		for(vector <Literal>::iterator it2 = C2.begin(); it2 != C2.end(); ++it2) {
			if(it2 != re2) {
				if(!contain(resolvents, *it2))
					resolvents.push_back(*it2);
			}
		}
	}
	return isResolvable;
}

/*
 If resolvents contain the literals like A V ¬ A, it should be discard because it always true.
 In addition, if resolvent is already included in the _new, it also should be discard.
 return:
 	true, if resolvents should discard.
 	false, if resolvents should not discard.
*/
bool discard(vector <Literal> & resolvents, vector <vector <Literal> > & _new) {
	bool needDiscard = false;
	// check whether resolvents contains A V ¬ A
	for(vector <Literal>::iterator it1 = resolvents.begin(); it1 != resolvents.end(); ++it1) {
		for(vector <Literal>::iterator it2 = it1 + 1; it2 != resolvents.end(); ++it2) {
			Literal literal1 = *it1;
			Literal literal2 = *it2;
			if(resolvable(literal1, literal2)) {
				needDiscard = true;
				break;
			}
		}
		if(needDiscard)
			break;
	}
	// check whether resolvents is already included in _new.
	if(!needDiscard) {
		for(vector <vector <Literal> >::iterator it = _new.begin(); it != _new.end(); ++it) {
			if(sameClause(*it, resolvents)) {
				needDiscard = true;
				break;
			}
		}
	}
	return needDiscard;
}

/*
 Add _new into clauses, expect the clause already in clauses.
 return:
 	true, not every clauses in _new is alreay in clauses.
 	false, every clauses in _new is alreay in clauses.
*/
bool unions(vector <vector <Literal> > & clauses, vector <vector <Literal> > & _new) {
	int addNum = 0;
	for(vector <vector <Literal> >::iterator clause1 = _new.begin(); clause1 != _new.end(); ++clause1) {
		bool isContained = false;
		for(vector <vector <Literal> >::iterator clause2 = clauses.begin(); clause2 != clauses.end(); ++clause2) {
			if(sameClause(*clause1, *clause2)) {
				isContained = true;
				break;
			}
		}
		if(!isContained) {
			clauses.push_back(*clause1);
			addNum++;
		}
	}
	return (addNum == 0);
}

/*
 function MODIFIED PL-RESOLUTION(KB) returns true or false
     inputs: KB, the knowledge base, a sentence in propositional logic

     clauses <– the set of clauses in the CNF representation of KB
     new <– {}
     loop do
          for each pair of clauses Ci, Cj in clauses do
               resolvents <– PL-RESOLVE(Ci, Cj)
               if resolvents contains the empty clause then return false
               new <– new U resolvents
          if new ∈ clauses then return true
          clauses <– clauses U new
*/
bool PL_RESOLUTION(vector <vector <Literal> > KB) {
	// vector <vector <Literal> > KB = CNF;
	int offset = 0;
	while(true) {
		vector <vector <Literal> > _new;
		for(vector <vector <Literal> >::iterator C1 = KB.begin(); C1 != KB.end(); ++C1) {
			// avoid repeat calculate
			for(vector <vector <Literal> >::iterator C2 = (C1 + 1 > KB.begin() + offset ? C1 + 1 : KB.begin() + offset);
			 C2 != KB.end(); ++C2) {
				vector <Literal> resolvents;
				// if C1 and C2 is resolvable, resolve
				if(Debug) {
					printTitle("Resolving");
					cout << "C1\t";
					printClause(*C1);
					cout << "C2:\t";
					printClause(*C2);
				}
				if(PL_RESOLVE(*C1, *C2, resolvents)) {
					// resolvents contains the empty clause then return false
					if(resolvents.size() == 0)
						return false;
					if(Debug) {
						cout << "R:\t";
						printClause(resolvents);
						cout << endl;
					}
					// check whether the resulted resolvents need discard
					if(!discard(resolvents, _new)) {
						// new <– new U resolvents
						_new.push_back(resolvents);
						if(Debug) {
							printTitle("Add to _new, _new contain");
							printSentence(_new);
							cout << endl;
						}
					}
					else {
						if(Debug) {
							printTitle("Discard");
							cout << endl;
						}
					}
				}
				else {
					if(Debug) {
						cout << "R:\t" << "Can not resolve." << endl << endl;
					}
				}
			}
		}
		offset = KB.size();
		// if new ∈ clauses then return true, clauses <– clauses U new
		if(unions(KB, _new))
			return true;
		if(Debug) {
			printTitle("One Loop End, updated KB");
			printSentence(KB);
			cout << endl;
		}
	}
}

/********************************************************************************************************
 Task 3: cont'
 You are also asked to implement the WalkSAT algorithm (AIMA Figure 7.18). There are many variants of this
 algorithm exist, but yours should be identical to the algorithm described in AIMA. There are two open 
 parameters associated with WalkSAT: <p> and <max_flips>.
 PL-Resolution is a sound and complete algorithm that can be used to determine satisfiability and 
 unsatisfiability with certainty. On the other hand, WalkSAT can determine satisfiability (if it finds a model),
 but it cannot asbolutely determine unsatisfiability.
********************************************************************************************************/
/*
 Generate a random true/false symbol and store in Model
 If the random number rands is less than 50, true.
 Otherwise, false.
*/
void modelGenerator() {
	for(int i = 0; i < M; i++) {
		for(int n = 0; n < N; n++) {
			int rands = rand() % 100 + 1;
			int index = i * N + n;
			if(rands <= 50)
				Model[index] = true;
			else
				Model[index] = false;
		}
	}
}

/*
 Check the given Model whether it is satisfy the currently clauses
 clauseNotSatisify: the random selected unsatisify clause
 return: true, satisfy.
 		 false, not satisfy.
*/
bool satisfy(vector <vector <Literal> > & clauses, vector <Literal> & clauseNotSatisify) {
	bool isSatisfy = true;
	vector <int> clauseNotSatisifyIndex;
	int clauseIndex = 0;
	// in every clause
	for(vector <vector <Literal> >::iterator clause = clauses.begin(); clause != clauses.end(); ++clause, ++clauseIndex) {
		int numNotSatisfy = 0;
		int numLiteral = (*clause).size();
		// in every literal
		for(vector <Literal>::iterator literal = (*clause).begin(); literal != (*clause).end(); ++literal) {
			int i = (*literal).Guest();
			int n = (*literal).Seat();
			bool neg = (*literal).Negative();
			// no ¬ symbol, literal should be true.
			int index = i * N + n;
			if(!neg) {
				// one literal of the clause is true, this clause satisfied
				if(Model[index] == true)
					break;
				else
					numNotSatisfy++;
			}
			else { // has ¬ symbol, literal should be false
				// one literal of the clause is true, this clause satisfied
				if(Model[index] == false)
					break;
				else
					numNotSatisfy++;
			}
		}
		// every literal in the clause is not satisfy, return failure
		if(numNotSatisfy == numLiteral) {
			// clauseNotSatisify = *clause;
			isSatisfy = false;
			// break;
			clauseNotSatisifyIndex.push_back(clauseIndex);
		}
	}
	// select clauseNotSatisify randomly
	if(clauseNotSatisifyIndex.size() != 0) {
		int rands = rand() % clauseNotSatisifyIndex.size();
		int offset = clauseNotSatisifyIndex[rands];
		clauseNotSatisify = *(clauses.begin() + offset);
	}
	return isSatisfy;
}

/*
 Flip the model at (i, n).
*/
void flip(int i, int n) {
	int index = i * N + n;
	if(Model[index] == true)
		Model[index] = false;
	else
		Model[index] = true;
}

/*
 Count the number of satisify clauses with current Model
 return: the number of satisify clauses.
*/
int countSatisify(vector <vector <Literal> > & clauses) {
	int numSatisify = 0;
	// in every clause
	for(vector <vector <Literal> >::iterator clause = clauses.begin(); clause != clauses.end(); ++clause) {
		// in every literal
		for(vector <Literal>::iterator literal = (*clause).begin(); literal != (*clause).end(); ++literal) {
			int i = (*literal).Guest();
			int n = (*literal).Seat();
			bool neg = (*literal).Negative();
			// no ¬ symbol, literal should be true.
			int index = i * N + n;
			if(!neg) {
				// one literal of the clause is true, this clause satisfied
				if(Model[index] == true) {
					numSatisify++;
					break;
				}
			}
			else { // has ¬ symbol, literal should be false
				// one literal of the clause is true, this clause satisfied
				if(Model[index] == false) {
					numSatisify++;
					break;
				}
			}
		}
	}
	return numSatisify;
}

/*
 function WALKSAT(clauses, p, max_flips) return a satisfying model or failure
	inputs:	clauses, a set of clauses in propositional logic
			p, the probability of choosing to do a "random walk" move, typically around 0.5
			max_flips, number of flips allowed before giving up
	
	model <– a random assignment of true/false to the symbols in clauses
	for i = 1 to max_flips do
		if model satisfies clauses then return  model
		clause <– a randomly selected clause from clauses that is false in model
		with probability p flip the value in model of a randomly selected symbol from clause
		else flip whichever symbol in clause maximizes the number of satisfied clauses
	return failure

 One of the simplest and most effective algorithms to emerge from all this work is called WALKSAT (Figure 7.18). 
 On every iteration, the algorithm picks an unsatisfied clause and picks a symbol in the clause to flip. 
 It chooses randomly between two ways to pick which symbol to flip: (1) a “min-conflicts” step that minimizes the 
 number of unsatisfied clauses in the new state and (2) a “random walk” step that picks the symbol randomly.

  return:
  		0 < ret < max_flips, indicate satisify under max_flips
  		ret = -1, indicate not satisify under max_flips
*/
int WALK_SAT(vector <vector <Literal> > & clauses, double p, int max_flips) {
	// bool findModel = false;
	// model <– a random assignment of true/false to the symbols in clauses
	modelGenerator();
	if(Debug) {
		printTitle("Model matrix");
		printModel();
	}

	for(int i = 0; i <= max_flips; i++) {
		// check if the clauses is satisify, if not, find the clause which not satisify
		vector <Literal> clauseNotSatisify;
		if(satisfy(clauses, clauseNotSatisify))
			return i;
		if(Debug) {
			cout << "The clause that not satisify: ";
			printClause(clauseNotSatisify);
		}
		int rands = rand() % 100 + 1;
		if(rands > p * 100) { // use "min-conflicts" step
			if(Debug)
				printTitle("Min-conflicts step");
			vector <Literal>::iterator itMax;
			int currentMaxSatisify = 0, i, n;
			// for every litural in clauseNotSatisify
			for(vector <Literal>::iterator it = clauseNotSatisify.begin(); it != clauseNotSatisify.end(); ++it) {
				i = (*it).Guest();
				n = (*it).Seat();
				// try to flip (i, n)
				flip(i, n);
				int count = countSatisify(clauses);
				if(count > currentMaxSatisify) {
					currentMaxSatisify = count;
					itMax = it;
				}
				// revert to original Model
				flip(i, n);
			}
			// flip the (i, n) which will satisify most clauses
			i = (*itMax).Guest();
			n = (*itMax).Seat();
			flip(i, n);
			if(Debug) {
				printTitle("Updated model");
				printModel();
			}
		}
		else { // use “random walk” step
			if(Debug)
				printTitle("Random walk step");
			int randomSelect = rand() % clauseNotSatisify.size();
			// slect the literal randomly
			vector <Literal>::iterator it = clauseNotSatisify.begin() + randomSelect;
			int i = (*it).Guest();
			int n = (*it).Seat();
			flip(i, n);
			if(Debug) {
				printTitle("Updated model");
				printModel();
			}
		}
	}
	return -1;
}

/********************************************************************************************************
 Task 4: Experiment 1
 The difficulty of wedding seat arrangement problem mostly results from dealing with the pairs of Enemies 
 (E) among guests. Use both algorithms to produce a plot of P(satisfiability) (See Figure 7.19(a), AIMA) 
 as a function of the probability <e> with which any two guests are Enemies (E) .

 Suppose we have a small wedding to plan, and set <M=16, N=2>. In order to eliminate the influence of 
 Friends (F) relationship, we set <f=0>. Generate a set of 100 random sentences for each setting of <e> 
 which increases from 2% to 20% at an interval of 2%, and use both of your algorithms to determine whether 
 they are satisfiable. For WalkSAT, we set <p=50%> and <max_flips=100>. Plot the results of P(satisfiability) 
 versus <e> for both algorithms on the same graph, so that you can compare.
********************************************************************************************************/
void experiment1() {
	double f = 0;
	double e = 0.02;
	int max_flips = 100;
 	double p = 0.5;
	printTitle("Experiment 1");
	// <e> increases from 2% to 20% at an interval of 2%
	for(; e <= 0.2; e += 0.02) {
		cout << "<e=" << e * 100 << "%>" << endl;
		int count_PL_RESOLUTION = 0;
		int count_WALK_SAT = 0;
		// set of 100 random sentences for each setting
		for(int i = 0; i < 100; i++) {
			instanceGenerator(M, N, f, e);

			CNF_CONVERSION();

			if(PL_RESOLUTION(CNF))
				count_PL_RESOLUTION++;
			if(WALK_SAT(CNF, p, max_flips) != -1)
				count_WALK_SAT++;
			// clean the CNF
			CNF.clear();
			cout << "\r" << i + 1 << "%" << flush;
		}
		cout << endl;
		cout << "PL_RESOLUTION:\tP=" << (double)count_PL_RESOLUTION << "%" << endl;
		cout << "WALK_SAT:\tP=" << (double)count_WALK_SAT << "%"  << endl;
		cout << endl;
	}
}


/********************************************************************************************************
 Task 5: Experiment 2
 You may have discovered that determining satisfiability with PL-Resolution can be frustratingly slow! 
 This was part of the motivation behind developing local search algorithms like WalkSAT. For easy SAT problems, 
 it can be very easy to find a model using a random walk through the possibilities. Using the relative 
 speed afforded by WalkSAT, we can run experiments that might otherwise be impractical.

 Suppose you want to know the effect of the probability <f> with which any two guests are Friends (F) on 
 P(satisfiability). We set <M=16, N=2> as in Experiment 1 and fix <e=5%>. Using only the WalkSAT algorithm 
 with parameter setting <p = 50%, max_flips=100>, run 100 random instances using different settings for <f>: 
 2% through 20% at an interval of 2%. You also need to vary <max_flips> and compare the results. 
 Increasing <max_flips> to 500 and 1000, run WalkSAT again respectively using the same random instances 
 as we set <max_flips=100>. Produce a plot of P(satisfiability) versus <f> for each setting of <max_flips> 
 on the same graph.
********************************************************************************************************/
void experiment2() {
	double f = 0.02;
	double e = 0.05;
	int max_flips = 100;
 	double p = 0.5;
	printTitle("Experiment 2");
	// <f> increases from 2% to 20% at an interval of 2%
	for(int cases = 1; f <= 0.2; f += 0.02, cases++) {
		cout << "<f=" << f * 100 << "%>" << endl;
		int count_WALK_SAT_100 = 0;
		int count_WALK_SAT_500 = 0;
		int count_WALK_SAT_1000 = 0;
		// set of 100 random sentences for each setting
		for(int i = 0; i < 100; i++) {
			instanceGenerator(M, N, f, e);

			CNF_CONVERSION();

			max_flips = 100;
			if(WALK_SAT(CNF, p, max_flips) != -1)
				count_WALK_SAT_100++;
			max_flips = 500;
			if(WALK_SAT(CNF, p, max_flips) != -1)
				count_WALK_SAT_500++;
			max_flips = 1000;
			if(WALK_SAT(CNF, p, max_flips) != -1)
				count_WALK_SAT_1000++;
			// clean the CNF
			CNF.clear();
			cout << "\r" << i + 1 << "%" << flush;
		}
		cout << endl;
		cout << "WALK_SAT:\tP=" << (double)count_WALK_SAT_100 << "%, max_flips=100" << endl;
		cout << "\t\tP=" << (double)count_WALK_SAT_500 << "%, max_flips=500" << endl;
		cout << "\t\tP=" << (double)count_WALK_SAT_1000 << "%, max_flips=1000" << endl;
		cout << endl;
	}
}

/********************************************************************************************************
 Task 6: Experiment 3
 The difficulty of the wedding seat arrangement also depends on how large the wedding is (how many guests 
 and how many tables). We now restrict our analysis to only satisfiable sentences with different settings 
 of <M, N>. We only record runtimes for sentences for which a model is found before WalkSAT reaches max_flips 
 iterations. Any sentences for which no model is found in max_flips iterations should be deemed unsatisfiable 
 and eliminated from the results.

 We set <f=2%, e=2%> throughout this experiment, and increase the size of the wedding by sequentially setting 
 <M, N> as <16, 2>, <24, 3>, <32, 4>, <40, 5> and <48, 6>.
 Fixing <p=50%, max_flips=1000>, run only the WalkSAT algorithm for random instances until 100 satisfiable 
 CNF sentences are generated for each setting of <M, N>. You are asked to record the runtime it takes at 
 average to determine their satisfiability status. Runtime is measured by counting the number of iterations 
 through the WalkSAT algorithm (i.e., following in AIMA Figure 7.18, what is the iterator i when finished?). 
 Record the ratio of the number of clauses to the number of symbols for each satisfiable sentence as well.

 Produce a plot of the average runtime versus the average ratio of clause/symbol for each above-mentioned 
 setting of <M, N>, but only over the 100 satisfiable sentences.
********************************************************************************************************/
int countSymbols() {
	int num = 0;
	int *symbol = new int[M * N];
	for(vector <vector <Literal> >::iterator itClause = CNF.begin(); itClause != CNF.end(); ++itClause) {
		for(vector <Literal>::iterator itLiteral = (*itClause).begin(); itLiteral != (*itClause).end(); ++itLiteral) {
			int i = (*itLiteral).Guest();
			int n = (*itLiteral).Seat();
			int index = i * N + n;
			symbol[index] = 1;
		}
	}
	for(int i = 0; i < M; i++) {
		for(int n = 0; n < N; n++) {
			int index = i * N + n;
			num += symbol[index];
		}
	}
	delete[] symbol;
	return num;
}

int countClauses() {
	return CNF.size();
}

void doExperiment3() {
	double f = 0.02;
	double e = 0.05;
	int max_flips = 1000;
 	double p = 0.5;
	int numSatisify = 0;
	cout << "<M=" << M << ", N=" << N << ">" << endl;
	int runtime = 0;
	double totalRuntime = 0;
	double averageRuntime = 0;
	int numSymbols = 0;
	int numClauses = 0;
	double ratio = 0;
	double totalRatio = 0;
	double averageRatio = 0;

 	while(numSatisify < 100) {
 		instanceGenerator(M, N, f, e);
 		CNF_CONVERSION();
		numSymbols = countSymbols();
		numClauses = countClauses();
		ratio = (double)numClauses / numSymbols;
 		if((runtime = WALK_SAT(CNF, p, max_flips)) != -1) {
 			if(Experiment_Debug)
 				cout << runtime << endl;
 			numSatisify++;
 			totalRuntime += runtime;
 			totalRatio += ratio;
 			if(Experiment_Debug)
 				cout << ratio << endl;
 		}
 		else {
 			if(Experiment_Debug)
 				cout << "Failed" << endl;
 		}
 		CNF.clear();
 		if(!Experiment_Debug)
 			cout << "\r" << numSatisify << "%" << flush;
 	}
 	cout << endl;
 	averageRuntime = totalRuntime / 100;
 	averageRatio = totalRatio / 100;
 	cout << "WALK_SAT:\taverage runtime=" << averageRuntime << endl;
 	cout << "\t\taverage ratio(clause/symbol)=" << averageRatio << endl << endl;
}

void experiment3() {
 	printTitle("Experiment 3");
 	// <M, N> as <16, 2>
 	M = M_EXP3_1;
 	N = N_EXP3_1;
 	R = R_EXP3_1;
 	Model = Model_EXP3_1;
 	doExperiment3();
 	// <M, N> as <24, 3>
 	M = M_EXP3_2;
 	N = N_EXP3_2;
 	R = R_EXP3_2;
 	Model = Model_EXP3_2;
 	doExperiment3();
 	// <M, N> as <32, 4>
 	M = M_EXP3_3;
 	N = N_EXP3_3;
 	R = R_EXP3_3;
 	Model = Model_EXP3_3;
 	doExperiment3();
 	// <M, N> as <40, 5>
 	M = M_EXP3_4;
 	N = N_EXP3_4;
 	R = R_EXP3_4;
 	Model = Model_EXP3_4;
 	doExperiment3();
 	// <M, N> as <48, 6>
 	M = M_EXP3_5;
 	N = N_EXP3_5;
 	R = R_EXP3_5;
 	Model = Model_EXP3_5;
 	doExperiment3();
 	// <M, N> as <56, 7>
 	M = M_EXP3_6;
 	N = N_EXP3_6;
 	R = R_EXP3_6;
 	Model = Model_EXP3_6;
 	// doExperiment3();
}

/********************************************************************************************************
 Test & Debug
********************************************************************************************************/
 void test() {
 	double f = 0.02;				// Friends probability
 	double e = 0.02;				// Enemies probability
 	int max_flips = 1000;			// Max number of trying to flip
 	double p = 0.5;					// Random probability
	instanceGenerator(M, N, f, e);
	printTitle("Relationship matrix");
	printMatrix();

	CNF_CONVERSION();
	printTitle("CNF of instance");
	printCNF();
	printTitle("Num of Clause");
	cout << CNF.size() << endl << endl;
	printTitle("Num of Symbol");
	cout << M * N << endl << endl;

	if(Test_PL_RESOLUTION) {
		if(PL_RESOLUTION(CNF))
			cout << "Sentences is satisfiability." << endl;
		else
			cout << "Sentences is not satisfiability." << endl;
	}

	if(Test_PL_WALK_SAT) {
		if(WALK_SAT(CNF, p, max_flips) != -1)
			cout << "Sentences is satisfiability." << endl;
		else
			cout << "Sentences is not satisfiability." << endl;
	}
 }

/*
 Test for AIAM Figure 7.19(b).
*/
 void test2() {
 	double f = 0.02;
	double e = 0.1;
	int max_flips = 1000;
 	double p = 0.5;
	int numSatisify = 0;
	int runtime = 0;
	double totalRuntime = 0;
	double averageRuntime = 0;
	int numSymbols = 0;
	int numClauses = 0;
	double ratio = 0;
	double totalRatio = 0;
	double averageRatio = 0;
	Debug = 0;
 	for(f = 0.02; f <= 0.2; f += 0.02) {
 		cout << "<f=" << f << ">" << endl;
 		 while(numSatisify < 100) {
	 		instanceGenerator(M, N, f, e);
	 		CNF_CONVERSION();
			numSymbols = countSymbols();
			numClauses = countClauses();
			ratio = (double)numClauses / numSymbols;
	 		if((runtime = WALK_SAT(CNF, p, max_flips)) != -1) {
	 			if(Experiment_Debug)
	 				cout << runtime << endl;
	 			numSatisify++;
	 			totalRuntime += runtime;
	 			totalRatio += ratio;
	 			if(Experiment_Debug)
	 				cout << ratio << endl;
	 		}
	 		else {
	 			if(Experiment_Debug)
	 				cout << "Failed" << endl;
	 		}
	 		CNF.clear();
	 		if(!Experiment_Debug)
	 			cout << "\r" << numSatisify << "%" << flush;
 		}
 		cout << endl;
	 	averageRuntime = totalRuntime / 100;
	 	averageRatio = totalRatio / 100;
	 	cout << "WALK_SAT:\taverage runtime=" << averageRuntime << endl;
	 	cout << "\t\taverage ratio(clause/symbol)=" << averageRatio << endl << endl;
		numSatisify = 0;
		runtime = 0;
		totalRuntime = 0;
		averageRuntime = 0;
		numSymbols = 0;
		numClauses = 0;
		ratio = 0;
		totalRatio = 0;
		averageRatio = 0;
 	}
 }

/********************************************************************************************************
 Process
********************************************************************************************************/
void process() {
	// do experiments
	switch(MODE) {
		case EXP1:
			experiment1();
			break;
		case EXP2:
			experiment2();
			break;
		case EXP3:
			experiment3();
			break;
		case TEST:
			test();
			// test2();
			break;
		default:
			break;
	}
}

/********************************************************************************************************
 Command
********************************************************************************************************/
void command(char * argv[]) {
	string command = string(argv[1]);
	if(command == "-exp1") {
		// set mode
		MODE = EXP1;
		// set arguments
		R = R_EXP1;
		Model = Model_EXP1;
		M = M_EXP1;
		N = N_EXP1;
	}
	else if(command == "-exp2") {
		// set mode
		MODE = EXP2;
		// set arguments
		R = R_EXP2;
		Model = Model_EXP2;
		M = M_EXP2;
		N = N_EXP2;
	}
	else if(command == "-exp3") {
		// set mode
		MODE = EXP3;
		// set arguments, different way
		// R = R_EXP3;
		// Model = Model_EXP3;
		// M = M_EXP3;
		// N = N_EXP3;
	}
	else if(command == "-debug") {
		MODE = TEST;
		Debug = 1;
		R = R_Debug;
		Model = Model_Debug;
		M = M_Debug;
		N = N_Debug;
	}
	else {
		error("unknown command");
	}
}

/********************************************************************************************************
 Main
********************************************************************************************************/
int main(int argc, char * argv[]) {
	if(argc == 2)
		command(argv);
	else
		error("command number error");
	process();
	return 0;
}







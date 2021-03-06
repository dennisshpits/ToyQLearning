#include<iostream>
#include<vector>
#include<string>
#include<map>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include"State.h"
#include"Action.h"
#define PAUSE()  do { printf("Press any key to continue . . ."); getchar(); } while (0)
/*
Improvments TBD:

- If we want to improve the q learning function we must add random exploration with probability epsilon
at each step. Currently the agent takes the best action based on what it knows so far.
- Provable to converge to real q* as long as policy allows for some random "exploration"
- Can turn off exploration once we believe the algorithim has converged and start "exploiting" the policy.
*/

using namespace std;

struct Node {
public:	
	int position;
	vector<Node*> children;
};

class MyTree {
public:
	Node *root;
	vector<vector<int>> PathsToPosition(int position) {
			vector<Node*> parentpositions;
			vector<vector<int>> allpaths;
			gotonode(root,position, parentpositions, &allpaths);
			return allpaths;
	}
private:
	void gotonode(Node * n,int p, vector<Node*> paths, vector<vector<int>> * ap) {
		paths.push_back(n);
		if (n->position == p) {
			vector<int> newvec;
			for (const auto child : paths) {
				newvec.push_back(child->position);
			}
			ap->push_back(newvec);
		}
		
		for (const auto child : n->children) {
			gotonode(child,p, paths,ap);
		}
	}
};



template <class T>
void printmatrix(T** m, int r, int c) {
	const char separator = ' ';
	const int width = 4;
	
	for (int j = 0; j < r;j++) {
		for (int i = 0;i < c;i++) {
			cout << left << setw(width) << setfill(separator) << m[j][i];
		}
		cout << endl;
	}
}

void printqmatrix(map<int, map<string, double>> myq, vector<Action*> a) {
	const char separator = ' ';
	const int width = 8;

	for (int i = 1;i <= myq.size() ;i++) {
		for (const auto anaction : a) {
			cout << left << setw(width) << setfill(separator) << myq[i][anaction->GetName()];
		}
		cout << endl;
	}

}

void addnode(Node * n, vector<Action*> myactions, State * s, map<int, int> p) {
	for (const auto anaction : myactions) {
		if (anaction->NextPositionValid(s)) {
			if (p.find(anaction->GetNextPosition(s)) == p.end()) {
				auto movenode = new Node;
				p[n->position] = n->position;
				movenode->position = anaction->GetNextPosition(s);
				n->children.push_back(movenode);
				auto newstate = new State(s->Rows(), s->Columns(), movenode->position);
				addnode(movenode, myactions, newstate, p);
			}
		}
	}
}

void BuildTree(MyTree * t, vector<Action*> a, State * s, int startposition) {
	t->root = new Node;
	t->root->position = startposition;
	map<int, int> parentpositions;
	addnode(t->root,a, s, parentpositions);
}

//Call q function
//An episode ends when an agent finds a success.
//This is called episodic learning
vector<int> TakePathToPosition(vector<Action*> a, State * s, int winningposition, map<int, map<string, double>> * qtable) {
	double alpha = 0.5; //learning rate (0 is q table never changes, 1 adopt new observation and throw away what you knew before)
	State mystate(s->Rows(), s->Columns(), s->CurrentPosition());
	//ways to update
	
	//choose an action to take based off of q table
	double qmax = -1000;
	Action * takeaction = nullptr;
	double currentq = 0;
	int reward = 0;
	map<int, int> past;
	srand(time(NULL));
	bool allactionsexplored = false;
	int randomAction = -1;
	bool stuck = false;
	int stuckcount = 0;
	double valuefunctionstateplusone = 0;
	vector<int> retvec;

	while (mystate.CurrentPosition() != winningposition) { //until we are not in the winning position keep exploring the world

		qmax = -1000;
		takeaction = nullptr;
		currentq = 0;
		reward = 0;
		allactionsexplored = false;
		randomAction = -1;
		map<string, string> pastActions;

		//for (const auto anaction : a) {
		while (allactionsexplored != true){

			randomAction = (rand() % a.size());
			auto anaction = a[randomAction];
			if (pastActions.find(anaction->GetName()) != pastActions.end()) { //dont move back to old action
				continue;
			} else {
				pastActions[anaction->GetName()] = anaction->GetName();
			}

			if (pastActions.size() == a.size()) {
				allactionsexplored = true;
			}

			currentq = qtable->find(mystate.CurrentPosition())->second.find(anaction->GetName())->second;

			if (anaction->NextPositionValid(&mystate)) {
				if (past.find(anaction->GetNextPosition(&mystate)) != past.end()) { //dont move back to old position
					//check if all possible actions lead to a past position
					
					stuckcount = 0;
					for (const auto theaction : a) { //loop through all actions
						if (theaction->NextPositionValid(&mystate)) {
							if (past.find(theaction->GetNextPosition(&mystate)) == past.end()) {
								break; // we are not stuck, we have more options
							} else {
								stuckcount++;
							}
						} else {
							stuckcount++;
						}


					}

					if (stuckcount == a.size()) {
						stuck = true;
						break;
					}
					
					continue;
				}
			}

			if (currentq > qmax) {
				qmax = currentq;
				takeaction = anaction;
			}
		}

		if (stuck == true) {
			break;
		}

		//Get the reward for that action
		reward = takeaction->RewardFunction(&mystate, winningposition);

		//Y maxa, q(s',a')
		valuefunctionstateplusone = 0;
		double vprime = 0;
		double maxaction = -1000;
		if (takeaction->NextPositionValid(&mystate) == true) {
			//find max action
			for (const auto actionprime : a) { //loop through all actions
				valuefunctionstateplusone = qtable->find(takeaction->GetNextPosition(&mystate))->second.find(actionprime->GetName())->second;
				if (valuefunctionstateplusone > maxaction) {
					maxaction = valuefunctionstateplusone;
					vprime = valuefunctionstateplusone;
				}
			}
		}

		qtable->find(mystate.CurrentPosition())->second.find(takeaction->GetName())->second = ( (1- alpha) * qtable->find(mystate.CurrentPosition())->second.find(takeaction->GetName())->second) + (alpha * (reward + vprime));

		if (takeaction->NextPositionValid(&mystate) == true) {
			past[mystate.CurrentPosition()] = mystate.CurrentPosition();
			retvec.push_back(mystate.CurrentPosition());
			mystate.UpdatePosition(takeaction->GetNextPosition(&mystate));
		}

	}

	retvec.push_back(mystate.CurrentPosition()); //add winning position
	return retvec;
}

int main() {

	int rows = 2;
	int columns = 3;
	int start = 1;
	int finish = 3;
	//ask user to input rows and columns and start

	State s(rows, columns, start);

	cout << "print enviornment grid:" << endl;
	s.printmatrix();

	///Create actions///
	MoveRight right;
	MoveDown down;
	MoveUp up;
	MoveLeft left;

	vector<Action*> av;
	av.push_back(&up);
	av.push_back(&down);
	av.push_back(&left);
	av.push_back(&right);
	///////actions/////////

	MyTree ntree;
	BuildTree(&ntree, av, &s, start);

	cout << "All possible paths to position " << finish << ":" << endl;
	vector<vector<int>> mypaths = ntree.PathsToPosition(finish);
	
	for (const auto childvec : mypaths) {
		string pathtaken = " ";
		for (const auto child : childvec) {
			pathtaken = pathtaken + " " + to_string(child);
		}
		cout << "Success:" << pathtaken << endl;
	}

	
	//Our goal here is to learn the q function q(s,a) where s is a state and a is an action
	//lets create our q table
	//in the beginning our q table is empty
	map<int,map<string, double>> q;

	for (int i = 1;i <= rows * columns;i++) {
		map<string, double> newmap;
		for (const auto anaction : av) {
			newmap[anaction->GetName()] = 0;
		}
		q[i] = newmap;
	}

	////////////////////QLearning//////////////////
	vector<int> cp;
	for (int i = 0; i < 15; i++) {
		cp=TakePathToPosition(av, &s, finish, &q);
	}

	cout << "The q after 10 iteration:" << endl;
	printqmatrix(q, av);

	cout << "The chosen path:" << endl;
	for (const auto val : cp) {
		cout << " " << val << " ";
	}
	cout << endl;
	///////////////////////////////////////////////

	PAUSE();
	return 0;
}

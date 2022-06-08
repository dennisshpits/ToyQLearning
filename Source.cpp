#include<iostream>
#include<vector>
#include<string>
#include<map>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include"State.h"
#include"Action.h"
#define PAUSE()  do { printf("Press any key to continue . . ."); getchar(); } while (0)

using namespace std;
omp_lock_t q_table_lock;

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
vector<int> TakePathToPosition(vector<Action*> a, State mystate, int winningposition, map<int, map<string, double>> * qtable) {
	double alpha = 0.5; //learning rate (0 is q table never changes, 1 adopt new observation and throw away what you knew before)
	
	//choose an action to take based off of q table
	double qmax = -1000;
	Action * takeaction = nullptr;
	double currentq = 0;
	int reward = 0;
	map<int, int> past;
	// srand needed for random_shuffle
	srand(time(NULL));
	bool stuck = false;
	int stuckcount = 0;
	double valuefunctionstateplusone = 0;
	vector<int> retvec;

	while (mystate.CurrentPosition() != winningposition) { //until we are not in the winning position keep exploring the world

		qmax = -1000;
		takeaction = nullptr;
		currentq = 0;
		reward = 0;

		// randomly shuffle the action vector so that the agent chooses a random action when exploring all possible actions
		random_shuffle(a.begin(),a.end());
		
		for (const auto anaction : a) {

			omp_set_lock(&q_table_lock);
			currentq = qtable->find(mystate.CurrentPosition())->second.find(anaction->GetName())->second;
			omp_unset_lock(&q_table_lock);

			if (anaction->NextPositionValid(&mystate)) {
				//dont move back to old position, but check if we are stuck
				if (past.find(anaction->GetNextPosition(&mystate)) != past.end()) {
					// TBD maybe move this logic into a class
					/*
						We need to check if the agent is stuck.
						This means that we need to check if all possible next actions the agent takes
						leads to a position that is not valid or already explored.

						For example if our grid is:
						1 2 3
						4 5 6
						And the agent has moved 1->2->5->4 then the agent is now stuck

						If the agent is stuck we have to end the episode
					*/
					stuckcount = 0;
					#pragma omp parallel for reduction(+ : stuckcount)
					for (const auto theaction : a) { //loop through all actions
						if (theaction->NextPositionValid(&mystate)) {
							if (past.find(theaction->GetNextPosition(&mystate)) == past.end()) {
								//break; // we are not stuck, we have more options
								#pragma omp cancel for
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
				
				omp_set_lock(&q_table_lock);
				valuefunctionstateplusone = qtable->find(takeaction->GetNextPosition(&mystate))->second.find(actionprime->GetName())->second;
				omp_unset_lock(&q_table_lock);

				if (valuefunctionstateplusone > maxaction) {
					maxaction = valuefunctionstateplusone;
					vprime = valuefunctionstateplusone;
				}
			}
		}

		omp_set_lock(&q_table_lock);
		qtable->find(mystate.CurrentPosition())->second.find(takeaction->GetName())->second = ( (1- alpha) * qtable->find(mystate.CurrentPosition())->second.find(takeaction->GetName())->second) + (alpha * (reward + vprime));
		omp_unset_lock(&q_table_lock);

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

	int rows = 4;
	int columns = 4;
	int start = 1;
	int finish = 16;
	int const ITERATIONS = 50;

	int num_threads = 1;
	#ifdef _OPENMP
		num_threads = 20;
		omp_set_num_threads(num_threads);
		char *hasCancel = getenv("OMP_CANCELLATION");
		if (hasCancel == nullptr) {
			cout << "Warning! cancel construct not set please run: (export OMP_CANCELLATION=true)" << endl;
		}
	#endif

	#pragma omp parallel
	{
		#pragma omp single nowait 
		{
			cout << "OpenMP cancellation = " << omp_get_cancellation() << endl;
			cout << "Number of threads = " << omp_get_num_threads() << endl;
		}
	}

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

	#pragma omp parallel for
	for (int i = 1;i <= rows * columns;i++) {
		map<string, double> newmap;
		for (const auto anaction : av) {
			newmap[anaction->GetName()] = 0;
		}
		q[i] = newmap;
	}

	////////////////////QLearning//////////////////
	omp_init_lock(&q_table_lock);
	#pragma omp parallel for schedule(dynamic, 10)
	for (int i = 0; i < ITERATIONS; i++) {
		TakePathToPosition(av, s, finish, &q);
	}

	vector<int> cp;
	cp=TakePathToPosition(av, s, finish, &q);
	omp_destroy_lock(&q_table_lock);

	cout << "The q table after " << ITERATIONS << " iterations:" << endl;
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
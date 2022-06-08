#pragma once
#include<string>
#include"State.h"

class Action {
public:
	virtual int GetNextPosition(State * s) = 0;
	virtual bool NextPositionValid(State * s) = 0;
	virtual string GetName() = 0;
	//virtual ostream& operator<<(ostream& os, const Action& dt);

	string name;
	int penalty;
	int winaward;
	Action() {
		name = "action";
		penalty = -1;
		winaward = 20;
	}

	int RewardFunction(State * s, int winningposition) {
		if (!NextPositionValid(s)) {
			return penalty;
		}

		if (GetNextPosition(s) == winningposition) {
			return winaward;
		}

		return 0;
	}

	// Overloading the << Operator.
	// The overloaded << operator function must then be declared as a friend of class Action so it can access the private data.
	friend ostream& operator<<(ostream& os, Action * dt);

protected:

	bool nextpositionvalid(int r, int c, int newrow, int newcol) {
		bool valid = true;
		if (newrow >= r || newrow < 0) {
			valid = false;
		}

		if (newcol >= c || newcol < 0) {
			valid = false;
		}
		return valid;
	}


};

class MoveRight : public Action {
public:
	string name = "right";

	int GetNextPosition(State * s) {
		return s->GetPosition(s->getcurrentrow(), 1 + s->getcurrentcol());
	}

	bool NextPositionValid(State * s) {
		bool v = false;
		if (nextpositionvalid(s->Rows(), s->Columns(), s->getcurrentrow(), 1 + s->getcurrentcol())) {
			v = true;
		}
		return v;
	}

	string GetName() { return this->name; }
};

class MoveDown : public Action {
public:
	string name = "down";
	int GetNextPosition(State * s) {
		return s->GetPosition(s->getcurrentrow() + 1, s->getcurrentcol());
	}

	bool NextPositionValid(State * s) {
		bool v = false;
		if (nextpositionvalid(s->Rows(), s->Columns(), s->getcurrentrow() + 1, s->getcurrentcol())) {
			v = true;
		}
		return v;
	}
	string GetName() { return this->name; }
};

class MoveUp : public Action {
public:
	string name = "up";

	int GetNextPosition(State * s) {
		return s->GetPosition(s->getcurrentrow() - 1, s->getcurrentcol());
	}

	bool NextPositionValid(State * s) {
		bool v = false;
		if (nextpositionvalid(s->Rows(), s->Columns(), s->getcurrentrow() - 1, s->getcurrentcol())) {
			v = true;
		}
		return v;
	}
	string GetName() { return this->name; }
};

class MoveLeft : public Action {
public:
	string name = "left";
	int GetNextPosition(State * s) {
		return s->GetPosition(s->getcurrentrow(), s->getcurrentcol() - 1);
	}

	bool NextPositionValid(State * s) {
		bool v = false;
		if (nextpositionvalid(s->Rows(), s->Columns(), s->getcurrentrow(), s->getcurrentcol() - 1)) {
			v = true;
		}
		return v;
	}
	string GetName() { return this->name; }
};

ostream& operator<<(ostream& os, Action * dt)
{
    os << dt->GetName();
    return os;
}
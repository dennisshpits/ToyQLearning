#pragma once
#include <iomanip>
#include<iostream>

using namespace std;

class State {
public:

	State(int Rows, int Columns, int Start) {
		r = Rows;
		c = Columns;
		currentposition = Start;

		matrix = new int*[r];
		for (int i = 0;i < r;i++) {
			matrix[i] = new int[c];
		}
		int assignnum = 1;
		for (int j = 0; j < r;j++) {
			for (int i = 0;i < c;i++) {
				matrix[j][i] = assignnum;
				assignnum++;
			}
		}
	}

	~State() {
		for (int i = 0; i < r; ++i)
			delete[] matrix[i];
		delete[] matrix;
	}

	void printmatrix() {
		const char separator = ' ';
		const int width = 4;

		for (int j = 0; j < r;j++) {
			for (int i = 0;i < c;i++) {
				cout << left << setw(width) << setfill(separator) << matrix[j][i];
			}
			cout << endl;
		}
	}

	int Rows() { return r; }
	int Columns() { return c; }
	int CurrentPosition() { return currentposition; }
	void UpdatePosition(int newposition) {
		currentposition = newposition;
	}
	void UpdatePosition(int row, int col) {
		currentposition = matrix[row][col];
	}
	int GetPosition(int row, int col) {
		return matrix[row][col];
	}

	int getcurrentrow() {
		int row = -1;
		for (int j = 0; j < r;j++) {
			for (int i = 0;i < c;i++) {
				if (matrix[j][i] == currentposition) {
					row = j;
					break;
				}
			}
		}

		return row;
	}

	int getcurrentcol() {
		int col = -1;
		for (int j = 0; j < r;j++) {
			for (int i = 0;i < c;i++) {
				if (matrix[j][i] == currentposition) {
					col = i;
					break;
				}
			}
		}

		return col;
	}

private:

	int** matrix;
	int r;
	int c;
	int currentposition;
};
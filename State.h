#pragma once
#include <iomanip>
#include <iostream>
// We only include omp.h in State since State is included everywhere else in the project
#include <omp.h>

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

	//copy constructor
	State(const State & obj) {
		r = obj.r; // copy the value
		c = obj.c;
		currentposition = obj.currentposition;

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

	// we can use openmp to search the row number
	// we are given a cell/grid value and we need to return the row location
	// OMP_CANCELLATION must be set
	int getcurrentrow() {
		int row = -1;
		int i;

		#pragma omp parallel for private(i)
		for (int j = 0; j < r;j++) {
			for (i = 0;i < c;i++) {
				
				// if a thread encounters a cancellation point, it checks if omp cancel was called
				#pragma omp cancellation point for

				if (matrix[j][i] == currentposition) {
					// since we expect only one thread to access row, there should be no race conditions
					row = j;

					// we want to stop all other threads
					// parallel break statement
					#pragma omp cancel for
					
					// we no longer need the break statment
					//break;
				}
			}
		}

		return row;
	}

	int getcurrentcol() {
		int col = -1;
		int i;

		#pragma omp parallel for private(i)
		for (int j = 0; j < r;j++) {
			for (i = 0;i < c;i++) {
				
				#pragma omp cancellation point for

				if (matrix[j][i] == currentposition) {
					col = i;
					#pragma omp cancel for
					//break;
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
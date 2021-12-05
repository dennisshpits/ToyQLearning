CC = g++
CFLAGS = -Wall -g

qlearningprogram: Source.cpp State.h Action.h 
	${CC} ${CFLAGS} -o qlearningprogram Source.cpp -fopenmp

clean:
	rm qlearningprogram
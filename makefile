CC = g++
CFLAGS = -std=c++17 -Wall -O3 -o maze.out
LIBS = -lncurses -lpthread
CXXFILES = main.cpp maze.cpp maze_solver.cpp

all:
	$(CC) $(CXXFILES) $(LIBS) $(CFLAGS)


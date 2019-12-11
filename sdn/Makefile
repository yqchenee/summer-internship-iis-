CFLAGS 	= -O3 -Wall -std=c++11

MAIN = main

all: tRun

		
tRun: src/graph.o src/tree.o src/algo.o src/main.o
	g++ -o a src/graph.o src/tree.o src/algo.o src/main.o

src/graph.o: src/graph.cpp src/graph.h src/util/heap.h src/define.h
	g++ -c $(CFLAGS) $< -o $@ 
src/tree.o: src/tree.cpp src/graph.h src/util/heap.h src/define.h
	g++ -c $(CFLAGS) $< -o $@ 
src/algo.o: src/algo.cpp src/algo.h src/define.h
	g++ -c $(CFLAGS) $< -o $@ 

src/main.o: src/main.cpp src/algo.h src/define.h
	g++ -c $(CFLAGS) $< -o $@ 

clean:
	rm -f a src/*.o 

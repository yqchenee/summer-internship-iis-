CFLAGS 	= -O3 -Wall -std=c++11

MAIN = main

all:  Run query_BQT query_BQT_k_U online_prunning
	@echo -n ""

Run: main readGraph metaGraph matrix mystring
	g++ -o Run src/main.o src/readGraph.o src/metaGraph.o src/matrix.o src/myString.o 
query_BQT : query_bqt 
	g++ -o BQT src/query_BQT.o src/readGraph.o src/metaGraph.o src/matrix.o src/myString.o 
query_BQT_k_U : query_bqtk
	g++ -o BQT_kU src/query_BQT_k_U.o src/readGraph.o src/metaGraph.o src/matrix.o src/myString.o
online_prunning : op
	g++ -o online_prunning src/online_query_prunning.o src/readGraph.o src/metaGraph.o src/matrix.o src/myString.o

main		: src/main.cpp
		g++ -c $(CFLAGS) $< -o src/main.o 
query_bqt   : src/query_BQT.cpp
		g++ -c $(CFLAGS) $< -o src/query_BQT.o
query_bqtk  : src/query_BQT_k_U.cpp
		g++ -c $(CFLAGS) $< -o src/query_BQT_k_U.o
op			: src/online_query_prunning.cpp
		g++ -c $(CFLAGS) $< -o src/online_query_prunning.o

readGraph	: src/readGraph.cpp src/metaGraph.h src/util.h
		g++ -c $(CFLAGS) $< -o src/readGraph.o
metaGraph	: src/metaGraph.cpp src/metaGraph.h src/util.h src/heap.h
		g++ -c $(CFLAGS) $< -o src/metaGraph.o
matrix		: src/matrix.cpp src/matrix.h src/util.h
		g++ -c $(CFLAGS) $< -o src/matrix.o
mystring	: src/myString.cpp src/util.h
		g++ -c $(CFLAGS) $< -o src/myString.o



clean:
	rm -f BQT Run BQT_kU online_prunning src/*.o 

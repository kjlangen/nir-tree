SXX = -std=c++11 	#Standard
FLAGS = -g -Wall	#Flags
DIR = src/include 	#Include directory
OBJECTS = geometry.o btree.o node.o rtree.o randomSquares.o randomPoints.o

# Build btree
btree.o:
	g++ ${SXX} -I ${DIR} -c src/btree/btree.cpp

# Build utils
geometry.o:
	g++ ${SXX} -I ${DIR} -c src/util/geometry.cpp

# Build node
node.o:
	g++ ${SXX} -I ${DIR} -c src/rtree/node.cpp

# Build rtree
rtree.o:
	g++ ${SXX} -I ${DIR} -c src/rtree/rtree.cpp

# Build benchmarks
randomSquares.o:
	g++ ${SXX} -I ${DIR} -c src/bench/randomSquares.cpp

randomPoints.o:
	g++ ${SXX} -I ${DIR} -c src/bench/randomPoints.cpp

# Build all together
all: ${OBJECTS}
	g++ ${SXX} src/main.cpp ${OBJECTS} -o bin/main -I src/include -lspatialindex

# Clean all together
clean:
	rm -rf bin/* *.o *.d

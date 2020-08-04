SXX = -std=c++11 	# Standard
FLAGS = -Wall	# Flags
DIR = src/include 	# Include directory
OBJECTS = geometry.o graph.o btree.o node.o rtree.o randomSquares.o randomPoints.o rPlusTree.o rPlusTreeNode.o \
		nirnode.o nirtree.o randomDisjointSquares.o splitPoints.o pencilPrinter.o
TESTS = testGeometry.o testRStarTree.o testRPlusTree.o

.PHONY : clean tests

# build btree
btree.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/btree/btree.cpp

# build utils
geometry.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/util/geometry.cpp

graph.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/util/graph.cpp

# build R* tree
node.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/rtree/node.cpp

rtree.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/rtree/rtree.cpp

# Build pencil printer
pencilPrinter.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/nirtree/pencilPrinter.cpp

# build NIR tree
nirnode.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/nirtree/node.cpp -o nirnode.o

nirtree.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/nirtree/nirtree.cpp

# Build benchmarks
randomSquares.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/bench/randomSquares.cpp

randomDisjointSquares.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/bench/randomDisjointSquares.cpp

randomPoints.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/bench/randomPoints.cpp

splitPoints.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/bench/splitPoints.cpp

# build R+ tree
rPlusTree.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/rplustree/rPlusTree.cpp

rPlusTreeNode.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/rplustree/rPlusTreeNode.cpp

# build tests
testGeometry.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/tests/testGeometry.cpp

testRStarTree.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/tests/testRStarTree.cpp

testRPlusTree.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/tests/testRPlusTree.cpp

# Build all together
all: ${OBJECTS}
	mkdir -p bin
	g++ ${SXX} ${FLAGS} src/main.cpp ${OBJECTS} -o bin/main -I ${DIR} -lspatialindex -lctemplate_nothreads

# unit tests
tests: ${OBJECTS} ${TESTS}
	mkdir -p bin
	g++ ${SXX} ${FLAGS} src/main.cpp ${OBJECTS} ${TESTS} -o bin/tests -I ${DIR} -lspatialindex \
 		-lctemplate_nothreads -DUNIT_TESTING

# Alter flags to include profiling
profileflags:
	$(eval FLAGS += -pg)

# Build all together with profiling
# Note: Problems will occur if files were previously compiled without -pg and were not altered since
profile: profileflags ${OBJECTS}
	mkdir -p bin
	g++ ${SXX} ${FLAGS} src/main.cpp ${OBJECTS} -o bin/profile -I ${DIR} -lspatialindex

# Clean all together
clean:
	rm -rf bin *.o *.d

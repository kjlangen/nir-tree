SXX = -std=c++11 	# Standard
FLAGS = -Wall	# Flags
DIR = src/include 	# Include directory
OBJECTS = geometry.o btree.o node.o rtree.o randomSquares.o randomPoints.o rPlusTree.o rPlusTreeNode.o
TESTS = testGeometry.o testRStarTree.o testRPlusTree.o

.PHONY : clean tests

# Build btree
btree.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/btree/btree.cpp

# Build utils
geometry.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/util/geometry.cpp

# Build node
node.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/rtree/node.cpp

# Build rtree
rtree.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/rtree/rtree.cpp

# Build benchmarks
randomSquares.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/bench/randomSquares.cpp

randomPoints.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/bench/randomPoints.cpp

rPlusTree.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/rplustree/rPlusTree.cpp

rPlusTreeNode.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/rplustree/rPlusTreeNode.cpp

testGeometry.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/tests/testGeometry.cpp

testRStarTree.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/tests/testRStarTree.cpp

testRPlusTree.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/tests/testRPlusTree.cpp

# Build all together
all: ${OBJECTS}
	mkdir -p bin
	g++ ${SXX} ${FLAGS} src/main.cpp ${OBJECTS} -o bin/main -I ${DIR} -lspatialindex

# unit tests
tests: ${OBJECTS} ${TESTS}
	mkdir -p bin
	g++ ${SXX} ${FLAGS} src/main.cpp ${OBJECTS} ${TESTS} -o bin/tests -I ${DIR} -lspatialindex -DUNIT_TESTING

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

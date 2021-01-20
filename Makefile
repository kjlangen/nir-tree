SXX = -std=c++11 # Standard
FLAGS = -Wall -O1 # Flags
DIR = src/include # Include directory
OBJECTS = geometry.o graph.o btree.o node.o rtree.o nirnode.o nirtree.o rPlusTree.o rPlusTreeNode.o rStarTree.o rstartreenode.o randomSquares.o randomPoints.o splitPoints.o pencilPrinter.o
TESTS = testRStarTree.o # testGeometry.o testRPlusTree.o - for now only test rstartree

.PHONY : clean tests

# Build btree
btree.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/btree/btree.cpp

# Build utils
geometry.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/util/geometry.cpp

graph.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/util/graph.cpp

# Build node
node.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/rtree/node.cpp

# Build rtree
rtree.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/rtree/rtree.cpp

# Build rplustree node
rPlusTreeNode.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/rplustree/rPlusTreeNode.cpp

# Build rplustree
rPlusTree.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/rplustree/rPlusTree.cpp

# Build rplustree node
rstartreenode.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/rstartree/node.cpp -o rstartreenode.o

# Build rplustree
rStarTree.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/rstartree/rStarTree.cpp

# Build pencil printer
pencilPrinter.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/nirtree/pencilPrinter.cpp

# Build nirtree node
nirnode.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/nirtree/node.cpp -o nirnode.o

# Build nirtree
nirtree.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/nirtree/nirtree.cpp

# Build benchmarks
randomSquares.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/bench/randomSquares.cpp

# randomDisjointSquares.o:
# 	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/bench/randomDisjointSquares.cpp

randomPoints.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/bench/randomPoints.cpp

splitPoints.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/bench/splitPoints.cpp

# Build tests
testGeometry.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/tests/testGeometry.cpp

testRStarTree.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/tests/testRStarTree.cpp

testRPlusTree.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/tests/testRPlusTree.cpp

# Unit tests
tests: ${OBJECTS} ${TESTS}
	mkdir -p bin
	g++ ${SXX} ${FLAGS} src/main.cpp ${OBJECTS} ${TESTS} -o bin/tests -I ${DIR} -lctemplate_nothreads -DUNIT_TESTING

# Clean all together
clean:
	rm -rf bin/* *.o *.d

# Alter flags to include profiling
profile:
	$(eval FLAGS += -pg)

perf:
	$(eval FLAGS += -ggdb)

# Alter flags to include debugging
debug:
	$(eval FLAGS += -DDEBUG)

# Alter flags to include statistics
stat:
	$(eval FLAGS += -DSTAT)

# Build all together
all: ${OBJECTS}
	mkdir -p bin
	g++ ${SXX} ${FLAGS} src/main.cpp ${OBJECTS} -o bin/main -I ${DIR} -lctemplate_nothreads

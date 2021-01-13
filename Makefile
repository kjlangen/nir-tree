SXX = -std=c++11 # Standard
FLAGS = -Wall -O3 -DDIM=3 # Flags
DIR = src/include # Include directory
OBJECTS = geometry.o graph.o btree.o node.o rtree.o nirnode.o nirtree.o rplustree.o rplusnode.o randomPoints.o bmpPrinter.o
TESTS = testGeometry.o testRStarTree.o testRPlusTree.o testNIRTree.o testBMPPrinter.o

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
rplusnode.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/rplustree/node.cpp -o rplusnode.o

# Build rplustree
rplustree.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/rplustree/rplustree.cpp

# Build nirtree node
nirnode.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/nirtree/node.cpp -o nirnode.o

# Build nirtree
nirtree.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/nirtree/nirtree.cpp

# Build pencil printer
# pencilPrinter.o:
#	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/util/pencilPrinter.cpp

# Build BMP printer
bmpPrinter.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/util/bmpPrinter.cpp

# Build benchmarks
randomPoints.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/bench/randomPoints.cpp

# Build tests
testGeometry.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/tests/testGeometry.cpp

testRStarTree.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/tests/testRStarTree.cpp

testRPlusTree.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/tests/testRPlusTree.cpp

testNIRTree.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/tests/testNIRTree.cpp

testBMPPrinter.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/tests/testBMPPrinter.cpp

# Unit tests
tests: ${OBJECTS} ${TESTS}
	mkdir -p bin
	g++ ${SXX} ${FLAGS} -DUNIT_TESTING src/main.cpp ${OBJECTS} ${TESTS} -o bin/tests -I ${DIR}

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
	$(eval FLAGS += -DDEBUG0)

trace:
	$(eval FLAGS += -DDEBUG1)

# Alter flags to include statistics
stat:
	$(eval FLAGS += -DSTAT)

# Build all together
all: ${OBJECTS}
	mkdir -p bin
	g++ ${SXX} ${FLAGS} src/main.cpp ${OBJECTS} -o bin/main -I ${DIR}

SXX = -std=c++11 	# Standard
FLAGS = -Wall	# Flags
DIR = src/include 	# Include directory
OBJECTS = geometry.o btree.o node.o rtree.o randomSquares.o randomPoints.o rPlusTree.o rPlusTreeNode.o

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

# Build all together
all: ${OBJECTS}
	mkdir -p bin
	g++ ${SXX} ${FLAGS} src/main.cpp ${OBJECTS} -o bin/main -I ${DIR} -lspatialindex

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

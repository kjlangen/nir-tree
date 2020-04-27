SXX = -std=c++11 	# Standard
FLAGS = -Wall	# Flags
DIR = src/include 	# Include directory
OBJECTS = geometry.o btree.o node.o rtree.o nirnode.o nirtree.o randomSquares.o randomPoints.o splitPoints.o

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

# Build nir tree node
nirnode.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/nirtree/node.cpp -o nirnode.o

# Build nirtree
nirtree.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/nirtree/nirtree.cpp

# Build benchmarks
randomSquares.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/bench/randomSquares.cpp

randomPoints.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/bench/randomPoints.cpp

splitPoints.o:
	g++ ${SXX} ${FLAGS} -I ${DIR} -c src/bench/splitPoints.cpp

# Build all together
all: ${OBJECTS}
	g++ ${SXX} ${FLAGS} src/main.cpp ${OBJECTS} -o bin/main -I src/include -lspatialindex

# Alter flags to include profiling
profileflags:
	$(eval FLAGS += -pg)

# Build all together with profiling
# Note: Problems will occur if files were previously compiled without -pg and were not altered since
profile: profileflags ${OBJECTS}
	g++ ${SXX} ${FLAGS} src/main.cpp ${OBJECTS} -o bin/profile -I src/include -lspatialindex

# Clean all together
clean:
	rm -rf bin/* *.o *.d

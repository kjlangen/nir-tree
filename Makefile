SXX = -std=c++11 	# Standard
FLAGS = -Wall	# Flags
INCLUDE = include 	# Include directory
OBJECTS = geometry.o btree.o node.o rtree.o randomSquares.o randomPoints.o

# Build btree
btree.o:
	g++ ${SXX} ${FLAGS} -I ${INCLUDE} -c src/btree/btree.cpp

# Build utils
geometry.o:
	g++ ${SXX} ${FLAGS} -I ${INCLUDE} -c src/util/geometry.cpp

# Build node
node.o:
	g++ ${SXX} ${FLAGS} -I ${INCLUDE} -c src/rtree/node.cpp

# Build rtree
rtree.o:
	g++ ${SXX} ${FLAGS} -I ${INCLUDE} -c src/rtree/rtree.cpp

# Build benchmarks
randomSquares.o:
	g++ ${SXX} ${FLAGS} -I ${INCLUDE} -c src/bench/randomSquares.cpp

randomPoints.o:
	g++ ${SXX} ${FLAGS} -I ${INCLUDE} -c src/bench/randomPoints.cpp

# Build all together
all: ${OBJECTS}
	g++ ${SXX} ${FLAGS} src/main.cpp ${OBJECTS} -o bin/main -I ${INCLUDE} -lspatialindex

# Alter flags to include profiling
profileflags:
	$(eval FLAGS += -pg)

# Build all together with profiling
# Note: Problems will occur if files were previously compiled without -pg and were not altered since
profile: profileflags ${OBJECTS}
	g++ ${SXX} ${FLAGS} src/main.cpp ${OBJECTS} -o bin/profile -I ${INCLUDE} -lspatialindex

# Clean all together
clean:
	rm -rf bin/* *.o *.d

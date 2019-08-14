SXX = -std=c++11 	#Standard
FLAGS = -g -Wall	#Flags
OBJECTS = geometry.o btree.o rtree.o

# Build btree
btree.o:
	g++ ${SXX} -I src/include -c src/btree/btree.cpp

# Build utils
geometry.o:
	g++ ${SXX} -I src/include -c src/util/geometry.cpp

# Build rtree
rtree.o:
	g++ ${SXX} -I src/include -c src/rtree/rtree.cpp

# Build all together
all: ${OBJECTS}
	g++ ${SXX} src/main.cpp ${OBJECTS} -o bin/main -I src/include

# Clean all together
clean:
	rm -rf bin/* *.o *.d

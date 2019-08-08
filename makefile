FLAGS = -g -Wall	#Flags

OBJBTREE = btree.o rtree.o

OBJECTS = ${OBJBTREE}

# Build btree
btree.o:
	g++ -c src/btree/btree.cpp

# Build rtree
rtree.o:
	g++ -c src/rtree/rtree.cpp

# Build all together
all: ${OBJECTS}
	g++ src/main.cpp ${OBJECTS} -o bin/main

# Clean all together
clean:
	rm -rf bin/* *.o *.d

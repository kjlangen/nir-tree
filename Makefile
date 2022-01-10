C++ := g++
DIR := src/include # Include directory
SXX := -std=c++20 # Standard
CXXFLAGS := -Wall -fno-strict-aliasing
CPPFLAGS := -DDIM=2 -I $(DIR)

ifdef PROD
CPPFLAGS := -DNDEBUG $(CPPFLAGS)
CXXFLAGS := $(CXXFLAGS) -O3
else ifdef EXP
CPPFLAGS := -DNDEBUG -DSTAT $(CPPFLAGS)
CXXFLAGS := -ggdb $(CXXFLAGS) -O3
else
CXXFLAGS := -ggdb $(CXXFLAGS)
endif

SRC := $(shell find . -path ./src/tests -prune -o \( -name '*.cpp' -a ! -name 'pencilPrinter.cpp' \) -print)
MAINS := ./src/main.cpp ./src/gen_tree.cpp
TREE_NODES := ./src/nirtree/node.o ./src/rtree/node.o ./src/rplustree/node.o ./src/rstartree/node.o ./src/quadtree/node.o ./src/revisedrstartree/node.o
SRC := $(filter-out $(MAINS),$(SRC))
OBJ := $(SRC:.cpp=.o)
OBJ_TO_COPY := $(filter-out $(TREE_NODES),$(OBJ))

TESTSRC := $(shell find ./src/tests -name '*.cpp')
TESTOBJ := $(TESTSRC:.cpp=.o)

%.o: %.cpp
	$(C++) $(SXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

all: bin/main bin/gen_tree bin/tests

src/main.o : src/main.cpp
	$(C++) $(SXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

src/gen_tree.o : src/gen_tree.cpp
	$(C++) $(SXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@


bin/main: $(OBJ) src/main.o
	mkdir -p bin
	cp src/nirtree/node.o bin/nirtreenode.o
	cp src/rtree/node.o bin/rtreenode.o
	cp src/rplustree/node.o bin/rplustreenode.o
	cp src/rstartree/node.o bin/rstartreenode.o
	cp src/quadtree/node.o bin/quadtreenode.o
	cp src/revisedrstartree/node.o bin/revisedrstartreenode.o
	rm -rf test*.o
	cp $(OBJ_TO_COPY) bin/
	$(C++) $(SXX) $(CXXFLAGS) $(CPPFLAGS) bin/*.o src/main.o -o bin/main -I $(DIR)

bin/gen_tree: $(OBJ) bin/main src/gen_tree.o
	$(C++) $(SXX) $(CXXFLAGS) $(CPPFLAGS) bin/*.o src/gen_tree.o -o bin/gen_tree -I $(DIR)

bin/tests: $(TESTOBJ) bin/main
	$(C++) $(SXX) $(CXXFLAGS) $(CPPFLAGS) bin/*.o src/tests/*.o -o bin/tests

.PHONY: all clean prod

clean:
	rm -rf bin/*
	find . -name "*.o" -delete
	find . -name "*.d" -delete

C++ = g++
DIR = src/include # Include directory
SXX = -std=c++20 # Standard
CXXFLAGS = -Wall -fno-strict-aliasing
CPPFLAGS = -DDIM=2 -I $(DIR)

ifdef PROD
CPPFLAGS := -DNDEBUG $(CPPFLAGS)
CXXFLAGS := $(CXXFLAGS) -O3
else ifdef EXP
CPPFLAGS := -DNDEBUG -DSTAT $(CPPFLAGS)
CXXFLAGS := $(CXXFLAGS) -O3
else
CXXFLAGS := -ggdb $(CXXFLAGS)
endif

SRC = $(shell find . -path ./src/tests -prune -false -o \( -name '*.cpp' -a ! -name 'pencilPrinter.cpp' \) )
OBJ = $(SRC:.cpp=.o)
TESTSRC = $(shell find ./src/tests -name '*.cpp')
TESTOBJ = $(TESTSRC:.cpp=.o)

%.o: %.cpp
	$(C++) $(SXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

all: bin/main bin/gen_tree bin/tests

bin/main: $(OBJ)
	mkdir -p bin
	cp src/nirtree/node.o nirtreenode.o
	cp src/rtree/node.o rtreenode.o
	cp src/rplustree/node.o rplustreenode.o
	cp src/rstartree/node.o rstartreenode.o
	cp src/quadtree/node.o quadtreenode.o
	cp src/revisedrstartree/node.o revisedrstartreenode.o
	find ./src \( -name "*.o" -a ! -name 'node.o' \) -exec cp {} ./ \;
	rm -rf test*.o
	mv gen_tree.o gen_tree.nocompile || echo
	$(C++) $(SXX) $(CXXFLAGS) $(CPPFLAGS) *.o -o bin/main -I $(DIR)
	mv gen_tree.nocompile gen_tree.o || echo

bin/gen_tree: $(OBJ)
	mkdir -p bin
	mv main.o main.nocompile || echo
	$(C++) $(SXX) $(CXXFLAGS) $(CPPFLAGS) *.o -o bin/gen_tree -I $(DIR)
	mv main.nocompile main.o || echo

bin/tests: $(TESTOBJ) bin/main
	find ./src/tests \( -name "*.o" -a ! -name 'node.o' \) -exec cp {} ./ \;
	mv main.o main.nocompile || echo
	mv gen_tree.o gen_tree.nocompile || echo 
	$(C++) $(SXX) $(CXXFLAGS) $(CPPFLAGS) *.o -o bin/tests
	mv main.nocompile main.o || echo
	mv gen_tree.nocompile gen_tree.o || echo 

.PHONY: all clean prod

clean:
	rm -rf bin/*
	find . -name "*.o" -delete
	find . -name "*.d" -delete

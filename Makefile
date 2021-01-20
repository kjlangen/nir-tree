C++ = g++
SXX = -std=c++11 # Standard
C++FLAGS = -g -Wall -O3 -DDIM=2 # Flags
DIR = src/include # Include directory
SRC = $(shell find . -path ./src/tests -prune -false -o \( -name '*.cpp' -a ! -name 'pencilPrinter.cpp' \) )
OBJ = $(SRC:.cpp=.o)
TESTSRC = $(shell find ./src/tests -name '*.cpp')
TESTOBJ = $(TESTSRC:.cpp=.o)

%.o: %.cpp
	$(C++) $(SXX) $(C++FLAGS) -I $(DIR) -c $< -o $@

all: bin/main bin/tests

bin/main: $(OBJ)
	mkdir -p bin
	cp src/nirtree/node.o nirtreenode.o
	cp src/rtree/node.o rtreenode.o
	cp src/rplustree/node.o rplustreenode.o
	cp src/rstartree/node.o rtstartreenode.o
	find ./src \( -name "*.o" -a ! -name 'node.o' \) -exec cp {} ./ \;
	rm test*.o
	$(C++) $(SXX) $(C++FLAGS) *.o -o bin/main -I $(DIR)

bin/tests: $(TESTOBJ) bin/main
	find ./src/tests \( -name "*.o" -a ! -name 'node.o' \) -exec cp {} ./ \;
	mv main.o main.nocompile || echo
	$(C++) $(SXX) $(C++FLAGS) *.o -o bin/tests -I $(DIR)
	mv main.nocompile main.o || echo
    
.PHONY: all clean

clean:
	rm -rf bin/*
	find . -name "*.o" -delete
	find . -name "*.d" -delete

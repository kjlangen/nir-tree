C++ = g++-9
DIR = src/include # Include directory
SXX = -std=c++17 # Standard
CXXFLAGS = -Wall
CPPFLAGS = -DDIM2 -I $(DIR)

ifndef PROD
CXXFLAGS := -ggdb $(CXXFLAGS)
else
CPPFLAGS := -DNDEBUG $(CPPFLAGS)
CXXFLAGS := $(CXXFLAGS) -O3
endif

SRC = $(shell find . -path ./src/tests -prune -false -o \( -name '*.cpp' -a ! -name 'pencilPrinter.cpp' \) )
OBJ = $(SRC:.cpp=.o)
TESTSRC = $(shell find ./src/tests -name '*.cpp')
TESTOBJ = $(TESTSRC:.cpp=.o)

%.o: %.cpp
	$(C++) $(SXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

all: bin/main bin/tests

bin/main: $(OBJ)
	mkdir -p bin
	cp src/nirtree/node.o nirtreenode.o
	cp src/rtree/node.o rtreenode.o
	cp src/rplustree/node.o rplustreenode.o
	cp src/rstartree/node.o rtstartreenode.o
	find ./src \( -name "*.o" -a ! -name 'node.o' \) -exec cp {} ./ \;
	rm -rf test*.o
	$(C++) $(SXX) $(CXXFLAGS) $(CPPFLAGS) *.o -o bin/main -I $(DIR)

bin/tests: $(TESTOBJ) bin/main
	find ./src/tests \( -name "*.o" -a ! -name 'node.o' \) -exec cp {} ./ \;
	mv main.o main.nocompile || echo
	$(C++) $(SXX) $(CXXFLAGS) $(CPPFLAGS) *.o -o bin/tests
	mv main.nocompile main.o || echo

.PHONY: all clean prod

clean:
	rm -rf bin/*
	find . -name "*.o" -delete
	find . -name "*.d" -delete

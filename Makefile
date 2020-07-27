.PHONY: test clean

PROGNAME = graphd

CXXFLAGS = -std=c++17 -Iinclude -Wall -Wextra -Wpedantic
OPT = -O2
TESTLIBS = -lgtest -lgtest_main

SRC = src
INC = include
BIN = bin
OBJ = obj
TSRC = test/src
TBIN = test/bin

all: $(BIN)/$(PROGNAME)

vpath %.cpp $(SRC)
vpath %.hpp $(INC)/graphd:$(INC)/graphd/input:$(INC)/graphd/input/parser

ALLOBJS = $(patsubst $(SRC)/%.cpp,$(OBJ)/%.o,$(wildcard $(SRC)/*.cpp))

$(BIN)/$(PROGNAME): main.cpp $(ALLOBJS) | $(BIN)
	$(CXX) $(CXXFLAGS) $(OPT) $^ -o $@

$(OBJ)/%.o: %.cpp %.hpp | $(OBJ)
	$(CXX) -c $(CXXFLAGS) $(OPT) $< -o $@

test: token_test parse_test graph_test

%_test: $(TBIN)/%_test
	$<

$(TBIN)/%_test: $(TSRC)/%_test.cpp $(ALLOBJS) | $(TBIN)
	$(CXX) $(CXXFLAGS) $(TESTLIBS) $^ -o $@

clean:
	rm -f $(BIN)/* $(OBJ)/* $(TBIN)/*

$(BIN) $(TBIN) $(OBJ):
	mkdir -p $@

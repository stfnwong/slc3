# LC-3 EMULATOR MAKEFILE
# This version adjusted for C++14
#
# Stefan Wong 2018
#

# OUTPUT DIRS
BIN_DIR=bin
OBJ_DIR=obj
SRC_DIR=src
TEST_DIR=test
TEST_BIN_DIR=$(BIN_DIR)/test
TOOL_DIR=tools

# Tool options
CXX=g++
OPT=-O0
CXXFLAGS=-Wall -g2 -pthread -std=c++14 -D_REENTRANT $(OPT)
TESTFLAGS=-lgtest -lgtest_main
LDFLAGS =$(shell root-config --ldflags) -pthread
LIBS = 
TEST_LIBS = -lgtest -lgtest_main

# Object targets
INCS=-I$(SRC_DIR)
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
# Unit tests 
TEST_SOURCES  = $(wildcard $(TEST_DIR)/*.cpp)
# Tools (program entry points)
TOOL_SOURCES = $(wildcard $(TOOL_DIR)/*.cpp)

.PHONY: clean

# Generic build 
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEPS)
	$(CXX) -c $< -o $@ $(CXXFLAGS)

OBJECTS := $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

$(OBJECTS): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

TEST_OBJECTS  := $(TEST_SOURCES:$(TEST_DIR)/%.cpp=$(OBJ_DIR)/%.o)

$(TEST_OBJECTS): $(OBJ_DIR)/%.o : $(TEST_DIR)/%.cpp 
	$(CXX) $(CXXFLAGS) $(INCS) -c $< -o $@ 

TOOL_OBJECTS  := $(TOOL_SOURCES:$(TOOL_DIR)/%.cpp=$(OBJ_DIR)/%.o)

$(TOOL_OBJECTS): $(OBJ_DIR)/%.o : $(TOOL_DIR)/%.cpp 
	$(CXX) $(CXXFLAGS) $(INCS) -c $< -o $@ 

# ======== UNIT TEST TARGETS ======== #
TESTS=test_machine test_lc3 test_mtrace test_lexer test_opcode \
	  test_assembler test_sourceinfo test_binary test_disassembler

$(TESTS): $(OBJECTS) $(TEST_OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) $(OBJ_DIR)/$@.o \
		$(INCS) -o $(TEST_BIN_DIR)/$@ $(LIBS) $(TEST_LIBS)

# ======== TOOL TARGETS ========= #
TOOLS = lc3asm 
# TODO : lc3dis
#
$(TOOLS): $(OBJECTS) $(TOOL_OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) $(OBJ_DIR)/$@.o \
		$(INCS) -o $(BIN_DIR)/$@ $(LIBS) 


# Main targets 
all : tools test 

tools: $(TOOLS)

test : $(TESTS)

clean:
	rm -rfv *.o $(OBJ_DIR)/*.o 

print-%:
	@echo $* = $($*)

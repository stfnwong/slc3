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
TEST_SOURCES  = $(wildcard $(TEST_DIR)/*.cpp)

.PHONY: clean

# Generic build 
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEPS)
	$(CXX) -c $< -o $@ $(CXXFLAGS)

OBJECTS := $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
TEST_OBJECTS  := $(TEST_SOURCES:$(TEST_DIR)/%.cpp=$(OBJ_DIR)/%.o)

$(OBJECTS): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo "Compiled "$<" "

$(TEST_OBJECTS): $(OBJ_DIR)/%.o : $(TEST_DIR)/%.cpp 
	$(CXX) $(CXXFLAGS) $(INCS) -c $< -o $@ 
	@echo "Compiled test object "$<""

# ======== UNIT TEST TARGETS ======== #
TESTS=test_machine test_lc3 test_mtrace test_lexer test_opcode

$(TESTS): $(OBJECTS) $(TEST_OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) $(OBJ_DIR)/$@.o \
		$(INCS) -o $(TEST_BIN_DIR)/$@ $(LIBS) $(TEST_LIBS)


# Main targets 
all : test

test : $(TESTS)

clean:
	rm -rfv *.o $(OBJ_DIR)/*.o 

print-%:
	@echo $* = $($*)

# LC-3 EMULATOR MAKEFILE
# Stefan Wong 2018
#

# OUTPUT DIRS
BIN_DIR=bin
OBJ_DIR=obj
SRC_DIR=src
TEST_DIR=test
TEST_BIN_DIR=$(BIN_DIR)/test

# Tool options
CC=gcc
OPT=-O0
CFLAGS=-Wall -g2 -pthread -D_REENTRANT $(OPT)
TESTFLAGS=-lgtest -lgtest_main
LDFLAGS =$(shell root-config --ldflags) -pthread
LIBS = 
TEST_LIBS = -lcheck


# Object targets
INCS=-I$(SRC_DIR)
SOURCES = $(wildcard $(SRC_DIR)/*.c)
TEST_SOURCES  = $(wildcard $(TEST_DIR)/*.c)

.PHONY: clean

# Generic build 
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS)
	$(CC) -c $< -o $@ $(CFLAGS)

OBJECTS := $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
TEST_OBJECTS  := $(TEST_SOURCES:$(TEST_DIR)/%.c=$(OBJ_DIR)/%.o)

$(OBJECTS): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" "

$(TEST_OBJECTS): $(OBJ_DIR)/%.o : $(TEST_DIR)/%.c 
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@ 
	@echo "Compiled test object "$<""

# ======== UNIT TEST TARGETS ======== #
test_state: $(OBJECTS) $(TEST_OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(OBJ_DIR)/$@.o \
		$(INCS) -o $(TEST_BIN_DIR)/$@ $(LIBS) $(TEST_LIBS)


all : test

# ======== TESTS ======== 
test : test_state

clean:
	rm -rfv *.o $(OBJ_DIR)/*.o 

print-%:
	@echo $* = $($*)

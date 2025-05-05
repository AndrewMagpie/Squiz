SHELL := /bin/bash
####################################################################################
# Program files
PROGRAM_DIR=./bin
PROGRAM_NAME=squiz
PROGRAM_SOURCE_DIR=./src
PROGRAM=$(PROGRAM_DIR)/$(PROGRAM_NAME)
####################################################################################
# Source files.
SRCS_CPP = $(wildcard $(PROGRAM_SOURCE_DIR)/*.cpp)
OBJECTS_CPP = $(addprefix $(PROGRAM_DIR)/obj/, $(patsubst ./%,%,$(SRCS_CPP:.cpp=.o)))
OBJECTS = $(OBJECTS_CPP)
####################################################################################
# Run options.
RUN_OPTIONS ?=
####################################################################################
# Compiler / Linker
CC=g++
CC_FLAGS=-Wall -O2 -std=c++20
LK_FLAGS=
####################################################################################
all:  $(PROGRAM)
####################################################################################
# Ensure output directory exists
$(PROGRAM_DIR):
	mkdir -p $(PROGRAM_DIR)
####################################################################################
# Build the program
$(PROGRAM):       $(PROGRAM_DIR)  $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(LK_FLAGS) $(OBJECTS) -o $@
	@echo Built: $@
####################################################################################
# Build the object files.
$(PROGRAM_DIR)/obj/%.o: ./%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CC_FLAGS) -c $< -o $@ 
####################################################################################
# Delete
clean:
	rm -rf $(PROGRAM_DIR)
####################################################################################
# Rebuild (clean and build)
rebuild: clean all
####################################################################################
run: all
	@echo Running: $(PROGRAM) $(RUN_OPTIONS)
	@$(PROGRAM) $(RUN_OPTIONS) || true	
####################################################################################

TestReturnCode = \
	echo "$(1) - Search with $(2) results:  $(3)"; \
	$(PROGRAM) -f "$(3)" -q -rc; \
	RC=$$?; \
	if [ $$RC -ne $(2) ]; then echo "$(1) - Return code: $$RC.  Expected: $(2)"; exit $$RC; fi; \
	exit 0;


TESTS := $(foreach i,$(shell seq 1 3),Test$(i))

tests: all $(TESTS)
	@echo "All tests passed!"

#No matches.
Test1:
	@$(call TestReturnCode, $@,0,FooeyGooey)

#1 match.
Test2:
	@$(call TestReturnCode, $@,1,Death-light of Africa!)

#Many matches.
Test3:
	@$(call TestReturnCode, $@,16,Don John of Austria)


####################################################################################



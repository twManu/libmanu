# A sample Makefile for building Google Test and using it in user
# tests.  Please tweak it to suit your environment and project.  You
# may want to move it to your project's root directory.
#
# SYNOPSIS:
#
#   make [all]  - makes everything.
#   make TARGET - makes the given target.
#   make clean  - removes all files generated by make.

# Please tweak the following variable definitions as needed by your
# project, except GTEST_HEADERS, which you can use in your own targets
# but shouldn't modify.

# Points to the root of Google Test, relative to where this file is.
# Remember to tweak this if you move this file.

# All tests produced by this Makefile.  Remember to add new tests you
# created to the list.


TESTS += testCgetchar unittest macroTest
INC_DIR = ../
GTEST_DIR = /usr/src/gtest


# specify user object files... so each need a rule.
USER_OBJ := testV4Addr.o

# Flags passed to the preprocessor.
CPPFLAGS += -I$(GTEST_DIR)/include -I$(INC_DIR)

# Flags passed to the C++ compiler.
#DEBUG and EXTRA_FLAG from makefile.inc
CXXFLAGS += -g -Wall -Wextra $(EXTRA_FLAG)


# House-keeping build targets.
.PHONY: all clean

all : $(TESTS)

clean :
	@rm -f $(TESTS) *.o

# Builds a sample test.  A test should link with either gtest.a or
# gtest_main.a, depending on whether it defines its own main()
# function.

###
# dependency
###
unittest : $(USER_OBJ)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ -lgtest -lgtest_main -lpthread

testCgetchar: testCgetchar.c
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@
	
macroTest: macroTest.cpp
	$(CXX) -E -o macroTest.i $^
	indent -kr macroTest.i
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@

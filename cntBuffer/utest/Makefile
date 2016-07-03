
GTEST_HEADERS = /usr/include/gtest/*.h /usr/include/gtest/internal/*.h
GTEST_DIR = /usr/src/gtest
CPPFLAGS += -I$(GTEST_DIR)/include -I ../../
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)
TESTS = utest

.PHONY: clean

utest: cntBuffer.o utest.o gtest_main.a OSLinux.o
	$(CROSS_COMPILE)$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ -lpthread

cntBuffer.o: ../cntBuffer.cpp ../cntBuffer.hpp ../../OSLinux.h
	$(CROSS_COMPILE)$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

OSLinux.o: ../../OSLinux.cpp ../../OSLinux.h
	$(CROSS_COMPILE)$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
	
utest.o: utest.cc
	$(CROSS_COMPILE)$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
	
clean :
	@rm -rf $(TESTS) gtest.a gtest_main.a *.gcda *.gcno *.o


# For simplicity and to avoid depending on Google Test's
# implementation details, the dependencies specified below are
# conservative and not optimized.  This is fine as Google Test
# compiles fast and for ordinary users its source rarely changes.
gtest-all.o : $(GTEST_SRCS_)
	$(CROSS_COMPILE)$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest-all.cc -o gtest-all.o

gtest_main.o : $(GTEST_SRCS_)
	$(CROSS_COMPILE)$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest_main.cc -o gtest_main.o

gtest.a : gtest-all.o
	$(CROSS_COMPILE)$(AR) $(ARFLAGS) $@ $^

gtest_main.a : gtest-all.o gtest_main.o
	$(CROSS_COMPILE)$(AR) $(ARFLAGS) $@ $^
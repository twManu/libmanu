TARGET=Buffer_utest BufferList_utest

GTEST_HEADERS = /usr/include/gtest/*.h /usr/include/gtest/internal/*.h
GTEST_DIR = /usr/src/gtest
CFLAGS += -I$(GTEST_DIR)/include
GTEST_SRCS_ = $(GTEST_DIR)/src/gtest-all.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

CFLAGS += -Wall -O2
LDFLAGS=-lpthread
AR=ar

include ../macro.inc

#all objs put in current dir to avoid mixed result across projects
OBJ_Buffer_utest=Buffer.o Buffer_utest.o gtest_main.a
OBJ_BufferList_utest=Buffer.o BufferList.o BufferList_utest.o gtest_main.a

#c and h use the same across projects
DEP_Buffer=Buffer.h Buffer.cpp ../libcommon.h
DEP_BufferList=Buffer.h BufferList.h BufferList.cpp ../BaseLock.h ../libcommon.h
DEP_Buffer_utest=Buffer_utest.cc Buffer.h
DEP_BufferList_utest=BufferList_utest.cc Buffer.h BufferList.h ../BaseLock.h ../libcommon.h

all: $(TARGET)

BufferList_utest: $(OBJ_BufferList_utest)
	$(CROSS_COMPILE)g++ -o $@ $^ $(LDFLAGS)

Buffer_utest: $(OBJ_Buffer_utest)
	$(CROSS_COMPILE)g++ -o $@ $^ $(LDFLAGS)

BufferList_utest.o: $(DEP_BufferList_utest)
	$(CROSS_COMPILE)g++ -c $(CFLAGS) -o $@ $(FIRST_C_IN_DEP)

Buffer_utest.o: $(DEP_Buffer_utest)
	$(CROSS_COMPILE)g++ -c $(CFLAGS) -o $@ $(FIRST_C_IN_DEP)

Buffer.o: $(DEP_Buffer)
	$(CROSS_COMPILE)g++ -c $(CFLAGS) -o $@ $(FIRST_C_IN_DEP)

BufferList.o: $(DEP_BufferList)
	$(CROSS_COMPILE)g++ -c $(CFLAGS) -o $@ $(FIRST_C_IN_DEP)

utest.o: $(DEP_utest)
	$(CROSS_COMPILE)g++ -c $(CFLAGS) -o $@ $(FIRST_C_IN_DEP)

clean:
	rm -f $(TARGET) gtest.a gtest_main.a *.o 


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

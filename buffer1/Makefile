TARGET=Buffer_utest BufferList_utest TeeBuffer_utest TeeInList_utest
GTEST_DIR=../gtest
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h $(GTEST_DIR)/include/gtest/internal/*.h
CFLAGS += -I$(GTEST_DIR)/include
GTEST_SRCS_ = $(GTEST_DIR)/src/gtest-all.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

CFLAGS += -Wall -O2
CPPFLAGS=$(CFLAGS)
LDFLAGS+=-lpthread
AR=ar


#all objs put in current dir to avoid mixed result across projects
OBJ_Buffer_utest=Buffer.o Buffer_utest.o gtest_main.a
OBJ_BufferList_utest=Buffer.o BufferList.o BufferList_utest.o gtest_main.a
OBJ_TeeBuffer_utest=TeeBuffer.o VideoBuffer.o Buffer.o BufferList.o TeeBuffer_utest.o gtest_main.a
OBJ_TeeInList_utest=TeeBuffer.o VideoBuffer.o Buffer.o BufferList.o\
	TeeInList.o TeeOutList.o TeeInList_utest.o gtest_main.a

#c and h use the same across projects
DEP_Buffer=Buffer.cpp Buffer.h
DEP_BufferList=BufferList.cpp BufferList.h $(DEP_Buffer)
DEP_Buffer_utest=Buffer_utest.cc $(DEP_Buffer)
DEP_BufferList_utest=BufferList_utest.cc $(DEP_BufferList)
DEP_VideoBuffer=VideoBuffer.cpp VideoBuffer.h $(DEP_Buffer)
DEP_TeeBuffer=TeeBuffer.cpp TeeBuffer.h $(DEP_VideoBuffer)
DEP_TeeBuffer_utest=TeeBuffer_utest.cc $(DEP_TeeBuffer)
DEP_TeeOutList=TeeOutList.cpp TeeOutList.h $(DEP_BufferList)
DEP_TeeInList=TeeInList.cpp TeeInList.h $(DEP_TeeOutList)
DEP_TeeInList_utest=TeeInList_utest.cc $(DEP_TeeBuffer) $(DEP_TeeInList)

all: $(TARGET)

BufferList_utest: $(OBJ_BufferList_utest)
	$(CROSS_COMPILE)g++ -o $@ $^ $(LDFLAGS)

Buffer_utest: $(OBJ_Buffer_utest)
	$(CROSS_COMPILE)g++ -o $@ $^ $(LDFLAGS)

TeeBuffer_utest: $(OBJ_TeeBuffer_utest)
	$(CROSS_COMPILE)g++ -o $@ $^ $(LDFLAGS)

TeeInList_utest: $(OBJ_TeeInList_utest)
	$(CROSS_COMPILE)g++ -o $@ $^ $(LDFLAGS)

BufferList_utest.o: $(DEP_BufferList_utest)
	$(CROSS_COMPILE)g++ -c $(CFLAGS) -o $@ $<

Buffer_utest.o: $(DEP_Buffer_utest)
	$(CROSS_COMPILE)g++ -c $(CFLAGS) -o $@ $<

Buffer.o: $(DEP_Buffer)
	$(CROSS_COMPILE)g++ -c $(CFLAGS) -o $@ $<

BufferList.o: $(DEP_BufferList)
	$(CROSS_COMPILE)g++ -c $(CFLAGS) -o $@ $<

VideoBuffer.o: $(DEP_VideoBuffer)
	$(CROSS_COMPILE)g++ -c $(CFLAGS) -o $@ $<

# TeeInList.h avoid recuirsive 
TeeOutList.o: $(DEP_TeeOutList) TeeInList.h
	$(CROSS_COMPILE)g++ -c $(CFLAGS) -o $@ $<

TeeInList.o: $(DEP_TeeInList) TeeOutList.o
	$(CROSS_COMPILE)g++ -c $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET) gtest.a gtest_main.a *.o 


# For simplicity and to avoid depending on Google Test's
# implementation details, the dependencies specified below are
# conservative and not optimized.  This is fine as Google Test
# compiles fast and for ordinary users its source rarely changes.
gtest-all.o : $(GTEST_SRCS_)
	$(CROSS_COMPILE)g++ $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest-all.cc -o gtest-all.o

gtest_main.o : $(GTEST_SRCS_)
	$(CROSS_COMPILE)g++ $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest_main.cc -o gtest_main.o

gtest.a : gtest-all.o
	$(CROSS_COMPILE)$(AR) $(ARFLAGS) $@ $^

gtest_main.a : gtest-all.o gtest_main.o
	$(CROSS_COMPILE)$(AR) $(ARFLAGS) $@ $^

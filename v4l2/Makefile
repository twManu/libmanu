TARGET=V4l2BaseTest V4l2MMapTest
CFLAGS=-Wall -O2
LDFLAGS=-lpthread

include ../macro.inc

#all objs put in current dir to avoid mixed result across projects
OBJ_V4l2BaseTest=V4l2BaseTest.o V4l2Base.o Buffer.o
OBJ_V4l2MMapTest=V4l2Base.o V4l2MMap.o V4l2MMapTest.o Buffer.o OSLinux.o

#c and h use the same across projects
DEP_V4l2Base=V4l2Base.cpp V4l2Base.h ../libcommon.h ../buffer/Buffer.h
DEP_V4l2BaseTest=V4l2BaseTest.cpp V4l2Base.h ../buffer/Buffer.h
DEP_V4l2MMapTest=V4l2MMapTest.cpp V4l2Base.h ../buffer/Buffer.h
DEP_Buffer=../buffer/Buffer.h ../buffer/Buffer.cpp
DEP_V4l2MMap=V4l2MMap.cpp V4l2MMap.h V4l2Base.h ../BaseLock.h ../BaseThread.h ../OSLinux.h
DEP_OSLinux=../BaseLock.h ../OSLinux.h ../OSLinux.cpp ../BaseThread.h ../TimeSec.hpp

all: $(TARGET)

V4l2BaseTest: $(OBJ_V4l2BaseTest)
	$(CROSS_COMPILE)g++ -o $@ $(O_IN_DEP) $(LDFLAGS)

V4l2MMapTest: $(OBJ_V4l2MMapTest)
	$(CROSS_COMPILE)g++ -o $@ $(O_IN_DEP) $(LDFLAGS)

Buffer.o: $(DEP_Buffer)
	$(CROSS_COMPILE)g++ -c $(CFLAGS) -o $@ $(FIRST_C_IN_DEP)

V4l2Base.o: $(DEP_V4l2Base)
	$(CROSS_COMPILE)g++ -c $(CFLAGS) -o $@ $(FIRST_C_IN_DEP)

V4l2MMap.o: $(DEP_V4l2MMap)
	$(CROSS_COMPILE)g++ -c $(CFLAGS) -o $@ $(FIRST_C_IN_DEP)

V4l2BaseTest.o: $(DEP_V4l2BaseTest)
	$(CROSS_COMPILE)g++ -c $(CFLAGS) -o $@ $(FIRST_C_IN_DEP)

OSLinux.o: $(DEP_OSLinux)
	$(CROSS_COMPILE)g++ -c $(CFLAGS) -o $@ $(FIRST_C_IN_DEP)

#%.o: %.c
#	$(CROSS_COMPILE)g++ -c $(CFLAGS) $<

#%.o : %.cpp
#	$(CROSS_COMPILE)g++ -c $(CFLAGS) $<

clean:
	rm -f $(TARGET) *.o 

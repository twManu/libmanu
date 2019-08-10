TARGET=y4mTest
CFLAGS=-Wall -O2 -I../ -DDEF_DEBUG -DIF_LINUX_TYPE_DEBUG=1
LDFLAGS=-lpthread
LIBMANU=..

include ${LIBMANU}/macro.inc

#all objs put in current dir to avoid mixed result across projects
OBJ_y4mTests=y4mTest.o y4mLib.o

#c and h use the same across projects
DEP_y4mLib=y4mLib.cpp y4mLib.h ../libcommon.h
DEP_y4mTest=y4mTest.cpp y4mLib.h ../libcommon.h

all: $(TARGET)

y4mTest: $(OBJ_y4mTests)
	$(CROSS_COMPILE)g++ -o $@ $(O_IN_DEP) $(LDFLAGS)


y4mLib.o: $(DEP_y4mLib)
	$(CROSS_COMPILE)g++ -c $(CFLAGS) -o $@ $(FIRST_C_IN_DEP)

y4mTest.o: $(DEP_y4mTest)
	$(CROSS_COMPILE)g++ -c $(CFLAGS) -o $@ $(FIRST_C_IN_DEP)


OSLinux.o: $(DEP_OSLinux)
	$(CROSS_COMPILE)g++ -c $(CFLAGS) -o $@ $(FIRST_C_IN_DEP)


clean:
	rm -f $(TARGET) *.o 

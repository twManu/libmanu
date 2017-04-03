# DBG_FLAGS =
# IF_LINUX_TYPE_DEBUG means (arg...) in macro, the opposite is (...) in macro
LIBMANU=.

include ${LIBMANU}/macro.inc
DBG_FLAGS = -DDEF_DEBUG -DIF_LINUX_TYPE_DEBUG -DDEF_DEBUG_THREAD
LDFLAGS = -lpthread -lrt
CFLAGS = -Wall -O2 -DDEF_LINUX -lpthread -lrt $(DBG_FLAGS)
OSDEP_HDR = BaseThread.h OSLinux.h rtpStreamTask.h TimedBufferListLinux.h LinuxTCP.hpp
OSINDEP_HDR = debug.h libcommon.h BaseLock.h BaseStream.h BufferList.h FileStream.h NetAddr.hpp BaseTCP.hpp rtp.h TimeSec.hpp
C_IN_DEP = $(filter %.c %.cc %.cpp %.cxx,$^)
EXEC = btree test1 testTH testBuffer testmmap
OBJ_FileDumpRead = FileDumpRead.o FileDump.o FileStream.o


btree: BTree.cpp
	$(CROSS_COMPILE)g++ -o $@ $^ $(CFLAGS) -Wno-deprecated

test1: test.cpp BufferList.cpp OSLinux.cpp LinuxTCP.cpp OSLinux.h $(OSINDEP_HDR) $(OSDEP_HDR)
	$(CROSS_COMPILE)g++ -o $@ $(C_IN_DEP) $(CFLAGS)


testTH: rtpStreamTask.cpp FileStream.cpp OSLinux.cpp rtp.cpp debug.h OSLinux.h BaseStream.h FileStream.h BaseThread.h testTH.cpp libcommon.h
	$(CROSS_COMPILE)g++ -o $@ $(C_IN_DEP) $(CFLAGS)

testBuffer: OSLinux.cpp BufferList.cpp TimedBufferListLinux.cpp debug.h OSLinux.h BaseThread.h testBuffer.cpp libcommon.h TimedBufferListLinux.h BufferList.h
	$(CROSS_COMPILE)g++ -o $@ $(C_IN_DEP) $(CFLAGS)

testmmap: testmmap.cpp
	$(CROSS_COMPILE)g++ -o $@ $(C_IN_DEP) $(CFLAGS)

FileDumpRead: ${OBJ_FileDumpRead}
	$(CROSS_COMPILE)g++ -o $@ $(O_IN_DEP) $(LDFLAGS)

$(call TARGET_OBJ,FileDumpRead)
	$(CROSS_COMPILE)g++ -c $(CFLAGS) -o $@ $(FIRST_C_IN_DEP)

$(call TARGET_OBJ,FileDump)
	$(CROSS_COMPILE)g++ -c $(CFLAGS) -o $@ $(FIRST_C_IN_DEP)

$(call TARGET_OBJ,FileStream)
	$(CROSS_COMPILE)g++ -c $(CFLAGS) -o $@ $(FIRST_C_IN_DEP)

clean:
	rm -f *.o $(EXEC)

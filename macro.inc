# use of module in libmanu is required to define
# 1. CROSS_COMPILE
# 2. LIBMANU : path of libmanu
# and they are expansion when evaluated
#

#
# retain basename of a list
# say, "test.o ../OSLinux.o" => "test.o OSLinux.o"
# usage: $(call BASE_IN_LIST,<list>)
# $1 : input list
#
define BASE_IN_LIST
$(foreach file,$1,$(shell basename $(file)))
endef

#
# retain basename of dependence
# usage: $(BASE_IN_DEP)
#
BASE_IN_DEP=$(call BASE_IN_LIST,$^)


#
# filter *.c *.cc *.cpp *.cxx in a list
# say, "abc.c abc.h" => "abc.c"
# usage: $(call C_IN_LIST,<list>)
# $1 : input list
#
define C_IN_LIST
$(filter %.c %.cc %.cpp %.cxx,$1)
endef

#
# filter *.o in a list
# say, "abc.o abc.h" => "abc.o"
# usage: $(call O_IN_LIST,<list>)
# $1 : input list
#
define O_IN_LIST
$(filter %.o,$1)
endef

#
# get all *.c *.cc *.cxx *.cpp in dependence
# usage: $(C_IN_DEP)
#
C_IN_DEP=$(call C_IN_LIST,$^)

#
# get all *.o in dependence
# usage: $(O_IN_DEP)
#
O_IN_DEP=$(call O_IN_LIST,$^)

#
# get first *.c *.cc *.cxx *.cpp in list
# usage: $(call FIRST_C_IN_LIST,<list>)
#
define FIRST_C_IN_LIST
$(firstword $(call C_IN_LIST,$1))
endef

#
# get first *.c *.cc *.cxx *.cpp in dependence
# usage: $(FIRST_C_IN_DEP)
#
FIRST_C_IN_DEP=$(call FIRST_C_IN_LIST,$^)

#
# get first *.o in list
# usage: $(call FIRST_O_IN_LIST,<list>)
#
define FIRST_O_IN_LIST
$(firstword $(call O_IN_LIST,$1))
endef

#
# get first *.o in dependence
# usage: $(FIRST_O_IN_DEP)
#
FIRST_O_IN_DEP=$(call FIRST_O_IN_LIST,$^)



#for xxx_DEP, the first must be the C file
Buffer_DEP = ${LIBBUFFER}/Buffer.cpp ${LIBBUFFER}/Buffer.h
BufferList_DEP = ${LIBBUFFER}/BufferList.cpp\
	${LIBMANU}/BaseLock.h ${LIBMANU}/libcommon.h\
	${LIBBUFFER}/Buffer.h ${LIBBUFFER}/BufferList.h
OSLinux_DEP = ${LIBMANU}/OSLinux.cpp ${LIBMANU}/OSLinux.h\
	${LIBMANU}/BaseLock.h ${LIBMANU}/libcommon.h\
	${LIBMANU}/BaseThread.h ${LIBMANU}/TimeSec.hpp
FileDump_DEP = ${LIBMANU}/FileDump.cpp ${LIBMANU}/FileDump.h\
	${FileStream_DEP}
FileStream_DEP = ${LIBMANU}/FileStream.cpp ${LIBMANU}/FileStream.h\
	${LIBMANU}/BaseStream.h ${LIBMANU}/libcommon.h
FileDumpRead_DEP = ${LIBMANU}/FileDumpRead.cpp ${FileDump_DEP}

#
# $1 - target w/o extention nor path. i.e. to generate in current path
# usage: $(call TARGET_OBJ,Buffer) ... no blank allowed
# which expands to
#     Buffer.o: $Buffer_DEP
define TARGET_OBJ
$1.o: $($1_DEP)
endef

#define TARGET_OBJ
#$1.o: $($1_DEP) ; $(CROSS_COMPILE)$(CXX) $(CFLAGS) -c -o $1.o $(call FIRST_C_IN_LIST,$($1_DEP))
#endef





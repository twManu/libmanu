INC_DIR = ../
SERVER_OBJ := server.o LinuxTCP.o
CLIENT_OBJ := client.o LinuxTCP.o

LinuxTCP_DEP := ../LinuxTCP.cpp ../LinuxTCP.hpp ../BaseTCP.hpp
APPS = server client

# Flags passed to the preprocessor.
CPPFLAGS += -DDEF_DEBUG -DIF_LINUX_TYPE_DEBUG -I$(INC_DIR)

# Flags passed to the C++ compiler.
#DEBUG and EXTRA_FLAG from makefile.inc
CXXFLAGS += -g -Wall -Wextra $(EXTRA_FLAG)


# House-keeping build targets.
.PHONY: all clean

all : $(APPS)

clean :
	@rm -f $(APPS) *.o
	
server : $(SERVER_OBJ) server.cpp $(LinuxTCP_DEP)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(SERVER_OBJ) -o $@ -lpthread

client : $(CLIENT_OBJ) client.cpp $(LinuxTCP_DEP)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CLIENT_OBJ) -o $@ -lpthread
	
LinuxTCP.o: $(LinuxTCP_DEP)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<
.PHONY: TARGET
TARGET: cntBuffer.o 
CPPFLAGS=-Wall -O2 -I ../

cntBuffer.o: cntBuffer.cpp cntBuffer.hpp ../BaseLock.h
	$(CROSS_COMPILE)gcc $(CPPFLAGS) -c -o $@ $<

clean:
	rm -rf *.o
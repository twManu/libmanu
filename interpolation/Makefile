.PHONY: TARGET
TARGET: interpolate.o

interpolate.o: interpolate.cpp interpolate.hpp
	$(CROSS_COMPILE)gcc -Wall -O2 -c -o $@ $<

clean:
	rm -rf *.o
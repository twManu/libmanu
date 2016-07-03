.PHONY: TARGET
TARGET: utest
CPPFLAGS = -I ../ -Wall -O2 

utest: utest.o
	$(CROSS_COMPILE)g++ -o $@ $^

utest.o: utest.cc ../interpolate.hpp
	$(CROSS_COMPILE)g++ $(CPPFLAGS) -c -o $@ $<

clean:
	rm -rf $(TARGET) *.o
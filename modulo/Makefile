.PHONY: all clean

TARGET=prog1 prog2
CFLAGS=-Wall -O2

all: $(TARGET)

prog1: prog1.c
	$(CROSS_COMPILE)gcc $(CFLAGS) -o $@ $<

prog2: prog2.c
	$(CROSS_COMPILE)gcc $(CFLAGS) -o $@ $<

clean:
	rm -rf *.o $(TARGET)

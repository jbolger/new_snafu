EXES = $(basename $(wildcard *.c))
CC = cc
CFLAGS = -o snafu -std=c99 -Wall -g `pkg-config --cflags --libs gtk+-2.0`

all: 
	$(MAKE) $(EXES)

%: %.c
	$(CC)  $@.c -o $@ $(CFLAGS)

clean:
	rm -f $(EXES) *.o

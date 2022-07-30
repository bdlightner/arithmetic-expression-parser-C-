# Makefile

CC=gcc
CCFLAGS=-g3 -Wall -O3

O_FILES = parser.o test.o 

all: test.c parser.c parser.h
	$(CC) $(CCFLAGS) -o parser test.c parser.c

clean:
	rm -f parser.exe
	rm -f parser
	rm -f *.o

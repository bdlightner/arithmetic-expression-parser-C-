CC=gcc
#
# optimise: -O3
#
##CCFLAGS=-g3 -Wall -pedantic -Wno-long-long -fmessage-length=0 -O3
CCFLAGS=-g3 -Wall -pedantic -O3

O_FILES = parser.o test.o 

test: $(O_FILES)
	@ echo "Linking parser...";
	@ $(CC) $(CCFLAGS) -o parser $(O_FILES)

clean:
	rm -f parser.exe
	rm -f parser
	rm -f *.o

o/%.o: %.c
	echo "  Compiling $@....";
	$(CC) -c $(CCFLAGS) $< -o $@

.c.o: parser.h
	$(CC) -c $(CCFLAGS) $<


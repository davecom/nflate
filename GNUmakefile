CC = gcc
FLAGS = -std=c11 -Wall -Werror -Wextra -Wpedantic -Wno-unused-variable
VPATH = src
OBJECTS = binarytree.o bitstream.o crc32.o gzipfile.o nflate.o main.o 

nflate: $(OBJECTS)
	$(CC) $(OBJECTS) -o nflate

release: FLAGS += -O3
release: nflate 

debug: FLAGS += -g
debug: nflate

binarytree.o: binarytree.c binarytree.h
	$(CC) $(FLAGS) -c src/binarytree.c

bitstream.o: bitstream.c bitstream.h
	$(CC) $(FLAGS) -c src/bitstream.c

crc32.o: crc32.c crc32.h
	$(CC) $(FLAGS) -c src/crc32.c

gzipfile.o: gzipfile.c gzipfile.h
	$(CC) $(FLAGS) -c src/gzipfile.c

nflate.o: nflate.c nflate.h bitstream.h binarytree.h
	$(CC) $(FLAGS) -c src/nflate.c

main.o: main.c crc32.h gzipfile.h nflate.h
	$(CC) $(FLAGS) -c src/main.c

clean:
	rm nflate *.o

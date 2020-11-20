CC = cl
FLAGS = /std:c17 /WX /EHsc
OBJECTS = binarytree.obj bitstream.obj crc32.obj gzipfile.obj nflate.obj main.obj

nflate: $(OBJECTS)
	$(CC) /Fe"nflate" $(OBJECTS)

release: FLAGS += /O2
release: nflate 

debug: FLAGS += /Zi
debug: nflate

binarytree.obj: src\binarytree.c src\binarytree.h
	$(CC) $(FLAGS) /c src\binarytree.c

bitstream.obj: src\bitstream.c src\bitstream.h
	$(CC) $(FLAGS) /c src\bitstream.c

crc32.obj: src\crc32.c src\crc32.h
	$(CC) $(FLAGS) /c src\crc32.c

gzipfile.obj: src\gzipfile.c src\gzipfile.h
	$(CC) $(FLAGS) /c src\gzipfile.c

nflate.obj: src\nflate.c src\nflate.h src\bitstream.h src\binarytree.h
	$(CC) $(FLAGS) /c src\nflate.c

main.obj: src\main.c src\crc32.h src\gzipfile.h src\nflate.h
	$(CC) $(FLAGS) /c src\main.c

clean:
	del nflate.exe *.obj

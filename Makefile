CC = cl
FLAGS = /std:c17 /WX /EHsc
OBJECTS = binarytree.obj bitstream.obj crc32.obj gzipfile.obj nflate.obj main.obj

nflate: $(OBJECTS)
	$(CC) /Fe"nflate" $(OBJECTS)

PPlot.obj: lib\PPlot.cpp lib\PPlot.h
	$(CC) $(FLAGS) /c lib\PPlot.cpp

SVGPainter.obj: lib\SVGPainter.cpp lib\SVGPainter.h
	$(CC) $(FLAGS) /c lib\SVGPainter.cpp

test.obj: src\test.cpp src\PriorityQueue.h src\timing.h
	$(CC) $(FLAGS) /I lib\ -c src\test.cpp

main.obj: src\main.cpp src\PriorityQueue.h src\timing.h
	$(CC) $(FLAGS) /I lib\ -c src\main.cpp

clean:
	del assignment12.exe *.obj *.svg

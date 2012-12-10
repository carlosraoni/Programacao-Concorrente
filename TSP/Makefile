CC := g++
CFLAGS := -O3

OBJECTS:= readtsplib.o

all: seq_bb_tsp

seq_bb_tsp: seq_bb_tsp.cpp $(OBJECTS)
	$(CC) -o seq_bb_tsp seq_bb_tsp.cpp $(OBJECTS) $(CFLAGS)

readtsplib.o: readtsplib.cpp readtsplib.h
	$(CC) -c readtsplib.cpp -o readtsplib.o $(CFLAGS)

clean :
	rm -rf $(OBJECTS)

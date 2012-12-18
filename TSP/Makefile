CC := g++
CFLAGS := -O3 -lpthread

OBJECTS:= readtsplib.o

all: seq_bb_tsp parallel_bb_tsp_1 parallel_bb_tsp_2

seq_bb_tsp: seq_bb_tsp.cpp $(OBJECTS)
	$(CC) -o seq_bb_tsp seq_bb_tsp.cpp $(OBJECTS) $(CFLAGS)

parallel_bb_tsp_1: parallel_bb_tsp_1.cpp $(OBJECTS)
	$(CC) -o parallel_bb_tsp_1 parallel_bb_tsp_1.cpp $(OBJECTS) $(CFLAGS)

parallel_bb_tsp_2: parallel_bb_tsp_2.cpp $(OBJECTS)
	$(CC) -o parallel_bb_tsp_2 parallel_bb_tsp_2.cpp $(OBJECTS) $(CFLAGS)

readtsplib.o: readtsplib.cpp readtsplib.h
	$(CC) -c readtsplib.cpp -o readtsplib.o $(CFLAGS)

clean :
	rm -rf $(OBJECTS)

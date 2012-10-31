CC := gcc
CFLAGS := -pthread -lm

OBJECTS:= shared_accumulator.o shared_task_queue.o adaptive_quadrature.o

all: solution1 solution2 solution3 solution4

solution1: concurrent_adaptive_quadrature_1.c $(OBJECTS)
	$(CC) -o solution1 concurrent_adaptive_quadrature_1.c $(OBJECTS) $(CFLAGS)

solution2: concurrent_adaptive_quadrature_2.c $(OBJECTS)
	$(CC) -o solution2 concurrent_adaptive_quadrature_2.c $(OBJECTS) $(CFLAGS)

solution3: concurrent_adaptive_quadrature_3.c $(OBJECTS)
	$(CC) -o solution3 concurrent_adaptive_quadrature_3.c $(OBJECTS) $(CFLAGS)

solution4: concurrent_adaptive_quadrature_4.c $(OBJECTS)
	$(CC) -o solution4 concurrent_adaptive_quadrature_4.c $(OBJECTS) $(CFLAGS)

shared_accumulator.o: shared_accumulator.c shared_accumulator.h
	$(CC) -c shared_accumulator.c -o shared_accumulator.o $(CFLAGS)

shared_task_queue.o: shared_task_queue.c shared_task_queue.h
	$(CC) -c shared_task_queue.c -o shared_task_queue.o $(CFLAGS)

adaptive_quadrature.o: adaptive_quadrature.c adaptive_quadrature.h
	$(CC) -c adaptive_quadrature.c -o adaptive_quadrature.o $(CFLAGS)

clean :
	rm -rf $(OBJECTS)

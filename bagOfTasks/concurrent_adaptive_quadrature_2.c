#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "shared_accumulator.h"
#include "shared_task_queue.h"
#include "adaptive_quadrature.h"

SharedTaskQueue taskQueue;
SharedAccumulator result;
pthread_t * workers;

int NWORKERS = 2;
int NTASKS = 1000;
double RANGE_INI = 0.0;
double RANGE_END = 15.0;
double (*f)(double);

#ifdef VERBOSE
pthread_mutex_t print_lock;
#endif

void * worker(void * arg){
	int threadId = *((int *) arg); // identificador da thread de envio
	double a, b, fa, fb, taskResult;
	double localAccumulator = 0.0;
	Task currentTask;

	while(dequeueToSharedTaskQueue(&taskQueue, &currentTask)){
		a = currentTask.a;
		b = currentTask.b;

#ifdef VERBOSE
		pthread_mutex_lock(&print_lock);
		printf("Thread %d starting to solve task [%.2f, %.2f]\n", threadId, a, b);
		pthread_mutex_unlock(&print_lock);
#endif

		fa = f(a);
		fb = f(b);

		taskResult = adaptiveQuadrature(a, b, fa, fb, f);

#ifdef VERBOSE
		pthread_mutex_lock(&print_lock);
		printf("Thread %d result to task [%.2f, %.2f] = %f\n", threadId, a, b, taskResult);
		printf("Pending Tasks: %d\n", getSharedTaskQueueSize(&taskQueue));
		pthread_mutex_unlock(&print_lock);
#endif

		localAccumulator += taskResult;
	}

#ifdef VERBOSE
	pthread_mutex_lock(&print_lock);
	printf("Thread %d finished tasks = %f\n", threadId, localAccumulator);
	pthread_mutex_unlock(&print_lock);
#endif

	addToSharedAccumulator(&result, localAccumulator);

	return NULL;
}

void createTasks(){
	int i;
	double slice = (RANGE_END - RANGE_INI) / NTASKS;
	double a, b;

	for(i = 0; i < NTASKS; i++){
		a = RANGE_INI + i * slice;
		b = a + slice;

		enqueueToSharedTaskQueue(&taskQueue, a, b);
	}
}

int main(int argc, char ** argv){
	clock_t begin, end;
	double timeSpent;
	int i;
	int * ids; // Ids das threads

	// Determina os parâmetros de execução
    if(argc != 5){
        printf("Usage: %s NWORKERS NTASKS RANGE_INI RANGE_END\n\n", argv[0]);
        printf("Using default values: \n");
    }
    else{
        NWORKERS = atoi(argv[1]);
        NTASKS = atoi(argv[2]);
        RANGE_INI = atof(argv[3]);
        RANGE_END = atof(argv[4]);
    }

    printf("NWORKERS = %d\n", NWORKERS);
    printf("NTASKS = %d\n", NTASKS);
    printf("RANGE_INI = %.2f\n", RANGE_INI);
    printf("RANGE_END = %.2f\n", RANGE_END);

	f = square;
	//f = fTest;

	if(!initSharedAccumulator(&result)){
		printf("Error initializing shared result!\n");
		return 1;
	}
	if(!initSharedTaskQueue(&taskQueue)){
		printf("Error initializing shared task queue!\n");
		return 1;
	}
#ifdef VERBOSE
	if (pthread_mutex_init(&print_lock, NULL) != 0){
		printf("Error initializing printing mutex!\n");
        return 1;
	}
#endif

	createTasks();

	workers = (pthread_t *) malloc(NWORKERS * sizeof(pthread_t));
	ids = (int *) malloc(NWORKERS * sizeof(int));

	begin = clock();

	for(i=0; i<NWORKERS; i++){
		ids[i] = i;
		if (pthread_create(&workers[i], NULL, worker, (void *) &ids[i])){
			printf("Error creating thread worker %d\n", i);
		}
	}

	// Aguardar fim de execução das threads workers
	for (i = 0; i < NWORKERS; i++){
		pthread_join(workers[i], NULL);
	}

	end = clock();
	timeSpent = (double) (end - begin) / CLOCKS_PER_SEC;

	printf("\n\nAdaptive Quadrature Result: %f\n\n", getSharedAccumulatorValue(&result));
	printf("Total Execution Time: %.3f (s)\n", timeSpent);

	free(workers);
	free(ids);
	destroySharedAccumulator(&result);
	destroySharedTaskQueue(&taskQueue);

	return 0;
}

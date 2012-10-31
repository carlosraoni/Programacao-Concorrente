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

SharedTaskQueue * taskQueues;
SharedAccumulator result;
pthread_t * workers;

int NWORKERS = 2;
int NTASKS = 1000;
double RANGE_INI = 0.0;
double RANGE_END = 15.0;
double (*f)(double);

pthread_mutex_t print_lock;

void * worker(void * arg){
	int threadId = *((int *) arg); // identificador da thread de envio
	double a, b, m, fa, fb, fm, area;
	double localAccumulator = 0.0;
	int i, pendingTaskFound = 1;
	Task currentTask;

	while(dequeueToSharedTaskQueue(&taskQueues[threadId], &currentTask)){
		a = currentTask.a;
		b = currentTask.b;

		pthread_mutex_lock(&print_lock);
		printf("Thread %d starting to solve task [%f, %f]\n", threadId, a, b);
		pthread_mutex_unlock(&print_lock);

		fa = f(a);
		fb = f(b);

		if(splitQuadratureTest(a, b, fa, fb, &m, &fm, &area, f)){
			enqueueToSharedTaskQueue(&taskQueues[threadId], a, m); // Coloca o primeiro trapezio da fila
			localAccumulator += adaptiveQuadrature(m, b, fm, fb, f); // Processa o segundo trapezio
		}
		else{
			localAccumulator += area;
		}

		pthread_mutex_lock(&print_lock);
		printf("Thread %d processed task [%f, %f]\n", threadId, a, b);
		pthread_mutex_unlock(&print_lock);
	}

	while(pendingTaskFound){
		pendingTaskFound = 0;
		for(i=0; i<NWORKERS; i++){
			if(i == threadId) continue;
			if(dequeueToSharedTaskQueue(&taskQueues[i], &currentTask)){
				pendingTaskFound = 1;
				break;
			}
		}
		if(pendingTaskFound){
			a = currentTask.a;
			b = currentTask.b;

			fa = f(a);
			fb = f(b);

			localAccumulator += adaptiveQuadrature(a, b, fa, fb, f);
		}
	}

	pthread_mutex_lock(&print_lock);
	printf("Thread %d finished tasks = %f\n", threadId, localAccumulator);
	pthread_mutex_unlock(&print_lock);

	addToSharedAccumulator(&result, localAccumulator);
}

int createTaskQueues(){
	int i;
	taskQueues = (SharedTaskQueue *) malloc(NWORKERS * sizeof(SharedTaskQueue));
	for(i =0; i < NWORKERS; i++){
		if(!initSharedTaskQueue(&taskQueues[i])){
			printf("Error initializing shared task queue of thread %d!\n", i);
			return 0;
		}
	}
	return 1;
}

void destroyTaskQueues(){
	int i;
	for(i = 0; i < NWORKERS; i++){
		destroySharedTaskQueue(&taskQueues[i]);
	}
	free(taskQueues);
}

void createTasks(){
	int i, currentWorker = 0;
	double slice = (RANGE_END - RANGE_INI) / NTASKS;

	for(i = 0; i < NTASKS; i++){
		double a = RANGE_INI + i * slice;
		double b = a + slice;

		enqueueToSharedTaskQueue(&taskQueues[currentWorker], a, b);
		currentWorker = (currentWorker + 1) % NWORKERS;
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
	if (pthread_mutex_init(&print_lock, NULL) != 0){
		printf("Error initializing printing mutex!\n");
        return 1;
	}
	if(!createTaskQueues()){
		printf("Error initializing task queues!\n");
        return 1;
	}
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
	destroyTaskQueues();

	return 0;
}

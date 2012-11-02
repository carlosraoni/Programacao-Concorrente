#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "shared_accumulator.h"
#include "adaptive_quadrature.h"

SharedAccumulator result;
pthread_t * workers;

int NWORKERS = 2;
double RANGE_INI = 0.0;
double RANGE_END = 15.0;
double (*f)(double);

#ifdef VERBOSE
pthread_mutex_t print_lock;
#endif

void * worker(void * arg){
	int threadId = *((int *) arg); // identificador da thread de envio

	double slice = (RANGE_END - RANGE_INI) / NWORKERS;
	double a = RANGE_INI + threadId * slice;
	double b = a + slice;
	double fa, fb, localResult;

#ifdef VERBOSE
	pthread_mutex_lock(&print_lock);
	printf("Thread %d starting to solve range [%.2f, %.2f]\n", threadId, a, b);
	pthread_mutex_unlock(&print_lock);
#endif

	fa = f(a);
	fb = f(b);

	localResult = adaptiveQuadrature(a, b, fa, fb, f);
	addToSharedAccumulator(&result, localResult);

#ifdef VERBOSE
	pthread_mutex_lock(&print_lock);
	printf("Thread %d result to range [%.2f, %.2f] = %f\n", threadId, a, b, localResult);
	pthread_mutex_unlock(&print_lock);
#endif

	return NULL;
}

int main(int argc, char ** argv){
	clock_t begin, end;
	double timeSpent;
	int i;
	int * ids; // Ids das threads

	// Determina os parâmetros de execução
    if(argc != 4){
        printf("Usage: %s NWORKERS RANGE_INI RANGE_END\n\n", argv[0]);
        printf("Using default values: \n");
    }
    else{
        NWORKERS = atoi(argv[1]);
        RANGE_INI = atof(argv[2]);
        RANGE_END = atof(argv[3]);
    }

    printf("NWORKERS = %d\n", NWORKERS);
    printf("RANGE_INI = %.2f\n", RANGE_INI);
    printf("RANGE_END = %.2f\n", RANGE_END);

	f = square;
	//f = foo;

	if(!initSharedAccumulator(&result)){
		printf("Error initializing shared result!\n");
		return 1;
	}
#ifdef VERBOSE
	if (pthread_mutex_init(&print_lock, NULL) != 0){
		printf("Error initializing printing mutex!\n");
        return 1;
	}
#endif

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

	return 0;
}

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define TOLERANCE 1e-16

typedef struct  {
	double value;
	pthread_mutex_t lock;
} SharedAccumulator;

int initSharedAccumulator(SharedAccumulator * acc){
	acc->value = 0.0;
	if (pthread_mutex_init(&acc->lock, NULL) != 0)
        return 0;
    return 1;
}

double getSharedAccumulatorValue(SharedAccumulator * acc){
	double value;

	pthread_mutex_lock(&acc->lock);
	value = acc->value;
	pthread_mutex_unlock(&acc->lock);

	return value;
}

void addToSharedAccumulator(SharedAccumulator * acc, double addValue){
	pthread_mutex_lock(&acc->lock);
	acc->value += addValue;
	pthread_mutex_unlock(&acc->lock);
}

int destroySharedAccumulator(SharedAccumulator * acc){
	pthread_mutex_destroy(&acc->lock);
}

SharedAccumulator result;
pthread_t * workers;

int NWORKERS = 2;
double RANGE_INI = 0.0;
double RANGE_END = 15.0;
double (*f)(double);

pthread_mutex_t print_lock;

double fTest(double num) {
	int j;
	double acc = 0.0;

  	for (j=0;j<10000000*num;j++)
    	acc += exp(sqrt(num*j*sin(num)))/log(j)*sqrt(j/25)*exp((num*j)*exp(exp(1.0/(j+1.0))));

	return acc;
}

double square (double num) {
	return num * num;
}

inline double calcTrapezoidArea(double a, double b, double fa, double fb){
	return (b - a) * (fa + fb) / 2.0;
}

double adaptiveQuadrature(double a, double b, double fa, double fb){
	double m = (a + b) / 2.0;
	double fm = f(m);

	double larea = calcTrapezoidArea(a, m, fa, fm);
	double rarea = calcTrapezoidArea(m, b, fm, fb);
	double area = calcTrapezoidArea(a, b, fa, fb);

	if(fabs(area - (larea + rarea)) >= TOLERANCE){
		return adaptiveQuadrature(a, m, fa, fm) + adaptiveQuadrature(m, b, fm, fb);
	}

	return area;
}

void * worker(void * arg){
	int threadId = *((int *) arg); // identificador da thread de envio

	double slice = (RANGE_END - RANGE_INI) / NWORKERS;
	double a = RANGE_INI + threadId * slice;
	double b = a + slice;

	pthread_mutex_lock(&print_lock);
	printf("Thread %d starting to solve range [%.2f, %.2f]\n", threadId, a, b);
	pthread_mutex_unlock(&print_lock);

	double fa = f(a);
	double fb = f(b);

	double localResult = adaptiveQuadrature(a, b, fa, fb);
	addToSharedAccumulator(&result, localResult);

	pthread_mutex_lock(&print_lock);
	printf("Thread %d result to range [%.2f, %.2f] = %f\n", threadId, a, b, localResult);
	pthread_mutex_unlock(&print_lock);
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
	//f = fTest;

	if(!initSharedAccumulator(&result)){
		printf("Error initializing shared result!\n");
		return 1;
	}
	if (pthread_mutex_init(&print_lock, NULL) != 0){
		printf("Error initializing printing mutex!\n");
        return 1;
	}

	workers = (pthread_t *) malloc(NWORKERS * sizeof(pthread_t));
	ids = (int *) malloc(NWORKERS * sizeof(int));

	begin = clock();

	for(i=0; i<NWORKERS; i++){
		ids[i] = i;
		if (pthread_create(&workers[i], NULL, worker, (void *) &ids[i])){
			printf("Error creating thread worker %d", i);
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

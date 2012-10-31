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

typedef struct {
	double a, b;
} Task;

typedef struct Node {
	Task task;
	struct Node * prev;
	struct Node * next;
} TaskNode;

typedef struct {
	int size;
	TaskNode * head;
	TaskNode * tail;
	pthread_mutex_t lock;
} SharedTaskQueue;

int initSharedTaskQueue(SharedTaskQueue * queue){
	queue->size = 0;
	queue->head = (TaskNode *) malloc(sizeof(TaskNode));
	queue->tail = (TaskNode *) malloc(sizeof(TaskNode));

	queue->head->prev = NULL;
	queue->head->next = queue->tail;
	queue->tail->prev = queue->head;
	queue->tail->next = NULL;

	if (pthread_mutex_init(&queue->lock, NULL) != 0)
        return 0;

    return 1;
}

int getSharedTaskQueueSize(SharedTaskQueue * queue){
	int size;

	pthread_mutex_lock(&queue->lock);
	size = queue->size;
	pthread_mutex_unlock(&queue->lock);

	return size;
}

void enqueueToSharedTaskQueue(SharedTaskQueue * queue, double taskA, double taskB){
	TaskNode * taskNode = (TaskNode *) malloc(sizeof(TaskNode));
	Task newTask;

	newTask.a = taskA;
	newTask.b = taskB;
	taskNode->task = newTask;

	pthread_mutex_lock(&queue->lock);

	taskNode->next = queue->head->next;
	taskNode->prev = queue->head;
	queue->head->next = taskNode;
	taskNode->next->prev = taskNode;
	queue->size++;

	pthread_mutex_unlock(&queue->lock);
}

int dequeueToSharedTaskQueue(SharedTaskQueue * queue, Task * task){
	TaskNode * taskNode;

	pthread_mutex_lock(&queue->lock);

	if(queue->size == 0){
		pthread_mutex_unlock(&queue->lock);
		return 0;
	}

	taskNode = queue->tail->prev;
	*task = taskNode->task;

	taskNode->prev->next = queue->tail;
	queue->tail->prev = taskNode->prev;
	queue->size--;

	pthread_mutex_unlock(&queue->lock);

	free(taskNode);

	return 1;
}

int destroySharedTaskQueue(SharedTaskQueue * queue){
	free(queue->head);
	free(queue->tail);
	pthread_mutex_destroy(&queue->lock);
}

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

SharedTaskQueue taskQueue;
SharedAccumulator result;
pthread_t * workers;

int NWORKERS = 2;
int NTASKS = 1000;
double RANGE_INI = 0.0;
double RANGE_END = 15.0;
double (*f)(double);

pthread_mutex_t print_lock;

double fTest(double num) {
	int j;
	double acc = 0.0;

  	for (j=0;j<100000*num;j++)
    	acc += exp(sqrt(num*j)*sin(num))/log(j+2.0)*sqrt(j/25)*exp((num*j)*exp(exp(1.0/(j+1.0))));

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
	double localAccumulator = 0.0;
	Task currentTask;

	while(dequeueToSharedTaskQueue(&taskQueue, &currentTask)){
		double a = currentTask.a;
		double b = currentTask.b;

		pthread_mutex_lock(&print_lock);
		printf("Thread %d starting to solve task [%.2f, %.2f]\n", threadId, a, b);
		pthread_mutex_unlock(&print_lock);

		double fa = f(a);
		double fb = f(b);

		double taskResult = adaptiveQuadrature(a, b, fa, fb);

		pthread_mutex_lock(&print_lock);
		printf("Thread %d result to task [%.2f, %.2f] = %f\n", threadId, a, b, taskResult);
		printf("Pending Tasks: %d\n", getSharedTaskQueueSize(&taskQueue));
		pthread_mutex_unlock(&print_lock);

		localAccumulator += taskResult;
	}

	pthread_mutex_lock(&print_lock);
	printf("Thread %d finished tasks = %f\n", threadId, localAccumulator);
	pthread_mutex_unlock(&print_lock);

	addToSharedAccumulator(&result, localAccumulator);
}

void createTasks(){
	int i;
	double slice = (RANGE_END - RANGE_INI) / NTASKS;

	for(i = 0; i < NTASKS; i++){
		double a = RANGE_INI + i * slice;
		double b = a + slice;

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
	if (pthread_mutex_init(&print_lock, NULL) != 0){
		printf("Error initializing printing mutex!\n");
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
	destroySharedTaskQueue(&taskQueue);

	return 0;
}

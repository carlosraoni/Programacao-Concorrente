#ifndef SHARED_TASK_QUEUE_H
#define SHARED_TASK_QUEUE_H

#include <pthread.h>

typedef struct {
	double a, b, fa, fb;
	char fValuesAvailable;
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

int initSharedTaskQueue(SharedTaskQueue * queue);

int getSharedTaskQueueSize(SharedTaskQueue * queue);

void enqueueToSharedTaskQueue(SharedTaskQueue * queue, double taskA, double taskB);

void enqueueToSharedTaskQueueWithFValues(SharedTaskQueue * queue, double taskA, double taskB, double fa, double fb);

int dequeueToSharedTaskQueue(SharedTaskQueue * queue, Task * task);

void destroySharedTaskQueue(SharedTaskQueue * queue);

#endif // SHARED_TASK_QUEUE_H

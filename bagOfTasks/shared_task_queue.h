#ifndef SHARED_TASK_QUEUE_H
#define SHARED_TASK_QUEUE_H

#include <pthread.h>

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

int initSharedTaskQueue(SharedTaskQueue * queue);

int getSharedTaskQueueSize(SharedTaskQueue * queue);

void enqueueToSharedTaskQueue(SharedTaskQueue * queue, double taskA, double taskB);

int dequeueToSharedTaskQueue(SharedTaskQueue * queue, Task * task);

int destroySharedTaskQueue(SharedTaskQueue * queue);

#endif // SHARED_TASK_QUEUE_H

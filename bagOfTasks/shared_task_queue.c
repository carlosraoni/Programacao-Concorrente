#include <stdlib.h>
#include "shared_task_queue.h"

int initSharedTaskQueue(SharedTaskQueue * queue){
	if (pthread_mutex_init(&queue->lock, NULL) != 0)
        return 0;

	queue->size = 0;
	queue->head = (TaskNode *) malloc(sizeof(TaskNode));
	queue->tail = (TaskNode *) malloc(sizeof(TaskNode));

	queue->head->prev = NULL;
	queue->head->next = queue->tail;
	queue->tail->prev = queue->head;
	queue->tail->next = NULL;

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

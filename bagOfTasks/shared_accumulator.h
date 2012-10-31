#ifndef SHARED_ACCUMULATOR_H
#define SHARED_ACCUMULATOR_H

#include <pthread.h>

typedef struct  {
	double value;
	pthread_mutex_t lock;
} SharedAccumulator;

int initSharedAccumulator(SharedAccumulator * acc);

double getSharedAccumulatorValue(SharedAccumulator * acc);

void addToSharedAccumulator(SharedAccumulator * acc, double addValue);

int destroySharedAccumulator(SharedAccumulator * acc);

#endif // SHARED_ACCUMULATOR_H

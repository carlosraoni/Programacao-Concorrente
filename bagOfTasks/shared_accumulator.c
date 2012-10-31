#include "shared_accumulator.h"

int initSharedAccumulator(SharedAccumulator * acc){
	if (pthread_mutex_init(&acc->lock, NULL) != 0)
        return 0;
	acc->value = 0.0;
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


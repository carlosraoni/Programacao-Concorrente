#ifndef SHARED_ACCUMULATOR_H
#define SHARED_ACCUMULATOR_H

#include <pthread.h>

// Arquivo de cabeçalho das definições para implementação de uma váriável de acumulação compartilhada

// Estrutura de dados da variável de acumulação compartilhada
typedef struct  {
	double value; // Valor da variável
	pthread_mutex_t lock; // Lock para acesso a variável
} SharedAccumulator;

// Função para inicialização da váriavel de acumulação compartilhada
int initSharedAccumulator(SharedAccumulator * acc);

// Retorna o valor da variável de acumulação compartilhada
double getSharedAccumulatorValue(SharedAccumulator * acc);

// Adiciona o valor addValue a variável de acumulação compartilhada
void addToSharedAccumulator(SharedAccumulator * acc, double addValue);

// Destrói a variável de acumulação compartilhada
void destroySharedAccumulator(SharedAccumulator * acc);

#endif // SHARED_ACCUMULATOR_H

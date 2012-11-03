#include "shared_accumulator.h"

// Função para inicialização da váriavel de acumulação compartilhada
int initSharedAccumulator(SharedAccumulator * acc){
	// Tenta inicializar o lock da váriavel, caso não consiga retorna 0
	if (pthread_mutex_init(&acc->lock, NULL) != 0)
        return 0;
	acc->value = 0.0; // Inicializa valor com 0
    return 1;
}

// Retorna o valor da variável de acumulação compartilhada
double getSharedAccumulatorValue(SharedAccumulator * acc){
	double value;

	pthread_mutex_lock(&acc->lock); // Obtém lock para leitura do valor da variável
	value = acc->value; // Copia valor
	pthread_mutex_unlock(&acc->lock); // Libera o lock

	return value; // Retorna valor
}

// Adiciona o valor addValue a variável de acumulação compartilhada
void addToSharedAccumulator(SharedAccumulator * acc, double addValue){
	pthread_mutex_lock(&acc->lock); // Obtém lock para escrita na variável
	acc->value += addValue; // Atualiza valor da variável adicionando addValue
	pthread_mutex_unlock(&acc->lock); // Libera lock
}

// Destrói a variável de acumulação compartilhada
void destroySharedAccumulator(SharedAccumulator * acc){
	pthread_mutex_destroy(&acc->lock); // Destroy lock da variável
}


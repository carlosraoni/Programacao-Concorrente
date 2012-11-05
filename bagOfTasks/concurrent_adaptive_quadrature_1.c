#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "shared_accumulator.h"
#include "adaptive_quadrature.h"

// Variável global que armazenará o resultado do cálculo
SharedAccumulator result;
// Array das threads trabalhadoras utilizadas no cálculo
pthread_t * workers;

int NWORKERS = 2; // Número de threads trabalhadoras
double RANGE_INI = 0.0; // Início do intervalo para o qual se deseja calcular a integral da função
double RANGE_END = 15.0; // Fim do intervalo para o qual se deseja calcular a integral da função
double (*f)(double); // Ponteiro para a função para o qual se deseja calcular a integral da função

#ifdef VERBOSE
pthread_mutex_t print_lock; // Mutex utilizado para impressão de informações pelas threads trabalhadoras
#endif

// Função principal das threads trabalhadoras
void * worker(void * arg){
	int threadId = *((int *) arg); // Identificador da thread

	// Tamanho da fatia do intervalo para qual a thread ficará responsável pelo cálculo
	double slice = (RANGE_END - RANGE_INI) / NWORKERS;
	double a = RANGE_INI + threadId * slice; // Ínicio do intervalor para cálculo da integral
	double b = a + slice; // Fim do intervalo para cálculo da integral
	double fa, fb, localResult; // Valor da função nos limites do intervalo [a,b] e resultado local do cálculo da thread

#ifdef VERBOSE
	pthread_mutex_lock(&print_lock);
	printf("Thread %d starting to solve range [%.2f, %.2f]\n", threadId, a, b);
	pthread_mutex_unlock(&print_lock);
#endif

	// Calcula os valores da função nos limites do intervalo [a,b]
	fa = f(a);
	fb = f(b);

	// Calcula a integral no intervalo [a,b] utilizando o método de quadratura adaptativa
	localResult = adaptiveQuadrature(a, b, fa, fb, f);
	// Soma o resultado local da thread ao resultado global
	addToSharedAccumulator(&result, localResult);

#ifdef VERBOSE
	pthread_mutex_lock(&print_lock);
	printf("Thread %d result to range [%.2f, %.2f] = %f\n", threadId, a, b, localResult);
	pthread_mutex_unlock(&print_lock);
#endif

	return NULL;
}

int main(int argc, char ** argv){
	clock_t begin, end; // Clock de ínicio e fim para cronometrar o tempo gasto no cálculo
	double timeSpent; // Tempo total gasto no cálculo
	int * ids; // Ids das threads
	int i;

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

	//f = square; // Utilizar a função x^2 nos testes
	//f = foo;  // Utilizar a função da Noemi nos testes
	//f = fmb;
	f = fl;

	// Inicialização das variáveis globais
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

	// Alocar e criar as threads trabalhadoras
	workers = (pthread_t *) malloc(NWORKERS * sizeof(pthread_t));
	ids = (int *) malloc(NWORKERS * sizeof(int));

	begin = clock(); // Clock de início do método

	// Cria as threads trabalhadoras
	for(i=0; i<NWORKERS; i++){
		ids[i] = i;
		if (pthread_create(&workers[i], NULL, worker, (void *) &ids[i])){
			printf("Error creating thread worker %d\n", i);
		}
	}

	// Aguardar fim de execução das threads trabalhadoras
	for (i = 0; i < NWORKERS; i++){
		pthread_join(workers[i], NULL);
	}

	end = clock(); // Clock de fim do método
	timeSpent = (double) (end - begin) / CLOCKS_PER_SEC; // Determina o tempo total gasto

	// Exibir resultado final e tempo total gasto
	printf("\n\nAdaptive Quadrature Result: %f\n\n", getSharedAccumulatorValue(&result));
	printf("Total Execution Time: %.3f (s)\n", timeSpent);

	// Liberar recursos
	free(workers);
	free(ids);
	destroySharedAccumulator(&result);

	return 0;
}

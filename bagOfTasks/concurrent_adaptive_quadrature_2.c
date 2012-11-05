#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "shared_accumulator.h"
#include "shared_task_queue.h"
#include "adaptive_quadrature.h"

// Fila compartilhada global de tarefas
SharedTaskQueue taskQueue;
// Variável global que armazenará o resultado do cálculo
SharedAccumulator result;
// Array das threads trabalhadoras utilizadas no cálculo
pthread_t * workers;

int NWORKERS = 2; // Número de threads trabalhadoras
int NTASKS = 1000; // Número de tarefas iniciais a serem criadas
double RANGE_INI = 0.0; // Início do intervalo para o qual se deseja calcular a integral da função
double RANGE_END = 15.0; // Fim do intervalo para o qual se deseja calcular a integral da função
double (*f)(double); // Ponteiro para a função para o qual se deseja calcular a integral da função

#ifdef VERBOSE
pthread_mutex_t print_lock; // Mutex utilizado para impressão de informações pelas threads trabalhadoras
#endif

// Função principal das threads trabalhadoras
void * worker(void * arg){
	int threadId = *((int *) arg); // Identificador da thread
	double a, b, fa, fb; // Limites do intervalo [a,b] e valores da função nos mesmos fa e fb
	double taskResult, localAccumulator = 0.0; // Resultado do cálculo de uma tarefa e acumulador do somatório de todas as tarefas resolvidas pela thread
	Task currentTask; // Tarefa atual sendo realizada pela thread

	// Enquanto tiver tarefas na fila, obter e realizar tarefa
	while(dequeueToSharedTaskQueue(&taskQueue, &currentTask)){
		a = currentTask.a; // Início do intervalo da tarefa
		b = currentTask.b; // Fim do intervalo da tarefa

#ifdef VERBOSE
		pthread_mutex_lock(&print_lock);
		printf("Thread %d starting to solve task [%.2f, %.2f]\n", threadId, a, b);
		pthread_mutex_unlock(&print_lock);
#endif

		// Calcula os valores da função nos limites do intervalo [a,b]
		fa = f(a);
		fb = f(b);

		// Calcula a integral no intervalo [a,b] utilizando o método de quadratura adaptativa
		taskResult = adaptiveQuadrature(a, b, fa, fb, f);

#ifdef VERBOSE
		pthread_mutex_lock(&print_lock);
		printf("Thread %d result to task [%.2f, %.2f] = %f\n", threadId, a, b, taskResult);
		printf("Pending Tasks: %d\n", getSharedTaskQueueSize(&taskQueue));
		pthread_mutex_unlock(&print_lock);
#endif

		// Soma o cálculo da tarefa no acumulador local de resultados da thread
		localAccumulator += taskResult;
	}

#ifdef VERBOSE
	pthread_mutex_lock(&print_lock);
	printf("Thread %d finished tasks = %f\n", threadId, localAccumulator);
	pthread_mutex_unlock(&print_lock);
#endif

	// Soma o resultado das tarefas realizadas pela thread no resultado global
	addToSharedAccumulator(&result, localAccumulator);

	return NULL;
}

// Cria as tarefas iniciais
void createTasks(){
	// Tamanho da fatia do intervalo para cada tarefa inicial
	double slice = (RANGE_END - RANGE_INI) / NTASKS;
	double a, b; // Limites do intervalo da tarefa
	int i;

	// Cria as tarefas iniciais e insere as mesmas na fila global de tarefas
	for(i = 0; i < NTASKS; i++){
		a = RANGE_INI + i * slice;
		b = a + slice;

		enqueueToSharedTaskQueue(&taskQueue, a, b);
	}
}

int main(int argc, char ** argv){
	clock_t begin, end; // Clock de ínicio e fim para cronometrar o tempo gasto no cálculo
	double timeSpent; // Tempo total gasto no cálculo
	int * ids; // Ids das threads
	int i;

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

	//f = square; // Utilizar a função x^2 nos testes
	//f = foo;  // Utilizar a função da Noemi nos testes
	//f = fmb;
	f = fl;

	// Inicialização das variáveis globais
	if(!initSharedAccumulator(&result)){
		printf("Error initializing shared result!\n");
		return 1;
	}
	if(!initSharedTaskQueue(&taskQueue)){
		printf("Error initializing shared task queue!\n");
		return 1;
	}
#ifdef VERBOSE
	if (pthread_mutex_init(&print_lock, NULL) != 0){
		printf("Error initializing printing mutex!\n");
        return 1;
	}
#endif

	createTasks(); // Cria as tarefas iniciais

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
	destroySharedTaskQueue(&taskQueue);

	return 0;
}

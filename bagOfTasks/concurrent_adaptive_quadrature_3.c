#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <time.h>
#include "shared_accumulator.h"
#include "shared_task_queue.h"
#include "adaptive_quadrature.h"

const double EPS = 1e-9;

// Fila compartilhada global de tarefas
SharedTaskQueue taskQueue;
// Variável global que armazenará o resultado do cálculo
SharedAccumulator result;
// Variável global que armazenará a soma do tamanho dos intervalos já calculados pelas threads
SharedAccumulator calculatedRange;
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

// Testa se todo o intervalo [RANGE_INI, RANGE_END] já foi calculado pelas threads
int isRangeCompleteCalculated(){
	return fabs((RANGE_END - RANGE_INI) - getSharedAccumulatorValue(&calculatedRange)) < EPS;
}

// Função principal das threads trabalhadoras
void * worker(void * arg){
	int threadId = *((int *) arg); // Identificador da thread
	double a, b, fa, fb, area; // Limites do intervalo [a,b], valores da função nos mesmos fa e fb e área do seu trapézio
	double m, fm; // Ponto médio do intervalo [a,b] e valor da função f no mesmo
	double localAccumulator = 0.0; // Acumulador do somatório de todas as tarefas resolvidas pela thread
	Task currentTask; // Tarefa atual sendo realizada pela thread

	// Enquanto o intervalo não foi completamente calculado, tentar realizar uma tarefa da fila de tarefas
	while(!isRangeCompleteCalculated()){
		// Tenta obter uma tafera para realização
		if(!dequeueToSharedTaskQueue(&taskQueue, &currentTask)){
			// Caso o intervalo não tenha sido completamente calculado e a fila esteja vazia a thread cede sua fatia de tempo de execução
			sched_yield();
			continue;
		}
		a = currentTask.a; // Início do intervalo da tarefa
		b = currentTask.b; // Fim do intervalo da tarefa

#ifdef VERBOSE
		pthread_mutex_lock(&print_lock);
		printf("Thread %d starting to solve task [%f, %f]\n", threadId, a, b);
		pthread_mutex_unlock(&print_lock);
#endif

		// Detemina os valores da função nos limites do intervalo [a,b]
		// Obtém os valores diretamente da tarefa caso disponível ou então calcula os mesmos caso contrário
		fa = (currentTask.fValuesAvailable) ? currentTask.fa : f(a);
		fb = (currentTask.fValuesAvailable) ? currentTask.fb : f(b);

		// Testa se a área do trapézio [a,b] está dentro da tolerância com as áreas dos trapézios [a,m] e [m,b]
		if(splitQuadratureTest(a, b, fa, fb, &m, &fm, &area, f)){
			// Caso não esteja na tolerância é preciso refinar o calculo dos trapézios [a,m] e [m,b]
			enqueueToSharedTaskQueueWithFValues(&taskQueue, a, m, fa, fm); // Coloca o primeiro trapézio da fila
			localAccumulator += adaptiveQuadrature(m, b, fm, fb, f); // Calcula o segundo trapézio e soma na variável local de resultados
			addToSharedAccumulator(&calculatedRange, b - m); // Atualiza o intervalo atualmente calculado
		}
		else{
			// Caso esteja na tolerância apenas atualiza o resultado local da thread e o intervalo já calculado
			localAccumulator += area;
			addToSharedAccumulator(&calculatedRange, b - a);
		}

#ifdef VERBOSE
		pthread_mutex_lock(&print_lock);
		printf("Thread %d processed task [%f, %f]\n", threadId, a, b);
		printf("Pending Tasks: %d\n", getSharedTaskQueueSize(&taskQueue));
		pthread_mutex_unlock(&print_lock);
#endif

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
	f = fmb;

	// Inicialização das variáveis globais
	if(!initSharedAccumulator(&result)){
		printf("Error initializing shared result!\n");
		return 1;
	}
	if(!initSharedAccumulator(&calculatedRange)){
		printf("Error initializing shared calculated range!\n");
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
	destroySharedAccumulator(&calculatedRange);
	destroySharedTaskQueue(&taskQueue);

	return 0;
}

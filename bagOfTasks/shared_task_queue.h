#ifndef SHARED_TASK_QUEUE_H
#define SHARED_TASK_QUEUE_H

#include <pthread.h>

// Arquivo de cabeçalho das definições para implementação de uma fila compartilhada de tarefas do método de aproximação
// de integral chamado quadratura adapitativa.

// Estrutura que define uma tarefa, que representa o cálculo aproximado da integral em um determinado intervalo [a,b]
typedef struct {
	double a, b, fa, fb; // Limites do intervalo e valores da função nestes limites
	char fValuesAvailable; // Indica se fa e fb já foram calculados e portanto se estão disponíveis na tarefa.
} Task;

// Estrutura que define o nó de uma lista duplamente encadeada que será utilizada para implementação da fila de tarefas.
typedef struct Node {
	Task task; // Tarefa do nó da lista.
	struct Node * prev; // Nó anterior na lista
	struct Node * next; // Nó seguinte na lista
} TaskNode;

// Estrutura que define uma fila compartilhada de tarefas.
typedef struct {
	int size; // Tamanho da fila.
	TaskNode * head; // Nó cabeçalho da lista duplamente encadeada utilizada na implementação da fila.
	TaskNode * tail; // Nó cauda da lista duplamente encadeada utilizada na implementação da fila.
	pthread_mutex_t lock; // Lock para acesso a fila.
} SharedTaskQueue;

// Inicializa a fila de tarefas compartilhada.
int initSharedTaskQueue(SharedTaskQueue * queue);

// Retorna o tamanho da fila de tarefas compartilhada.
int getSharedTaskQueueSize(SharedTaskQueue * queue);

// Adiciona a tarefa do intervalo [a,b] na fila de tarefas compartilhada.
void enqueueToSharedTaskQueue(SharedTaskQueue * queue, double a, double b);

// Adiciona a tarefa do intervalo [a,b] na fila de tarefas compartilhada já com os valores da função cálculados
// para a (fa) e b (fb).
void enqueueToSharedTaskQueueWithFValues(SharedTaskQueue * queue, double a, double b, double fa, double fb);

// Tenta obter uma tarefa da fila de tarefas compartilhada, retornando a mesma no parâmetro task ou zero
// para o caso da fila estar vazia.
int dequeueToSharedTaskQueue(SharedTaskQueue * queue, Task * task);

// Destrói a fila de tarefas compartilhada.
void destroySharedTaskQueue(SharedTaskQueue * queue);

#endif // SHARED_TASK_QUEUE_H

#include <stdlib.h>
#include "shared_task_queue.h"

// Inicializa a fila de tarefas compartilhada.
int initSharedTaskQueue(SharedTaskQueue * queue){
	// Tenta inicializar o lock da fila, retornando zero caso não tenha sucesso
	if (pthread_mutex_init(&queue->lock, NULL) != 0)
        return 0;

	queue->size = 0; // Fila vazia iniciamente
	queue->head = (TaskNode *) malloc(sizeof(TaskNode)); // Aloca nó cabeçalho da lista duplamente encadeada utilizada na implementação da fila
	queue->tail = (TaskNode *) malloc(sizeof(TaskNode)); // Aloca nó cauda da lista duplamente encadeada utilizada na implementação da fila

	// Inicializa ponteiros da lista duplamente encadeada utilizada na implementação da fila.
	queue->head->prev = NULL;
	queue->head->next = queue->tail;
	queue->tail->prev = queue->head;
	queue->tail->next = NULL;

    return 1;
}

// Retorna o tamanho da fila de tarefas compartilhada.
int getSharedTaskQueueSize(SharedTaskQueue * queue){
	int size;

	pthread_mutex_lock(&queue->lock); // Obtém lock da fila para leitura do seu tamanho.
	size = queue->size; // Copia tamanho atual da fila.
	pthread_mutex_unlock(&queue->lock); // Libera lock da fila

	// Retorna tamanho atual
	return size;
}

// Insere um nó tarefa na lista compartilhada de tarefas
void enqueueTaskNode(SharedTaskQueue * queue, TaskNode * taskNode){
	pthread_mutex_lock(&queue->lock); // Obtém lock da fila para alteração da mesma.

	// Insere nó como próximo elemento do cabeçalho da fila, atualizando devidamente os ponteiros da lista duplamente encadeada.
	taskNode->next = queue->head->next;
	taskNode->prev = queue->head;
	queue->head->next = taskNode;
	taskNode->next->prev = taskNode;
	queue->size++; // Atualiza tamalho da fila.

	pthread_mutex_unlock(&queue->lock); // Libera lock da fila.
}

// Adiciona a tarefa do intervalo [a,b] na fila de tarefas compartilhada.
void enqueueToSharedTaskQueue(SharedTaskQueue * queue, double a, double b){
	TaskNode * taskNode = (TaskNode *) malloc(sizeof(TaskNode)); // Aloca nó tarefa
	Task newTask;

	// Atualiza os valores da tarefa.
	newTask.a = a;
	newTask.b = b;
	newTask.fValuesAvailable = 0; // Tarefa sem a disponibilidade dos valores da função nos limites do intervalo [a,b].
	taskNode->task = newTask; // Atribui a nova tarefa ao nó a ser inserido na lista.

	// Insere novo nó tarefa na lista
	enqueueTaskNode(queue, taskNode);
}

// Adiciona a tarefa do intervalo [a,b] na fila de tarefas compartilhada já com os valores da função cálculados para a (fa) e b (fb).
void enqueueToSharedTaskQueueWithFValues(SharedTaskQueue * queue, double a, double b, double fa, double fb){
	TaskNode * taskNode = (TaskNode *) malloc(sizeof(TaskNode)); // Aloca nó tarefa
	Task newTask;

	// Atualiza os valores da tarefa.
	newTask.a = a;
	newTask.b = b;
	newTask.fa = fa;
	newTask.fb = fb;
	newTask.fValuesAvailable = 1; // Indica a disponibilidade dos valores da função nos limites do intervalo [a,b].
	taskNode->task = newTask; // Atribui a nova tarefa ao nó a ser inserido na lista.

	// Insere novo nó tarefa na lista
	enqueueTaskNode(queue, taskNode);
}


// Tenta obter uma tarefa da fila de tarefas compartilhada
int dequeueToSharedTaskQueue(SharedTaskQueue * queue, Task * task){
	TaskNode * taskNode;

	pthread_mutex_lock(&queue->lock); // Obtém lock da fila para manipulação da mesma.
	// Caso a fila esteja vazia retorna 0, que indica insucesso na tentativa de obtenção de uma tarefa da fila.
	if(queue->size == 0){
		pthread_mutex_unlock(&queue->lock); // Libera lock antes de retornar insucesso
		return 0;
	}

	// Obtém nó anterior ao nó cauda da lista duplamente encadeada utilizada na implementação da fila.
	taskNode = queue->tail->prev;
	*task = taskNode->task; // Copia tafera do referido nó para o parâmetro de saída task.

	// Remove o referido nó da lista duplamente encadeada, atualizando devidamente os ponteiros.
	taskNode->prev->next = queue->tail;
	queue->tail->prev = taskNode->prev;
	queue->size--; // Diminui tamanho da fila

	pthread_mutex_unlock(&queue->lock); // Libera o lock da fila

	free(taskNode); // Libera memória do nó removido da lista duplamente encadeada.

	return 1;
}

// Destrói a fila de tarefas compartilhada.
void destroySharedTaskQueue(SharedTaskQueue * queue){
	free(queue->head); // Libera memória do nó cabeçalho.
	free(queue->tail); // Libera memória do nó cauda.
	pthread_mutex_destroy(&queue->lock); // Destrói lock da fila.
}

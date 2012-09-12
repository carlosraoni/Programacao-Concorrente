/*

Trabalho da disciplina INF2591 - Programação concorrente
Prof.: Noemi

Aluno: Carlos Raoni de Alencar Mendes

Broadcast via buffer circular - Solução usando notação await:

// Estrutura de dados
int BUFFER_SIZE; // Tamanho do buffer
int NTHREADS; // Número total de threads

Dados buffer[BUFFER_SIZE]; // Buffer para armazenamento das mensagens transmitidas
BufferStatus buffer_status[BUFFER_SIZE]; // Controla o estado (livre ou usado) de cada posição do buffer

int last_send_message; // Sequencial que indica a última mensagem que foi enviada
int next_to_write; // Indica a próxima posição do buffer que deve ser usada para escrita

int last_read[NTHREADS]; // Armazena para cada thread qual foi a última mensagem que a mesma leu

send(int threadId, Dados msg){
    <
        await(buffer_status[next_to_write] == FREE) // Aguarda a posição do buffer para escrita estar livre

        buffer[next_to_write] = msg; // Escreve mensagem no buffer
        buffer_status[next_to_write] = USED; // Marca o status da posição de escrita para usado
        last_send_message++; // Atualiza o identificador da última mensagem transmitida
        next_to_write = (next_to_write + 1) % BUFFER_SIZE; // Atualiza a posição para escrita
    >
}

receive(int threadId){
    <
        await(last_read[threadId] < last_send_message) // Aguarda até que existam mensagens que não foram lidas
    >

    int next_to_read = (last_read[id] + 1) % BUFFER_SIZE; // Determina a posição do buffer da próxima mensagem a ser lida
    LeDados(buffer[next_to_read]); // Lê a mensagem do buffer

    <
        last_read[threadId]++; // Atualiza última mensagem lida pela thread
        int read_message = last_read[threadId]; // Armazena identificador da mensagem que acabou de ser lida pela thread

        bool message_has_pending_reads = false; // Indica se a mensagem que acabou de ser lida possui threads que ainda não a leram
        for(int i=0; i<NTHREADS; i++){
            if(last_read[i] < read_message){ // Verfica se a thread de idenficador id ainda não leu a mensagem
                message_has_pending_reads = true;
                break;
            }
        }
        if(!message_has_pending_reads){ // Verifica se a mensagem que acabou de ser lida já foi lida por todas as threads
            buffer_status[next_to_read] = FREE; // Libera a posição do buffer que a mensagem já lida por todos ocupava
        }
    >
}

*/

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <pthread.h>
#include <semaphore.h>

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <map>

using namespace std;

const int BUFFER_SIZE = 5; // Tamanho do buffer
const int NTHREADS = 10; // Número total de threads

typedef enum {FREE, USED} BufferEntryStatus; // Enum de status de cada entrada do buffer

vector<string> buffer(BUFFER_SIZE); // Buffer para armazenamento das mensagens transmitidas
vector<BufferEntryStatus> buffer_status(BUFFER_SIZE, FREE); // Controla o estado (livre ou usado) de cada posição do buffer, sendo o valor inicial livre para todas as entradas

int last_send_message = -1; // Sequencial que indica a última mensagem que foi enviada
int next_to_write = 0; // Indica a próxima posição do buffer que deve ser usada para escrita
vector<int> last_read(NTHREADS, -1); // Armazena para cada thread qual foi a última mensagem que a mesma leu

// Variáveis utilizadas para implementação da passagem de bastão
int delayed_senders = 0; // Contador do número de threads suspensas ao tentar enviar mensagem
vector<int> delayed_receive(NTHREADS, 0); // Indicador para cada thread se a mesma está suspensa esperando a próxima mensagem para leitura

sem_t e; // Semáforo da exclusao mutua
sem_t sem_receive[NTHREADS]; // Semáforo de leitura para cada thread
sem_t sem_send; // Semáforo de escrita

// Primitiva send recebe o identificador da thread e a mensagem a ser enviada
void send(int id, const string & msg){

    // < await(buffer_status[next_to_write] == FREE) // Aguarda a posição do buffer para escrita estar livre
    sem_wait(&e); // P(e)
    if(buffer_status[next_to_write] == USED){
        delayed_senders++;
        sem_post(&e); // V(e)
        sem_wait(&sem_send); // P(sem_send)
    }

    cout << "buffer[" << next_to_write << "] = " << msg << endl;

    // Escrevendo no buffer e atualizando estruturas
    buffer[next_to_write] = msg; // Escreve mensagem no buffer
    buffer_status[next_to_write] = USED; // Marca o status da posição de escrita para usado
    last_send_message++; // Atualiza o identificador da última mensagem transmitida
    next_to_write = (next_to_write + 1) % BUFFER_SIZE; // Atualiza a posição para escrita

    //SIGNAL
    int delayed_receiver = -1; // Identificador de thread que estivesse suspensa aguardando nova mensagem a ser lida
    for(int i=0; i<NTHREADS; i++){
        if(delayed_receive[i] > 0){ // Verifica se a thread de id i está suspensa aguardando mensagem
            delayed_receiver = i;
            break;
        }
    }
    if(delayed_receiver >= 0){ // Se existia alguma thread suspensa esperando mensagem
        delayed_receive[delayed_receiver]--; // Decrementa contador de thread suspensa por leitura
        sem_post(&sem_receive[delayed_receiver]); // V(sem_receive[delayed_receiver])
    }
    else{
        sem_post(&e); // V(e)
    }
    //>
}

// Primitiva receive recebe o identificador da thread e um vetor dinâmico que armazena as mensagens recebidas sequencialmente
void receive(int id, vector<string> & received){

    sem_wait(&e);
    if(last_read[id] == last_send_message){
        delayed_receive[id]++;
        sem_post(&e);
        sem_wait(&sem_receive[id]);
    }
    // SIGNAL
    sem_post(&e);

    int next_to_read = (last_read[id] + 1) % BUFFER_SIZE;
    received.push_back(buffer[next_to_read]);

    sem_wait(&e);
    int read_message = ++last_read[id];

    bool message_has_pending_reads = false;
    for(int i=0; i<NTHREADS; i++){
        if(last_read[i] < read_message){
            message_has_pending_reads = true;
            break;
        }
    }
    if(!message_has_pending_reads){
        buffer_status[next_to_read] = FREE;
    }

    // SIGNAL
    int delayed_receiver = -1;
    for(int i=0; i<NTHREADS; i++){
        if(delayed_receive[i] > 0 && last_read[i] < last_send_message){
            delayed_receiver = i;
            break;
        }
    }
    if(delayed_receiver >= 0){
        delayed_receive[delayed_receiver]--;
        sem_post(&sem_receive[delayed_receiver]);
    }
    else if(delayed_senders > 0 && buffer_status[next_to_read] == FREE){
        delayed_senders--;
        sem_post(&sem_send);
    }
    else{
        sem_post(&e);
    }
}

void * thread_main(void * arg){
	int threadId = *((int *) arg);
	vector<string> received;

    ostringstream out;
    out << "[" << threadId << "]";
    send(threadId, out.str());

    while(last_read[threadId] < NTHREADS){
        receive(threadId, received);
    }

    sem_wait(&e);
    cout << "Rec. Msgs from thread " << threadId << ": ";
    for(int i=0; i<received.size(); i++){
        cout << received[i];
    }
    cout << endl;
    sem_post(&e);
}

int main(int argc, char ** argv){

	pthread_t t[NTHREADS];

	sem_init(&e, 0, 1);
	sem_init(&sem_send, 0, 0);
	for(int i=0; i<NTHREADS; i++){
        sem_init(&sem_receive[i], 0, 0);
	}

	for(int i=0; i<NTHREADS; i++){
		if (pthread_create(&t[i], NULL, thread_main, (void *) new int(i))){
			cout << "Error creating thread " << i << "!" << endl;
		}
	}

	for (int i = 0; i < NTHREADS; i++){
		pthread_join(t[i], NULL);
	}

	return 0;
}

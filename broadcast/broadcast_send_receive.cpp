/*

Trabalho da disciplina INF2591 - Programação concorrente
Prof.: Noemi

Aluno: Carlos Raoni de Alencar Mendes

Broadcast via buffer circular - Solução usando notação await:

// Estrutura de dados
int BUFFER_SIZE; // Tamanho do buffer
int NSENDERS; // Número total de threads que enviam mensagens
int NRECEIVERS; // Número total de threads que recebem mensagens

Dados buffer[BUFFER_SIZE]; // Buffer para armazenamento das mensagens transmitidas
BufferStatus buffer_status[BUFFER_SIZE]; // Controla o estado (livre ou usado) de cada posição do buffer

int last_send_message; // Sequencial que indica a última mensagem que foi enviada
int next_to_write; // Indica a próxima posição do buffer que deve ser usada para escrita

int last_read[NRECEIVERS]; // Armazena para cada thread de recebimento qual foi a última mensagem que a mesma leu

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

    int next_to_read = (last_read[threadId] + 1) % BUFFER_SIZE; // Determina a posição do buffer da próxima mensagem a ser lida
    LeDados(buffer[next_to_read]); // Lê a mensagem do buffer

    <
        last_read[threadId]++; // Atualiza última mensagem lida pela thread
        int read_message = last_read[threadId]; // Armazena identificador da mensagem que acabou de ser lida pela thread

        bool message_has_pending_reads = false; // Indica se a mensagem que acabou de ser lida possui threads que ainda não a leram
        for(int i=0; i<NRECEIVERS; i++){
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
#include <ctime>
#include <pthread.h>
#include <semaphore.h>

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <map>

using namespace std;

int BUFFER_SIZE = 5; // Tamanho do buffer
int NSENDERS = 20; // Número total de threads que enviam mensagens
int NRECEIVERS = 10; // Número total de threads que recebem mensagens
int MESSAGES_PER_SENDER = 1; // Número de mensagens a serem enviadas por cada thread de envio

typedef enum {FREE, USED} BufferEntryStatus; // Enum de status de cada entrada do buffer

vector<string> buffer; // Buffer para armazenamento das mensagens transmitidas
vector<BufferEntryStatus> buffer_status; // Controla o estado (livre ou usado) de cada posição do buffer, sendo o valor inicial livre para todas as entradas

int last_send_message = -1; // Sequencial que indica a última mensagem que foi enviada
int next_to_write = 0; // Indica a próxima posição do buffer que deve ser usada para escrita
vector<int> last_read; // Armazena para cada thread de recebimento qual foi a última mensagem que a mesma leu

// Variáveis utilizadas para implementação da passagem de bastão
int delayed_senders = 0; // Contador do número de threads suspensas ao tentar enviar mensagem
vector<int> delayed_receive; // Indicador para cada thread de recebimento se a mesma está suspensa esperando a próxima mensagem para leitura

sem_t e; // Semáforo da exclusao mutua
vector<sem_t> sem_receive; // Semáforo de leitura para cada thread de recebimento
sem_t sem_send; // Semáforo para threads de escrita

// Primitiva send recebe o identificador da thread e a mensagem a ser enviada
void send(int id, const string & msg){

    // < await(buffer_status[next_to_write] == FREE) // Aguarda a posição do buffer para escrita estar livre
    sem_wait(&e); // P(e)
    if(buffer_status[next_to_write] == USED){
        delayed_senders++;
        sem_post(&e); // V(e)
        sem_wait(&sem_send); // P(sem_send)
    }

    cout << "Thread sender " << id << " sending message: " << msg << endl;

    // Escrevendo no buffer e atualizando estruturas
    buffer[next_to_write] = msg; // Escreve mensagem no buffer
    buffer_status[next_to_write] = USED; // Marca o status da posição de escrita para usado
    last_send_message++; // Atualiza o identificador da última mensagem transmitida
    next_to_write = (next_to_write + 1) % BUFFER_SIZE; // Atualiza a posição para escrita

    //SIGNAL
    int delayed_receiver = -1; // Identificador de thread que estivesse suspensa aguardando nova mensagem a ser lida
    for(int i=0; i<NRECEIVERS; i++){
        if(delayed_receive[i] > 0){ // Verifica se a thread de id i está suspensa aguardando mensagem
            delayed_receiver = i;
            break;
        }
    }
    if(delayed_receiver >= 0){ // Se existia alguma thread suspensa esperando mensagem
        // Acorda a thread recebimento
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

    // <await(last_read[threadId] < last_send_message) // Aguarda até que existam mensagens que não foram lidas>
    sem_wait(&e); // P(e)
    if(last_read[id] == last_send_message){
        delayed_receive[id]++;
        sem_post(&e); // V(e)
        sem_wait(&sem_receive[id]); // P(sem_receive[id])
    }
    // SIGNAL
    sem_post(&e); // V(e)

    int next_to_read = (last_read[id] + 1) % BUFFER_SIZE; // Determina a posição do buffer da próxima mensagem a ser lida
    received.push_back(buffer[next_to_read]); // Armazena no final do vetor dinâmico a mensagem recebida

    // <
    sem_wait(&e); // P(e)
    int read_message = ++last_read[id]; // Atualiza última mensagem lida pela thread e atribui a read_message

    bool message_has_pending_reads = false; // Indica se a mensagem que acabou de ser lida possui threads que ainda não a leram
    for(int i=0; i<NRECEIVERS; i++){
        if(last_read[i] < read_message){ // Verfica se a thread de idenficador id ainda não leu a mensagem
            message_has_pending_reads = true;
            break;
        }
    }
    if(!message_has_pending_reads){ // Verifica se a mensagem que acabou de ser lida já foi lida por todas as threads
        buffer_status[next_to_read] = FREE; // Libera a posição do buffer que a mensagem já lida por todos ocupava
    }

    // SIGNAL
    int delayed_receiver = -1; // Identificador de thread que estivesse suspensa aguardando nova mensagem a ser lida
    for(int i=0; i<NRECEIVERS; i++){
        if(delayed_receive[i] > 0 && last_read[i] < last_send_message){ // Verifica se a thread de id i está suspensa e possui mensagem nova para a mesma ler
            delayed_receiver = i;
            break;
        }
    }
    if(delayed_receiver >= 0){ // Se existia alguma thread suspensa com mensagem pendente para recebimento
        // Acorda a thread de recebimento
        delayed_receive[delayed_receiver]--; // Decrementa contador de thread suspensa por recebimento
        sem_post(&sem_receive[delayed_receiver]); // V(sem_receive[delayed_receiver])
    }
    else if(delayed_senders > 0 && buffer_status[next_to_read] == FREE){ // Verifica se existe thread de escrita suspensa e o buffer acabou de liberar uma posição para escrita
        // Acorda thread de envio
        delayed_senders--; // Decrementa contador de thread suspensa por envio
        sem_post(&sem_send); // V(sem_send)
    }
    else{
        sem_post(&e); // V(e)
    }
    // >
}

// Gera uma mensagem aleatória
string generate_random_message(int id){
    // Gera um caracter aleatório
    int range = 'A' - 'Z' - 1;
    char randomChar = rand() % range + 'A';

    ostringstream out;
    out << "[" << randomChar << id << "]";

    return out.str();
}

// Loop principal das threads de envio
void * sender(void * arg){
	int threadId = *((int *) arg); // identificador da thread de envio

    // Envia MESSAGES_PER_SENDER mensagens aleatórias
    for(int i=0; i<MESSAGES_PER_SENDER; i++){
        string msg = generate_random_message(threadId);
        send(threadId, msg);
    }
}

// Loop principal das threads de recebimento
void * receiver(void * arg){
	int threadId = *((int *) arg); // identificador da thread de recebimento
	vector<string> received; // vetor dinâmico para armazenamento das mensagens recebidas

    // Enquanto não recebeu todas as mensagens a serem enviadas pelas threads de envio
    while(last_read[threadId] < (NSENDERS * MESSAGES_PER_SENDER) - 1){
        // Recebe mensagem
        receive(threadId, received);
    }

    // Reporta as mensagens recebidas em sequencia pela respectiva thread de recebimento
    sem_wait(&e);
    cout << "Rec. Msgs from thread receiver " << threadId << ": ";
    for(int i=0; i<received.size(); i++){
        cout << received[i];
    }
    cout << endl;
    sem_post(&e);
}



int main(int argc, char ** argv){
    srand ( time(NULL) ); // Inicializa semente de geração de números aleatórios

    // Determina os parâmetros de execução
    if(argc != 5){
        cout << "Usage: " << argv[0] << " NSENDERS NRECEIVERS BUFFER_SIZE MESSAGES_PER_SENDER" << endl << endl;
        cout << "Using default values: " << endl;
    }
    else{
        NSENDERS = atoi(argv[1]);
        NRECEIVERS = atoi(argv[2]);
        BUFFER_SIZE = atoi(argv[3]);
        MESSAGES_PER_SENDER = atoi(argv[4]);
    }

    cout << "NSENDERS = " << NSENDERS << endl;
    cout << "NRECEIVERS = " << NRECEIVERS << endl;
    cout << "BUFFER_SIZE = " << BUFFER_SIZE << endl;
    cout << "MESSAGES_PER_SENDER = " << MESSAGES_PER_SENDER << endl << endl;

    // Inicializa estruturas de dados
    vector<string> buffer_ini(BUFFER_SIZE);
    buffer = buffer_ini;

    vector<BufferEntryStatus> buffer_status_ini(BUFFER_SIZE, FREE);
    buffer_status = buffer_status_ini;

    vector<int> last_read_ini(NRECEIVERS, -1);
    last_read = last_read_ini;

    vector<int> delayed_receive_ini(NRECEIVERS, 0);
    delayed_receive = delayed_receive_ini;

    vector<sem_t> sem_receive_ini(NRECEIVERS);
    sem_receive = sem_receive_ini;

	pthread_t senders[NSENDERS]; // Array de threads de envio
	pthread_t receivers[NRECEIVERS]; // Array de threads de recebimento

	sem_init(&e, 0, 1); // Inicializa semáforo da exclusão mútua
	sem_init(&sem_send, 0, 0); // Inicializa semáforo das threads de envio
	for(int i=0; i<NRECEIVERS; i++){
        sem_init(&sem_receive[i], 0, 0); // Inicializa semáforo da threa de leitura i
	}

    // Cria threads de envio
	for(int i=0; i<NSENDERS; i++){
		if (pthread_create(&senders[i], NULL, sender, (void *) new int(i))){
			cout << "Error creating thread sender " << i << "!" << endl;
		}
	}

    // Cria threads de recebimento
	for(int i=0; i<NRECEIVERS; i++){
		if (pthread_create(&receivers[i], NULL, receiver, (void *) new int(i))){
			cout << "Error creating thread receiver " << i << "!" << endl;
		}
	}

    // Aguardar fim de execução das threads de envio
	for (int i = 0; i < NSENDERS; i++){
		pthread_join(senders[i], NULL);
	}

    // Aguardar fim de execução das threads de recebimento
	for (int i = 0; i < NRECEIVERS; i++){
		pthread_join(receivers[i], NULL);
	}

	return 0;
}

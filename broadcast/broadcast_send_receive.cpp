#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <pthread.h>
#include <semaphore.h>

#include <iostream>
#include <sstream>
#include <vector>
#include <string>

using namespace std;

const int BUFFER_SIZE = 10;
const int NUM_TOTAL_THREADS = 10;

typedef enum {FREE, USED} BufferEntryStatus;

vector<string> buffer(BUFFER_SIZE);
vector<BufferEntryStatus> buffer_status(BUFFER_SIZE, FREE);

vector<int> next_to_read(NUM_TOTAL_THREADS, 0);
vector<int> num_pending_reads(BUFFER_SIZE, 0);
int next_to_write = 0;

sem_t e; // semaforo da exclusao mutua
sem_t sem_senders[BUFFER_SIZE]; // semaforo para cada posicao do buffer
sem_t sem_receivers[BUFFER_SIZE];

vector<int> delayed_senders(BUFFER_SIZE, 0);
vector<int> delayed_receivers(BUFFER_SIZE, 0);

void send(int id){
//<await (status[proximoaserescrito] == livre)

    sem_wait(&e);
    if(buffer_status[next_to_write] == USED){
        delayed_senders[next_to_write]++;
        sem_post(&e);
        sem_wait(&sem_senders[next_to_write]);
    }

    //EscreveDados(proximoaserescrito, dados);
    ostringstream msg;
    //msg << "Thread " << id << " message at position " << next_to_write;
    msg << next_to_write;
    cout << "Thread " << id << " wrote the message: " << msg.str() << endl;
    buffer[next_to_write] = msg.str();

    //status[proximoaserescrito] = ocupado;
    buffer_status[next_to_write] = USED;
    //faltam[proximoaserescrito] = totalthreads;
    num_pending_reads[next_to_write] = NUM_TOTAL_THREADS;
    int writing_position = next_to_write;
    //proximoaserescrito = (proximoaserescrito + 1) mod tambuffer;
    next_to_write = (next_to_write + 1) % BUFFER_SIZE;

    //SIGNAL
    if(delayed_receivers[writing_position] > 0){
        delayed_receivers[writing_position]--;
        sem_post(&sem_receivers[writing_position]);
    }
    else{
        sem_post(&e);
    }
//>
}

void receive(int id, vector<string> & received){
    //< await (status[proximoaserlido[eu]] == ocupado)>
    sem_wait(&e);
    if(buffer_status[next_to_read[id]] == FREE){
        delayed_receivers[next_to_read[id]]++;
        sem_post(&e);
        sem_wait(&sem_receivers[next_to_read[id]]);
    }
    sem_post(&e);

    //meusdados = LeDados([proximoaserlido[eu]]);
    received.push_back(buffer[next_to_read[id]]);
    //cout << "Thread " << id << " read the message: " << buffer[next_to_read[id]] << endl;

    //<
    sem_wait(&e);
    //faltam[proximoaserlido[eu]]--;
    num_pending_reads[next_to_read[id]]--;
    //if (faltam[proximoaserlido[eu]] == 0)
        //status[proximoaserlido[eu]] = livre;
    if(num_pending_reads[next_to_read[id]] == 0)
        buffer_status[next_to_read[id]] = FREE;

    // SIGNAL
    if(delayed_senders[next_to_read[id]] > 0 && buffer_status[next_to_read[id]] == FREE){
        delayed_senders[next_to_read[id]]--;
        sem_post(&sem_senders[next_to_read[id]]);
    }
    else if(delayed_receivers[next_to_read[id]] > 0 && buffer_status[next_to_read[id]] == USED){
        delayed_receivers[next_to_read[id]]--;
        sem_post(&sem_receivers[next_to_read[id]]);
    }
    else{
        sem_post(&e);
    }
    //>

    //proximoaserlido[eu] = (proximoaserlido[eu] + 1 ) mod tambuffer;
    next_to_read[id] = (next_to_read[id] + 1) % BUFFER_SIZE;
}

void * action(void * arg){
	int threadId = *((int *) arg);
	vector<string> received;

    send(threadId);
    while(received.size() < NUM_TOTAL_THREADS){
        receive(threadId, received);
    }

    sem_wait(&e);
    cout << "Received Messages from thread " << threadId << ": " << endl;
    for(int i=0; i<received.size(); i++){
        cout << received[i];
    }
    cout << endl;
    sem_post(&e);
}

int main(int argc, char ** argv){

	pthread_t thread[NUM_TOTAL_THREADS];

	sem_init(&e, 0, 1);
	for(int i=0; i<BUFFER_SIZE; i++){
        sem_init(&sem_senders[i], 0, 0);
        sem_init(&sem_receivers[i], 0, 0);
	}

	for(int i=0; i<NUM_TOTAL_THREADS; i++){
		if (pthread_create(&thread[i], NULL, action, (void *) new int(i))){
			cout << "Error creating thread " << i << "!" << endl;
		}
	}

	for (int i = 0; i < NUM_TOTAL_THREADS; i++){
		pthread_join(thread[i], NULL);
	}

	return 0;
}

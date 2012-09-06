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

const int BUFFER_SIZE = 10;
const int NUM_TOTAL_THREADS = 10;

typedef enum {FREE, USED} BufferEntryStatus;

vector<string> buffer(BUFFER_SIZE);
vector<BufferEntryStatus> buffer_status(BUFFER_SIZE, FREE);
int total_free_entrys = BUFFER_SIZE;

int last_send_message = 0;
vector<int> last_read(NUM_TOTAL_THREADS, 0);
map<int, int> map_msg_buffer_index;

int delayed_senders = 0;
vector<int> delayed_receive(NUM_TOTAL_THREADS, 0);

sem_t e; // semaforo da exclusao mutua
sem_t sem_receive[NUM_TOTAL_THREADS];
sem_t sem_send;

void send(int id){

    sem_wait(&e);
    if(total_free_entrys == 0){
        delayed_senders++;
        sem_post(&e);
        sem_wait(&sem_send);
    }
    total_free_entrys--;
    // Procura uma entrada livre no buffer
    int writing_index = -1;
    for(int i=0; i<BUFFER_SIZE; i++){
        if(buffer_status[i] == FREE){
            writing_index = i;
            break;
        }
    }
    if(writing_index == -1){
        cout << "ERRO: Nao encontrou posicao livre no buffer!" << endl;
    }

    //EscreveDados(proximoaserescrito, dados);
    ostringstream msg;
    msg << "[" << id << ", " << writing_index << "]";
    cout << "Thread " << id << " wrote the message: " << msg.str() << endl;
    buffer[writing_index] = msg.str();
    map_msg_buffer_index[++last_send_message] = writing_index;
    buffer_status[writing_index] = USED;

    //SIGNAL
    int delayed_receiver = -1;
    for(int i=0; i<NUM_TOTAL_THREADS; i++){
        if(delayed_receive[i] > 0){
            delayed_receiver = i;
            break;
        }
    }
    if(delayed_receiver >= 0){
        delayed_receive[delayed_receiver]--;
        sem_post(&sem_receive[delayed_receiver]);
    }
    else{
        sem_post(&e);
    }
//>
}

void receive(int id, vector<string> & received){

    sem_wait(&e);
    if(last_read[id] == last_send_message){
        delayed_receive[id]++;
        sem_post(&e);
        sem_wait(&sem_receive[id]);
    }
    sem_post(&e);

    int next_to_read = last_read[id] + 1;
    received.push_back(buffer[map_msg_buffer_index[next_to_read]]);
    last_read[id] = next_to_read;

    sem_wait(&e);
    bool message_has_pending_reads = false;
    for(int i=0; i<NUM_TOTAL_THREADS; i++){
        if(last_read[i] < next_to_read){
            message_has_pending_reads = true;
            break;
        }
    }
    if(!message_has_pending_reads){
        buffer_status[map_msg_buffer_index[next_to_read]] = FREE;
        map_msg_buffer_index.erase(next_to_read);
        total_free_entrys++;
    }

    // SIGNAL
    int delayed_receiver = -1;
    for(int i=0; i<NUM_TOTAL_THREADS; i++){
        if(delayed_receive[i] > 0 && last_read[i] < last_send_message){
            delayed_receiver = i;
            break;
        }
    }
    if(delayed_receiver >= 0){
        delayed_receive[delayed_receiver]--;
        sem_post(&sem_receive[delayed_receiver]);
    }
    else if(delayed_senders > 0 && total_free_entrys > 0){
        delayed_senders--;
        sem_post(&sem_send);
    }
    else{
        sem_post(&e);
    }
}

void * action(void * arg){
	int threadId = *((int *) arg);
	vector<string> received;

    send(threadId);
    while(received.size() < NUM_TOTAL_THREADS){
        receive(threadId, received);
    }

    sem_wait(&e);
    cout << "Rec. Msgs from thread " << threadId << ": ";;
    for(int i=0; i<received.size(); i++){
        cout << received[i];
    }
    cout << endl;
    sem_post(&e);
}

int main(int argc, char ** argv){

	pthread_t thread[NUM_TOTAL_THREADS];

	sem_init(&e, 0, 1);
	sem_init(&sem_send, 0, 0);
	for(int i=0; i<NUM_TOTAL_THREADS; i++){
        sem_init(&sem_receive[i], 0, 0);
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

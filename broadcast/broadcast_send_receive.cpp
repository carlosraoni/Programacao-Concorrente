#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <pthread.h>
#include <semaphore.h>

#include <iostream>
#include <vector>
#include <string>

using namespace std;

const int BUFFER_SIZE = 10;
const int NUM_TOTAL_THREADS = 10;

typedef enum {FREE, USED} BufferEntryStatus;

vector<string> buffer(BUFFER_SIZE);
vector<BufferEntryStatus> buffer_usage(BUFFER_SIZE);

sem_t e;

void send(int id){	
	sem_wait(&e);	
	cout << "Sender number: " << id << endl;
	sem_post(&e);
}

void receive(int id){
	sem_wait(&e);
	cout << "Receiver number: " << id << endl;
	sem_post(&e);
}

void * action(void * arg){
	int threadId = *((int *) arg);
	if(threadId % 2 == 0){
		send(threadId);
	}
	else{
		receive(threadId);	
	}
}

int main(int argc, char ** argv){

	pthread_t thread[NUM_TOTAL_THREADS];
	
	sem_init(&e, 0, 1);

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

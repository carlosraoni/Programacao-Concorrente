#include <iostream>
#include <cstdio>
#include <ctime>
#include <vector>
#include <set>
#include <string>
#include <queue>
#include <cmath>
#include <algorithm>
#include <utility>
#include <pthread.h>
#include <semaphore.h>

#include "readtsplib.h"

using namespace std;

const int INF = 1000000;

clock_t begin, end, clockOfBest; // Clock de ínicio e fim para cronometrar o tempo gasto no cálculo
double timeSpent; // Tempo total gasto no cálculo

int n; // Número de cidades
int ** Dist; // Matriz de distâncias
double lowerBoundIni = INF;

int INITIAL_CITY = 0;
int INITIAL_TASKS_DEPTH = 1;
int NWORKERS = 2;

// Imprime a matriz de custos
void printCosts(){
	cout << "Number of citys: " << n << endl;
	for(int i=0; i < n; i++){
		for(int j=i+1; j<n; j++){
			cout << Dist(i, j) << " ";
		}
		cout << endl;
	}
}

// Classe que encapsula um tour no grafo
class Tour{
public:

	Tour() : visited(vector<bool>(n, false)), cost(0), sz(0){
	}

	// Adiciona uma cidade ao Tour
	void addCity(int c){
		int deltaCost = getCostOfAddCity(c);
		//int deltaLowerBound = getDeltaLowerBoundOfAddCity(c);

		tour.push_back(c);
		sz++;
		visited[c] = true;
		cost += deltaCost;
	}

	// Determina o custo de adicionar uma cidade
	int getCostOfAddCity(int c) const{
		if(sz == 0) return 0;

		int delta = Dist(tour[sz-1], c);
		if(sz == n - 1)
			delta += Dist(c, tour[0]);

		return delta;
	}

	// Remove a última cidade do tour
	void removeLastCity(){
		int c = tour[sz - 1];
		if(sz == n){
			cost -= Dist(c, tour[0]);
		}
		if(sz > 1){
			cost -= Dist(tour[sz - 2], c);
		}

		tour.pop_back();
		sz--;
		visited[c] = false;
	}

	// Retorna o custo atual do Tour
	int getCost() const{
		return cost;
	}

	// Retorna a cidade em um determinado índice
	int getCity(int index) const{
		return tour[index];
	}

	// Retorna o tamanho do tour
	int getSize() const{
		return sz;
	}

	// Determina se uma cidade já foi visitado no tour
	bool isVisited(int c) const{
		return visited[c];
	}

private:
	vector<int> tour; // Guarda sequência de visitas as cidades (tour)
	vector<bool> visited; // Armazena para cada cidade se a mesma já foi visitada
	int cost; // Custo do tour
	int sz; // Tamanho do tour
};

vector< Tour > tasks; // Tarefas iniciais
Tour best; // Melhor solução encontrada
pthread_mutex_t tasksLock;
pthread_mutex_t bestLock;

void initializeBestLock(){
	pthread_mutex_init(&bestLock, NULL);
}

void initializeTasksLock(){
	pthread_mutex_init(&tasksLock, NULL);
}

Tour getNewTask(bool & sucess){
	Tour t;
	sucess = false;

	pthread_mutex_lock(&tasksLock); // Obtém lock
	int sz = tasks.size();
	if(sz > 0){
		t = tasks.back();
		tasks.pop_back();
		sucess = true;
	}
	pthread_mutex_unlock(&tasksLock); // Libera o lock

	return t;
}

double getUpperBound(){
	//double value;

	//pthread_mutex_lock(&bestLock); // Obtém lock
	//value = best.getCost(); // Copia valor
	//pthread_mutex_unlock(&bestLock); // Libera o lock

	//return value; // Retorna valor
	return best.getCost();
}


void checkIfUpdateBestSolution(Tour & newBest){
	pthread_mutex_lock(&bestLock); // Obtém lock
	if(newBest.getCost() < best.getCost()){
	    clockOfBest = clock();
		timeSpent = (double) (clockOfBest - begin) / CLOCKS_PER_SEC; // Determina o tempo total gasto
		printf("Current Execution Time: %.3f (s)\n", timeSpent);
		cout << "Update Best: " << newBest.getCost() << endl;
		best = newBest;
	}
	pthread_mutex_unlock(&bestLock); // Libera o lock
}


bool operator<(const Tour & t1, const Tour & t2){
	return t1.getCost() < t2.getCost();
}

// Imprime um tour
void printTour(Tour & t){
	int sz = t.getSize();
	if(sz == 0) return;

	cout << "Tour: " << t.getCity(0);
	for(int i=1; i<sz; i++){
		cout << " -> " << t.getCity(i);
	}
	cout << " -> " << t.getCity(0) << endl;
	cout << "\tCost: " << t.getCost() << endl;

}

// Constrói um tour utilizando a estratégia gulosa partindo de uma cidade inicial
Tour buildGreedyTour(int ini){
	Tour greedyTour;
	vector<int> unVisited;

	greedyTour.addCity(ini);

	for(int i=0; i<n; i++){
		if(i == ini) continue;
		unVisited.push_back(i);
	}

	while(!unVisited.empty()){
		int minCost = INF, indexMin;
		int sz = unVisited.size();
		for(int i=0; i<sz; i++){
			int costOfAdd = greedyTour.getCostOfAddCity(unVisited[i]);
			if(costOfAdd < minCost){
				minCost = costOfAdd;
				indexMin = i;
			}
		}
		greedyTour.addCity(unVisited[indexMin]);
		swap(unVisited[indexMin], unVisited[sz - 1]);
		unVisited.pop_back();
	}

	return greedyTour;
}

// Determina o menor tour entre todos os tour's possíveis utilizando a estratégia gulosa
Tour getMinGreedyTour(){
	Tour best = buildGreedyTour(0);
	for(int i=1; i<n; i++){
		Tour t = buildGreedyTour(i);
		//cout << "==== Greedy "<< i << endl;
		//printTour(t);
		if(t.getCost() < best.getCost())
			best = t;
	}
	return best;
}

void updateMinTwoEdges(vector< vector<int> > & minTwoEdges, int src, int dest){
	int cost = Dist(src, dest);
	if(cost < minTwoEdges[src][1]){
		minTwoEdges[src][1] = cost;
		if(cost < minTwoEdges[src][0])
			swap(minTwoEdges[src][0], minTwoEdges[src][1]);
	}
	if(cost < minTwoEdges[dest][1]){
		minTwoEdges[dest][1] = cost;
		if(cost < minTwoEdges[dest][0])
			swap(minTwoEdges[dest][0], minTwoEdges[dest][1]);
	}
}

void calculateInitialLowerBound(){
	vector< vector<int> > minTwoEdges(n, vector<int>(2, INF));
	for(int i=0; i<n; i++){
		for(int j=i+1; j<n; j++){
			updateMinTwoEdges(minTwoEdges, i, j);
		}
	}

	int lb = 0;
	for(int i=0; i<n; i++){
		lb += minTwoEdges[i][0];
		lb += minTwoEdges[i][1];
	}

	lowerBoundIni = (double) lb / 2.0;
}

double calculateLowerBound(const Tour & t){
	int sz = t.getSize();

	if(sz <= 1){
		return (double) lowerBoundIni;
	}
	if(sz == n){
		return (double) t.getCost();
	}

	vector<int> unVis;
	for(int i=0; i<n; i++){
		if(!t.isVisited(i))
			unVis.push_back(i);
	}
	int first = t.getCity(0);
	int last = t.getCity(sz - 1);

	vector< vector<int> > minTwoEdges(n, vector<int>(2, INF));

	int szUnvis = unVis.size(), cost;
	for(int i=0; i<szUnvis; i++){
		int src = unVis[i];
		for(int j=i+1; j<szUnvis; j++){
			int dest = unVis[j];
			updateMinTwoEdges(minTwoEdges, src, dest);
		}
		updateMinTwoEdges(minTwoEdges, src, first);
		updateMinTwoEdges(minTwoEdges, src, last);
	}

	int lb = 2 * t.getCost();
	lb += minTwoEdges[first][0];
	lb += minTwoEdges[last][0];

	for(int i=0; i<szUnvis; i++){
		int v = unVis[i];
		lb += minTwoEdges[v][0];
		lb += minTwoEdges[v][1];
	}

	return (double) lb / 2.0;
}

void dfs(Tour & t){
	if(t.getSize() == n){
		checkIfUpdateBestSolution(t);
		return;
	}

	vector< pair<double, Tour> > candidates;
	for(int i=0; i<n; i++){
		if(t.isVisited(i))
			continue;

		Tour nextTour = t;
		nextTour.addCity(i);
		double lb = calculateLowerBound(nextTour);
		//cout << "lb: " << lb << " , cost: " << t.getCost() << endl;

		if(lb >= getUpperBound()){
			continue;
		}

		candidates.push_back(pair<double, Tour>(lb, nextTour));
	}
	sort(candidates.begin(), candidates.end());

	int nc = candidates.size();
	for(int i=0; i<nc; i++){
		if(candidates[i].first < getUpperBound()){
			dfs(candidates[i].second);
		}
	}
}

void createTasks(Tour & t){
	if(t.getSize() - 1 == INITIAL_TASKS_DEPTH){
		tasks.push_back(t);
		return;
	}
	for(int i=0; i<n; i++){
		if(!t.isVisited(i)){
			t.addCity(i);
			createTasks(t);
			t.removeLastCity();
		}
	}
}

void createInitialTasks(){
	Tour t;
	t.addCity(INITIAL_CITY);

	createTasks(t);

	sort(tasks.rbegin(), tasks.rend());
}

void * worker(void * arg){
	int threadId = *((int *) arg); // identificador da thread

	while(true){
		bool hasNewTask;
		Tour t = getNewTask(hasNewTask);
		if(!hasNewTask)
			break;
		double lb = calculateLowerBound(t);
		if(lb < getUpperBound()){
			dfs(t);
		}
	}
}

int main(int argc, char ** argv){

	if(argc < 2){
		cout << "Use: " << argv[0] << " 'tsp_input_file.tsp'" << endl;
		return 1;
	}

	if(!read(argv[1], Dist, n)){
		cout << "Error reading file: " << argv[1] << endl;
		return 1;
	}

	if(argc == 4){
		INITIAL_TASKS_DEPTH = atoi(argv[2]);
		NWORKERS = atoi(argv[3]);
	}

	cout << "Instance " << argv[1] << " read: " << n << endl;
	cout << "INITIAL_TASKS_DEPTH = " << INITIAL_TASKS_DEPTH << endl;
	cout << "NWORKERS = " << NWORKERS << endl;

	calculateInitialLowerBound();

	Tour minGreedy = getMinGreedyTour();
	cout << "------------ Best Greedy --------------" << endl;
	printTour(minGreedy);
	best = minGreedy;

	cout << "------------ Initializing Search --------------" << endl;
	begin = clock(); // Clock de início do método

	initializeTasksLock();
	initializeBestLock();
	createInitialTasks();

	pthread_t workers[NWORKERS];
	int ids[NWORKERS];

	// Cria as threads trabalhadoras
	for(int i=0; i<NWORKERS; i++){
		ids[i] = i;
		if (pthread_create(&workers[i], NULL, worker, (void *) &ids[i])){
			printf("Error creating thread worker %d\n", i);
		}
	}

	// Aguardar fim de execução das threads trabalhadoras
	for (int i = 0; i < NWORKERS; i++){
		pthread_join(workers[i], NULL);
	}

	end = clock(); // Clock de fim do método
	timeSpent = (double) (end - begin) / CLOCKS_PER_SEC; // Determina o tempo total gasto

	cout << "------------ Best Found --------------" << endl;
	printTour(best);

	printf("Total Execution Time: %.3f (s)\n\n", timeSpent);

    double btf = (double) (clockOfBest - begin) / CLOCKS_PER_SEC; // Determina o tempo total gasto
    printf("btf:%.3f\n", btf);
    printf("bsf:%d\n", best.getCost());
    printf("tet:%.3f\n", timeSpent);

	return 0;
}

#include <iostream>
#include <cstdio>
#include <ctime>
#include <vector>
#include <string>
#include <queue>
#include <cmath>
#include <algorithm>
#include <utility>
#include "readtsplib.h"

using namespace std;

const int INF = 100000000;

int n; // Número de cidades
int ** Dist; // Matriz de distâncias

// Classe que encapsula um tour no grafo
class Tour{
public:

	Tour() : visited(vector<bool>(n, false)), cost(0), sz(0){
	}

	// Adiciona uma cidade ao Tour
	void addCity(int c){
		int deltaCost = getCostOfAddCity(c);
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
		if(sz == n)
			cost -= Dist(c, tour[0]);
		if(sz > 1)
			cost -= Dist(tour[sz - 2], c);
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

// Imprime um tour
void printTour(const Tour & t){
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
	//cout << "==== Greedy 0" << endl;
	//printTour(best);
	for(int i=1; i<n; i++){
		Tour t = buildGreedyTour(i);
		//cout << "==== Greedy "<< i << endl;
		//printTour(t);
		if(t.getCost() < best.getCost())
			best = t;
	}
	return best;
}

vector<int> sumCostOfMinEdges;
int EDGE_PENALTY = INF;

void initializeSumCostOfMinEdges(){
	vector<int> e;
	for(int i=0; i<n; i++){
		for(int j=i+1; j<n; j++){
			e.push_back(Dist(i, j));
		}
	}
	sort(e.begin(), e.end());

	sumCostOfMinEdges.push_back(e[0]);
	for(int i=1; i<n; i++){
		sumCostOfMinEdges.push_back(e[i] + sumCostOfMinEdges[i-1]);
	}
	EDGE_PENALTY = e[e.size() - 1];

//	for(int i=0; i<n; i++){
//		cout << "sc[" << i << "] = " << sumCostOfMinEdges[i] << endl;
//	}
}

int getMinEdgesLowerBound(const Tour & t){
	int sz = t.getSize();
	if(sz == n)
		return 0;
	if(sz == 1)
		return sumCostOfMinEdges[n - 1];

	int numMissingEdges = n - (sz - 1);

	return t.getCost() + sumCostOfMinEdges[numMissingEdges - 1];
}

clock_t begin, end; // Clock de ínicio e fim para cronometrar o tempo gasto no cálculo
double timeSpent; // Tempo total gasto no cálculo

Tour best;

void dfs(Tour & t){
	if(t.getSize() == n){
		if(t.getCost() < best.getCost()){
			timeSpent = (double) (clock() - begin) / CLOCKS_PER_SEC; // Determina o tempo total gasto
			printf("Current Execution Time: %.3f (s)\n", timeSpent);
			cout << "Update Best: " << t.getCost() << endl;
			best = t;
		}
		return;
	}

	for(int i=0; i<n; i++){
		if(t.isVisited(i))
			continue;
		int deltaCost = t.getCostOfAddCity(i);
		if(t.getCost() + deltaCost >= best.getCost()){
			continue;
		}
		t.addCity(i);
		if(getMinEdgesLowerBound(t) < best.getCost()){
			dfs(t);
		}
		t.removeLastCity();
	}
}

void dfs(){
	Tour t;
	t.addCity(0);

	dfs(t);
}

bool operator<(const Tour & t1, const Tour & t2){
	//return t1.getCost() > t2.getCost();
	//return getMinEdgesLowerBound(t1) > getMinEdgesLowerBound(t2);
	//return getMinEdgesLowerBound(t1) + ((n - t1.getSize())*sumCostOfMinEdges[n-1]) > getMinEdgesLowerBound(t2) + ((n - t2.getSize())*sumCostOfMinEdges[n-1]);
	//return getMinEdgesLowerBound(t1) + ((n - t1.getSize())*EDGE_PENALTY) > getMinEdgesLowerBound(t2) + ((n - t2.getSize())*EDGE_PENALTY);
	return t1.getCost() + ((n - t1.getSize())*EDGE_PENALTY) > t2.getCost() + ((n - t2.getSize())*EDGE_PENALTY);
	//return t1.getCost() + ((n - t1.getSize())*INF) > t2.getCost() + ((n - t2.getSize())*INF);
}

void bbTsp(){
	priority_queue<Tour> q;
	Tour tIni;
	tIni.addCity(0);

	q.push(tIni);
	while(!q.empty()){
		Tour current = q.top();
		q.pop();
		//cout << "Curr " << current.getSize() << " = " << current.getCost() << endl;

		for(int i=0; i<n; i++){
			if(current.isVisited(i))
				continue;

			int deltaCost = current.getCostOfAddCity(i);
			if(current.getCost() + deltaCost >= best.getCost()){
				continue;
			}

			current.addCity(i);
			int lowerBound = getMinEdgesLowerBound(current);
			if(lowerBound < best.getCost()){
				if(current.getSize() == n){
					if(current.getCost() < best.getCost()){
						timeSpent = (double) (clock() - begin) / CLOCKS_PER_SEC; // Determina o tempo total gasto
						printf("Current Execution Time: %.3f (s)\n", timeSpent);
						cout << "Update Best: " << best.getCost() << " to " << current.getCost() << endl;
						best = current;
					}
				}
				else{
					q.push(current);
				}
			}
			current.removeLastCity();

		}

	}
}

int main(int argc, char ** argv){

	if(argc != 2){
		cout << "Use: " << argv[0] << " 'tsp_input_file.tsp'" << endl;
		return 1;
	}

	if(!read(argv[1], Dist, n)){
		cout << "Error reading file: " << argv[1] << endl;
		return 1;
	}

	cout << "Instance " << argv[1] << " read: " << n << endl;
	//printCosts();

	Tour minGreedy = getMinGreedyTour();

	cout << "------------ Best Greedy --------------" << endl;
	printTour(minGreedy);

	//best = minGreedy;

	initializeSumCostOfMinEdges();

	Tour ini;
	for(int i=0; i<n; i++)
		ini.addCity(i);
	best = ini;

	begin = clock(); // Clock de início do método

	//dfs();
	bbTsp();

	end = clock(); // Clock de fim do método
	timeSpent = (double) (end - begin) / CLOCKS_PER_SEC; // Determina o tempo total gasto

	cout << "------------ Best Found --------------" << endl;
	printTour(best);

	printf("Total Execution Time: %.3f (s)\n", timeSpent);

	return 0;
}

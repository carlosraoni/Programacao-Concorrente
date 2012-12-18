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
#include "readtsplib.h"

using namespace std;

const int INF = 1000000;

clock_t begin, end; // Clock de ínicio e fim para cronometrar o tempo gasto no cálculo
double timeSpent; // Tempo total gasto no cálculo

int n; // Número de cidades
int ** Dist; // Matriz de distâncias
double lowerBoundIni = INF;

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

Tour best;

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
		cout << "==== Greedy "<< i << endl;
		printTour(t);
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
		return (double) lowerBoundIni / 2.0;
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
		if(t.getCost() < best.getCost()){
			timeSpent = (double) (clock() - begin) / CLOCKS_PER_SEC; // Determina o tempo total gasto
			printf("Current Execution Time: %.3f (s)\n", timeSpent);
			cout << "Update Best: " << t.getCost() << endl;
			best = t;
		}
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

		if(lb >= (double) best.getCost()){
			continue;
		}

		candidates.push_back(pair<double, Tour>(lb, nextTour));
	}
	sort(candidates.begin(), candidates.end());

	int nc = candidates.size();
	for(int i=0; i<nc; i++){
		if(candidates[i].first < (double) best.getCost()){
			dfs(candidates[i].second);
		}
	}
}

void dfs(){
	Tour t;
	t.addCity(0);

	dfs(t);
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

	calculateInitialLowerBound();

	Tour minGreedy = getMinGreedyTour();
	cout << "------------ Best Greedy --------------" << endl;
	printTour(minGreedy);
	best = minGreedy;

	cout << "------------ Initializing Search --------------" << endl;

	begin = clock(); // Clock de início do método

	dfs();

	end = clock(); // Clock de fim do método
	timeSpent = (double) (end - begin) / CLOCKS_PER_SEC; // Determina o tempo total gasto

	cout << "------------ Best Found --------------" << endl;
	printTour(best);

	printf("Total Execution Time: %.3f (s)\n", timeSpent);

	return 0;
}

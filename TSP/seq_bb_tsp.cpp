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

int n; // Número de cidades
int ** Dist; // Matriz de distâncias
int lowerBoundIni = 0;

//vector< pair<int, int> > minEdges; // Guarda as duas menores arestas de cada vértice
vector< set<pair<int, int> > > minEdges; // Guarda as duas menores arestas de cada vértice
int EDGE_PENALTY = INF;

pair<int, int> getMinEdgesCosts(set< pair<int, int> > & e){
	set< pair<int, int> >::iterator it = e.begin();

	int c0 = (*it).first;
	it++;
	int c1 = (*it).first;

	return pair<int, int>(c0, c1);
}

int initializeMinEdges(){
	int initialLowerBound = 0;

	set< pair<int, int> > e;
	for(int i=0; i<n; i++){
		for(int j=0; j<n; j++){
			if(i == j) continue;
			e.insert(pair<int, int>(Dist(i, j), j));
		}
		pair<int, int> minCosts = getMinEdgesCosts(e);
		initialLowerBound += minCosts.first; initialLowerBound += minCosts.second;

		minEdges.push_back(e);
		cout << "Min[" << i << "] = " << minCosts.first << " - " << minCosts.second << endl;

		e.clear();
	}

	return initialLowerBound;
}



// Classe que encapsula um tour no grafo
class Tour{
public:

	Tour() : visited(vector<bool>(n, false)), cost(0), sz(0), lowerBound(lowerBoundIni), minEdgesTour(minEdges){
		for(int i=0; i<n; i++){
			pair<int, int> minCosts = getMinEdgesCosts(minEdgesTour[i]);
			lowerBoundV.push_back( minCosts.first + minCosts.second );
		}
	}

	// Adiciona uma cidade ao Tour
	void addCity(int c){
		int deltaCost = getCostOfAddCity(c);
		//int deltaLowerBound = getDeltaLowerBoundOfAddCity(c);

		tour.push_back(c);
		sz++;
		visited[c] = true;
		cost += deltaCost;

		if(sz > 1){
			updateLowerBoundLastAddedEdge();
		}
		if(sz == n){
			updateLowerBoundCloseCycle();
		}
		//lowerBound += deltaLowerBound;
	}

	// Determina o custo de adicionar uma cidade
	int getCostOfAddCity(int c) const{
		if(sz == 0) return 0;

		int delta = Dist(tour[sz-1], c);
		if(sz == n - 1)
			delta += Dist(c, tour[0]);

		return delta;
	}

	void updateLowerBoundLastAddedEdge(){
		int idxSource = sz - 2, idxDest = sz -1;
		int source = tour[idxSource];
		int dest = tour[idxDest];
		int cost = Dist(source, dest);

		lowerBound -= lowerBoundV[source];
		lowerBound -= lowerBoundV[dest];

		minEdgesTour[source].erase(pair<int, int>(cost, dest));
		minEdgesTour[dest].erase(pair<int, int>(cost, source));

		lowerBoundV[source] = cost;
		if(idxSource >= 1){
			lowerBoundV[source] += Dist(tour[idxSource - 1], source);
		}
		else{
			lowerBoundV[source] += (*minEdgesTour[source].begin()).first;
		}

		lowerBoundV[dest] = cost;
		lowerBoundV[dest] += (*minEdgesTour[dest].begin()).first;

		lowerBound += lowerBoundV[source] + lowerBoundV[dest];
	}

	void updateLowerBoundCloseCycle(){
		int first = tour[0];
		int last = tour[sz - 1];
		int cost = Dist(last, first);

		lowerBound -= lowerBoundV[last];
		lowerBound -= lowerBoundV[first];

		lowerBoundV[first] = Dist(first, tour[1]);
		lowerBoundV[first] += Dist(last, first);

		lowerBoundV[last] = Dist(tour[sz-2], last);
		lowerBoundV[last] += Dist(last, first);

		minEdgesTour[first].erase(pair<int, int>(cost, last));
		minEdgesTour[last].erase(pair<int, int>(cost, first));

		lowerBound += lowerBoundV[last] + lowerBoundV[first];
	}

	void updateLowerBoundRemoveLastEdge(){
		int source = tour[sz - 2];
		int dest = tour[sz - 1];
		int cost = Dist(source, dest);

		lowerBound -= lowerBoundV[source];
		lowerBound -= lowerBoundV[dest];

		minEdgesTour[source].insert(pair<int, int>(cost, dest));
		minEdgesTour[dest].insert(pair<int, int>(cost, source));

		pair<int, int> minCostsSource = getMinEdgesCosts(minEdgesTour[source]);
		lowerBoundV[source] =  minCostsSource.first;
		if(sz >= 3){
			lowerBoundV[source] +=  Dist(tour[sz - 3], source);
		}
		else{
			lowerBoundV[source] +=  minCostsSource.second;
		}

		pair<int, int> minCostsDest = getMinEdgesCosts(minEdgesTour[dest]);
		lowerBoundV[dest] =  minCostsDest.first + minCostsDest.second;

		lowerBound += lowerBoundV[source] + lowerBoundV[dest];
	}

	void updateLowerBoundRemoveClosingEdge(){
		int source = tour[sz - 1];
		int dest = tour[0];
		int cost = Dist(source, dest);

		lowerBound -= lowerBoundV[source];
		lowerBound -= lowerBoundV[dest];

		minEdgesTour[source].insert(pair<int, int>(cost, dest));
		minEdgesTour[dest].insert(pair<int, int>(cost, source));

		pair<int, int> minCostsSource = getMinEdgesCosts(minEdgesTour[source]);
		lowerBoundV[source] =  minCostsSource.first;
		lowerBoundV[source] +=  Dist(tour[sz - 2], source);

		pair<int, int> minCostsDest = getMinEdgesCosts(minEdgesTour[dest]);
		lowerBoundV[dest] =  minCostsDest.first;
		lowerBoundV[source] +=  Dist(dest, tour[1]);

		lowerBound += lowerBoundV[source] + lowerBoundV[dest];
	}

	// Remove a última cidade do tour
	void removeLastCity(){
		int c = tour[sz - 1];
		if(sz == n){
			cost -= Dist(c, tour[0]);
			updateLowerBoundRemoveClosingEdge();
		}
		if(sz > 1){
			cost -= Dist(tour[sz - 2], c);
			updateLowerBoundRemoveLastEdge();
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

	double getLowerBound() const{
		return (double) lowerBound / 2.0;
	}

	int getDoubleLowerBound() const{
		return lowerBound;
	}

private:
	vector<int> tour; // Guarda sequência de visitas as cidades (tour)
	vector<bool> visited; // Armazena para cada cidade se a mesma já foi visitada
	int cost; // Custo do tour
	int sz; // Tamanho do tour
	int lowerBound;
	vector<int> lowerBoundV;
	vector< set< pair<int, int> > > minEdgesTour;
};


bool operator<(const Tour & t1, const Tour & t2){
	return t1.getCost() < t2.getCost();
}

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
	cout << "\tLB: " << t.getLowerBound() << endl;

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
		cout << "==== Greedy "<< i << endl;
		printTour(t);
		if(t.getCost() < best.getCost())
			best = t;
	}
	return best;
}

clock_t begin, end; // Clock de ínicio e fim para cronometrar o tempo gasto no cálculo
double timeSpent; // Tempo total gasto no cálculo

Tour best;

void dfs(Tour & t){
	if(t.getSize() == n){
		cout << t.getLowerBound() << " " << t.getCost() << endl;
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

		if(nextTour.getLowerBound() >= (double) best.getCost()){
			continue;
		}

		//candidates.push_back(pair<double, Tour>(nextTour.getLowerBound(), nextTour));
		candidates.push_back(pair<double, Tour>(nextTour.getCost(), nextTour));
	}
	sort(candidates.begin(), candidates.end());

	int nc = candidates.size();
	for(int i=0; i<nc; i++){
		if(candidates[i].second.getLowerBound() < (double) best.getCost()){
			dfs(candidates[i].second);
		}
	}
}

void dfs(){
	Tour t;
	t.addCity(0);

	dfs(t);
}

struct CompareLowerBound{
	bool operator()(const Tour & t1, const Tour & t2) const{
		return t1.getLowerBound() + (n - t1.getSize()) * EDGE_PENALTY > t2.getLowerBound() + (n - t2.getSize()) * EDGE_PENALTY;
	}
};

void bbTsp(){
	priority_queue<Tour, vector<Tour>, CompareLowerBound> q;
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
			double lowerBound = current.getLowerBound();
			if(lowerBound < (double) best.getCost()){
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
	lowerBoundIni = initializeMinEdges();

	Tour minGreedy = getMinGreedyTour();

	cout << "------------ Best Greedy --------------" << endl;
	printTour(minGreedy);

	best = minGreedy;

	begin = clock(); // Clock de início do método

	dfs();
	//bbTsp();

	end = clock(); // Clock de fim do método
	timeSpent = (double) (end - begin) / CLOCKS_PER_SEC; // Determina o tempo total gasto

	cout << "------------ Best Found --------------" << endl;
	printTour(best);

	printf("Total Execution Time: %.3f (s)\n", timeSpent);

	return 0;
}

#include "ts.h"
#include "readtsplib.h"
#include <hash_map.h>

#define INCREASE 1.5
#define DECREASE 0.5
#define MAXCHAOS 5
#define MAX_STEPS 5
#define CICLE_MAX 10
#define REP 4
#define Mult(v,m) ( (int) (((double)(v)) *m) )

using namespace std;

hash_map<int,visita,hash<int>,eqint> visitas;
int list_size,chaotic,steps_since_last_change;
double moving_average;

void initRTS(int **&tabu,int n){
	for(int i=0;i<n;i++)
		for(int j=0;j<n;j++)
			tabu[i][j] = -INF;	
	list_size = 2;
	chaotic = 0;
	moving_average = 0.0;
	steps_since_last_change = 0;
	visitas.clear();
}


/* ************************************
 * void initialGreedySolution(sol &s)
 * ************************************
 * GERA UMA SOLUÇÃO INICIAL "S"
 * Algoritmo utilizado: Heurística de Bellmore-Neumhauser, com nó inicial randômico
 */
void initialGreedySolution(sol &s, int n, int **dist, int noInicial) {
	bool *visitados = (bool*) malloc(sizeof(bool)*n);
	list<int> lista_sol;
	
	memset(visitados,0,sizeof(bool)*n);
	
	s.custo = 0;
	lista_sol.push_front(noInicial);
	visitados[noInicial] = true;
	
	for (int i=1;i<n;i++) {
		bool atras = false;
		int melhor = INF, ind = -1;

		/* escolhe a menor aresta s.v[i-1] -> nó */
		for (int j=0;j<n;j++)
			if (j != lista_sol.back() && !visitados[j] && melhor > dist(lista_sol.back(),j)) {
				melhor = dist(lista_sol.back(),j);
				ind = j;
			}
		
		/* escolhe a menor aresta no -> s.v[0] */
		for (int j=0;j<n;j++)
			if (j != lista_sol.front() && !visitados[j] && melhor > dist(lista_sol.front(),j)) {
				melhor = dist(lista_sol.front(),j);
				ind = j;
				atras = true;
			}

		visitados[ind] = true;
		if (atras) lista_sol.push_front(ind);
		else lista_sol.push_back(ind);
		
		s.custo += melhor;
	}
	s.custo += dist(lista_sol.back(),lista_sol.front());
	int j = 0;
	for (list<int>::iterator i = lista_sol.begin(); i != lista_sol.end(); i++,j++) s.v[j] = *i;

	free(visitados);
}


/* *****************************************
 * void printSolution(const sol &s, FILE *f)
 * *****************************************
 * Imprime no arquivo f a solucao s
 */
void printSolution(const sol &s, int n, FILE *f) {
	fprintf(f,"Solucao = {");
	for (int i=0;i<n;i++) fprintf(f," %d",s.v[i]);
	fprintf(f,"}\nCusto = %d\n",s.custo);
}

bool check_for_repetitions(sol s,int iter,int n){
	visita v,nova;
	int lenght;
	steps_since_last_change++;
	if(visitas.count(s.custo)){
		//printf("jah visitei\n");
		v = visitas[s.custo];
		visitas[s.custo].freq++;	
		lenght = iter - v.iter;
		visitas[s.custo].iter = iter;
		//printf("freq : %d\n",v.freq+1);
		if( v.freq+1 > REP){
			chaotic++;
			if(chaotic > MAXCHAOS){
				chaotic = 0;
				//visitas.clear();
				return false;
			}
		}
		//if(lenght < cicle_max){
			moving_average = 0.05*lenght + 0.95*moving_average;
			list_size = Mult(list_size,INCREASE);
			steps_since_last_change = 0;
		//}
		if(list_size >= n*n){
			list_size = Mult(list_size,DECREASE/2.0);
			list_size = ( list_size < 2 ) ? 2 : list_size;
		}
	}
	else{
		nova.custo = s.custo;
		nova.iter = iter;
		nova.freq = 1;
		visitas[s.custo] = nova;
	}
	//printf("moving_average : %.3lf, steps_since_last_change: %d\n",moving_average,steps_since_last_change);
	
	if(steps_since_last_change > MAX_STEPS){
		//printf("list_size : %d, dec : %.3lf\n",list_size,dec);
		list_size = Mult(list_size,DECREASE);
		list_size = ( list_size < 2 ) ? 2 : list_size;
		steps_since_last_change = 0;
	}
	return true;
}

/* *************************************************
 * void doSwap(int **next, int **prev, int **trocas)
 * *************************************************
 * Faz as trocas atualizando o vetor "next"
 * Pos-condicao: o vetor "next" contem o novo ciclo
 */
void doSwap(int *next, int *prev, int **trocas) {
	/* Recalcula o ciclo de S */
	next[ trocas[0][0] ] = trocas[1][0];
	int no = trocas[1][0];
	while (no != trocas[0][1]) {
		next[ no ] = prev[ no ];
		no = prev[ no ];
	}
	next[ trocas[0][1] ] = trocas[1][1];
}

/* ****************************************
 * void generateNewCicle(sol &s, int *next)
 * ****************************************
 * Atualiza o ciclo de "s" com o contido em "next"
 */

void generateNewCicle(sol &s, int n, int *next) {
	/* Gera o novo ciclo em 's' */
	s.v[0] = 0;
	for (int i=1;i<n;i++) {
		s.v[i] = next [ s.v[i-1] ];
	}
}

/* *************************************************************************************
 * bool twoOpt(sol &s, int n, int **dist, int **edgeTabu, int **edgeFreq, int bestCusto)
 * *************************************************************************************
 * 1. Cria um vetor "next", que armazena na posição 'i' o elemento seguinte a 'i' no ciclo hamiltoniano
 * 2. Em dois laços, vai fixando dois pares de elementos sequenciais e fazendo as trocas de aresta.
 * 	2.1. Se alguma das arestas que vai entrar acabou de sair, marca como TABU
 * 	2.2. Se alguma das arestas que vai sair acabou de entrar, marca como TABU
 * 	2.3. Se s' for melhor e (nao for tabu OU melhor que o melhor) entao para a busca e atualiza s = s'
 * 	2.4. Se s' for o melhor que piora E não for tabu, guarda as arestas que geram s' e continua a busca
 * 3. Retorna verdadeiro se conseguiu fazer alguma troca e falso caso contrario
 * PARAMETROS
 * ----------
 * s - solução atual
 * n - tamanho da instancia
 * dist - valores das arestas
 * edgeTabu - memória de curto prazo
 * edgeFreq - memória de longo prazo
 * bestCusto - melhor custo encontrado no programa
 * iter - iteracao atual do algoritmo
 */

bool twoOpt(sol &s, int n, int **dist, int** edgeTabu, int ** edgeFreq, int noInicial,
	    int noFinal, int bestCusto, int iter) {

	int ganho, nos[2][2], penalidade, maxganho, maxpen;
	bool tabu;
	
	/* inicializa as variaveis */
	maxganho = -(INF);
	maxpen = 0;

	/* Aloca a matriz que vai guardar as trocas feitas */
	int **trocas = (int**) malloc(sizeof(int*)*2);
	for (int i=0;i<2;i++) trocas[i] = (int*) malloc(sizeof(int)*2);
	
	/* alocar e atualizar o vetor de next */
	int *next = (int*) malloc(sizeof(int)*n);
	for (int i=0;i<n-1;i++)
		next[s.v[i]] = s.v[i+1];
	next[s.v[n-1]] = s.v[0];

	/* alocar e atualizar o vetor de prev */
	int *prev = (int*) malloc(sizeof(int)*n);
	for (int i=1;i<n;i++)
		prev[s.v[i]] = s.v[i-1];
	prev[s.v[0]] = s.v[n-1];
	
	nos[0][0] = 0;
	for (int i=noInicial;i<=noFinal;i++) {
		nos[0][1] = next[nos[0][0]];
		nos[1][0] = next[nos[0][1]];
		for (int j=i+2;j<=noFinal;j++) {
			nos[1][1] = next[nos[1][0]];
			
			ganho = dist( nos[0][0], nos[0][1] ) + dist( nos[1][0], nos[1][1] ) -
				dist( nos[0][0], nos[1][0] ) - dist( nos[0][1], nos[1][1] );
					
			penalidade = 0;
			// Se estamos em uma solução de não melhoria, aplicamos a memória de longo termo para diversificarmos
			if (ganho <= 0) {
				/* adiciona as penalidades relativas as entradas das arestas nos[0][0]->nos[1][0]
				 * e nos[0][1]->nos[1][1] e saidas das arestas nos[0][0]->nos[0][1] e nos[1][0]->nos[1][1] */
				penalidade += ADDPENALIDADE * max( addFreq(nos[0][0],nos[1][0]), addFreq(nos[0][1], nos[1][1]) );
				penalidade += DELPENALIDADE * max( delFreq(nos[0][0],nos[0][1]), delFreq(nos[1][0], nos[1][1]) );
			}
			
			// Descobre se a adição ou remoção de alguma das arestas é tabu
			tabu = (iter - delTabu(nos[0][0],nos[1][0]) <= list_size) || (iter - delTabu(nos[0][1],nos[1][1]) <= list_size) ||
			       (iter - addTabu(nos[0][0],nos[0][1]) <= list_size) || (iter - addTabu(nos[1][0],nos[1][1]) <= list_size);
			//printf("Iter : %d, tabu : %d\n",iter,tabu);	
			/* Pega o primeiro vizinho que apresente ganho ou critério de aspiracao */
			if (ganho > 0 && (!tabu || bestCusto > s.custo-ganho) && ganho > maxganho) {
				//printf("2-OPT: saem (%d,%d) e (%d,%d) - entram (%d,%d) e (%d,%d) - ganho = %d\n",nos[0][0],nos[0][1],
				//	nos[1][0],nos[1][1],nos[0][0],nos[1][0],nos[0][1],nos[1][1],ganho);
				maxganho = ganho;
				maxpen = 0;
				trocas[0][0] = nos[0][0];
				trocas[0][1] = nos[0][1];
				trocas[1][0] = nos[1][0];
				trocas[1][1] = nos[1][1];
			}
			
			/* Se não houver solucao que melhore, pega a que menos piora */
			if (ganho - penalidade > maxganho && !tabu) {
				//printf("2-OPT: saem (%d,%d) e (%d,%d) - entram (%d,%d) e (%d,%d) - ganho = %d\n",nos[0][0],nos[0][1],
				//	nos[1][0],nos[1][1],nos[0][0],nos[1][0],nos[0][1],nos[1][1],ganho);
				maxganho = ganho;
				maxpen = penalidade;
				trocas[0][0] = nos[0][0];
				trocas[0][1] = nos[0][1];
				trocas[1][0] = nos[1][0];
				trocas[1][1] = nos[1][1];
			}
	
			// atualiza os nós (vai para o proximo)
			nos[1][0] = nos[1][1];
		}
		// atualiza os nós (vai para o proximo)
		nos[0][0] = nos[0][1];
	}
	
	if (maxganho == -INF) {
		free(next);
		free(prev);
		for (int i=0;i<2;i++) free(trocas[i]);
		free(trocas);
		return false;
	}
	
	/* s <- s'
	 * Atualiza a solucao
	 */
	doSwap(next,prev,trocas);
	generateNewCicle(s,n,next);
	s.custo = s.custo - maxganho;
	
	/* Atualiza a memória de longo prazo: frequências de inserções e deleções */
	addFreq( trocas[0][0], trocas[1][0] )++;
	addFreq( trocas[0][1], trocas[1][1] )++;
	
	delFreq( trocas[0][0], trocas[0][1] )++;
	delFreq( trocas[1][0], trocas[1][1] )++;
			
	/* Atualiza a memória de curto prazo: atributos com status tabu */
	addTabu( trocas[0][0], trocas[1][0] ) = iter;
	addTabu( trocas[0][1], trocas[1][1] ) = iter;
	
	delTabu( trocas[0][0], trocas[0][1] ) = iter;
	delTabu( trocas[1][0], trocas[1][1] ) = iter;
	
	free(next);
	free(prev);
	for (int i=0;i<2;i++) free(trocas[i]);
	free(trocas);
	
	return true;
}



/* ************************************************
 * void tabuSearch(sol *s, int n, list<sol*> &elite)
 * *************************************************
 * Faz uma busca tabu a partir da solucao s, populando a lista de solucoes "elite"
 * s - solucao inicial a ser usada na busca
 * n - tamanho da instancia
 * edgeFreq - memoria de longo prazo
 * elite - lista de solucoes elite
 */
sol reativeTabuSearch(sol &s, int n, int **dist, int **&edgeFreq, TRandomMersenne &rg) {
	sol melhor;
	melhor.custo = INF;
	
	/* inicializa a memória de curto prazo */
	int **edgeTabu = (int**) calloc(n,sizeof(int*));
	for (int i=0;i<n;i++) edgeTabu[i] = (int*) calloc(n,sizeof(int));
	
	initRTS(edgeTabu,n);
	
	int iter = 1, numiter = (n < 300) ? 2000 : 500;
	
	while (iter < numiter && twoOpt(s, n, dist, edgeTabu, edgeFreq,0,n-1, melhor.custo,iter)) {
		//printf("list_size : %d\n",list_size);
		if(!check_for_repetitions(s,iter,n)){
			//printf("CHAOS\n");
			int steps = (int) (1.0 + (1.0+(((double) rg.IRandom(0,100)) * 10e-2))*(moving_average/2.0));  
			initRTS(edgeTabu,n);
			numiter += steps;
		}
		if (s.custo < melhor.custo)
			melhor = s;
		//printf("Iteracao = %d Solucao de Custo = %d Melhor solucao = %d\n",iter,s.custo,melhor.custo);
		//printSolution(s,n,stdout);
		//printf("Iteracao %d\n",iter);
		iter++;
	}
	//printf("Iteracao = %d Solucao de Custo = %d Melhor solucao = %d\n",iter,s.custo,melhor.custo);
	//printf("ULTIMA : %d\n",iter);
	for (int i=0;i<n;i++) free(edgeTabu[i]);
	free(edgeTabu);
	
	return melhor;
}

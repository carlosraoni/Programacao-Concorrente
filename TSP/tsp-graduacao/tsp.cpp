#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "tsp.h"
#include "randomc.h"

int ** d,n;
TRandomMersenne r(time(0));

void set_tam(int tam){
	n = tam;
}

int calc_cost(Tour tour){
	int cost = 0;
	for(int i=0;i<n;i++)
		cost += dist(tour->c[i],tour->c[PROX(i)]);
	return cost;
}

void print_tour(Tour tour){
	for(int i=0;i<n;i++)
		printf("%d -> ",tour->c[i]);
	printf("%d\n",tour->c[0]);
	printf("CUSTO : %d\n",tour->cost);
}

void copy_tour(Tour dest,Tour src){
	dest->cost = src->cost;
	for(int i=0;i<n;i++){
		dest->c[i] = src->c[i];
		dest->pos[i] = src->c[i];
	}
}

void free_tour(Tour tour){
	free(tour->c);
	free(tour->pos);
	free(tour);
}

Tour rand_tour(){
	int * visitado = new_int(n);	
	
	Tour tour = new_tour();
	tour->c = new_int(n);
	tour->pos = new_int(n);
	
	init_int(visitado,n);
	for(int i=0;i<n;i++){
		int c = Rand();
		while(visitado[c])
			c = Rand();
		visitado[c] = 1;
		tour->c[i] = c;
		tour->pos[c] = i;
	}
	tour->cost = calc_cost(tour);
	return tour;
}

Tour greedy_tour(int inicial){
	int * visitado = new_int(n),melhor,menor,atual;	
	
	Tour tour = new_tour();
	tour->c = new_int(n);
	tour->pos = new_int(n);	
	
	init_int(visitado,n);
	atual = inicial;
	visitado[atual] = 1;
	tour->c[0] = atual;
	tour->pos[atual] = 0;
	
	for(int i=1;i<n;i++){
		menor = INF;
		for(int j=0;j<n;j++){
			if(!visitado[j] && dist(atual,j)<menor){
				menor = dist(atual,j);
				melhor = j;
			}
		}
		atual = melhor;
		tour->c[i] = atual;
		tour->pos[atual] = i;
		visitado[atual] = 1;
	}
	
	tour->cost = calc_cost(tour);
	return tour;
}

#define TAM_PMX 5

void gerafilho(Tour filho,Tour p1,Tour p2,int ini,int fim){
	int * visitado = new_int(n),atual;
	init_int(visitado,n);
	for(int i=ini;i<fim;i++){
		filho->c[i] = p2->c[i];
		visitado[filho->c[i]] = 1;
	}
	for(int i=0;i<n;i++){
		if(i>=ini && i<fim) continue;
		if(!visitado[p1->c[i]])
			filho->c[i] = p1->c[i];
		else{
			atual = p1->c[p2->pos[p1->c[i]]];
			while(visitado[atual])
				atual = p1->c[p2->pos[atual]];
			filho->c[i] = atual;
		}
		visitado[filho->c[i]] = 1;
	}
	filho->cost = 0;
	for(int i=0;i<n;i++){
		filho->pos[filho->c[i]] = i;
		filho->cost += dist(filho->c[i],filho->c[PROX(i)]);
	}
	free(visitado);
}

void PMX(Tour f1,Tour f2,Tour p1,Tour p2){
	int ini = Rand(),fim;
	if(ini + TAM_PMX >= n)
		ini -= (ini + TAM_PMX - n);
	fim = ini + TAM_PMX;
	gerafilho(f1,p1,p2,ini,fim);
	gerafilho(f2,p2,p1,ini,fim);
}





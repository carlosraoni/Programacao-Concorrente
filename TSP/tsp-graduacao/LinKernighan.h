#ifndef LK_H
#define LK_H

#include <iostream>
#include <hash_set.h>
#include "tsp.h"

#define MAXOPT 4
#define MAXCAND 20

typedef struct{
	int * t,opt,ganho;
}Solucao;

typedef struct{
	int from,to;	
}Aresta;

typedef struct{
	int id,cost;
}Candidate;

struct eq{
	bool operator()(const int c1,const int c2) const {
		return (c1==c2);
	}
};

extern hash_set<int, hash<int>, eq> X,Y,visitas;
extern int ** candidates;

void LinKernighan(Tour tour);
void createSet();

#endif

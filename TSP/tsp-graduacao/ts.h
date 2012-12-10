#ifndef TS_H
#define TS_H

#define addTabu(i,j) edgeTabu[max(i,j)][min(i,j)]
#define delTabu(i,j) edgeTabu[min(i,j)][max(i,j)]

#define addFreq(i,j) edgeFreq[min(i,j)][max(i,j)]
#define delFreq(i,j) edgeFreq[max(i,j)][min(i,j)]

#define ADDPENALIDADE 10
#define DELPENALIDADE 10
#define MAX 4000
#define INF 1000000000

#include <cstdio>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <list>
#include "randomc.h"

using namespace std;

typedef struct {
	short v[MAX];
	int custo;
} sol;

typedef struct{
	int custo,iter,freq;
} visita;

struct eqint{
         bool operator ()(const int i1,const int i2)const{
                 return i1-i2;        
         }
};

/* Funcoes da biblioteca */
void initialGreedySolution(sol &s, int n, int **dist, int noInicial);
void printSolution(const sol &s, int n, FILE *f);
void doSwap(int *next, int *prev, int **trocas);
void generateNewCicle(sol &s, int n, int *next);
bool twoOpt(sol &s, int n, int **dist, int **edgeTabu, int **edgeFreq, int noInicial, int noFinal, int bestCusto, int iter);
//void tabuSearch(sol &s, int n, int **dist, int **edgeFreq, list<sol*> &elite);
sol reativeTabuSearch(sol &s, int n, int **dist, int **&edgeFreq, TRandomMersenne &rg);

#endif

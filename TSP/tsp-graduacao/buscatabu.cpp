#include <cstdio>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <sys/times.h>
#include <sys/types.h>

using namespace std;

/* para leitura dos dados da TSPLIB */
#include "readtsplib.h"
/* gerador de números aleatórios Mersenne Twister */
#include "randomc.h"
/* biblioteca que implementa as funcoes da busca */
#include "ts.h"

#define LEN 200
#define RANDOMSEED time(0)

#define addTabu(i,j) edgeTabu[max(i,j)][min(i,j)]
#define delTabu(i,j) edgeTabu[min(i,j)][max(i,j)]

#define addFreq(i,j) edgeFreq[min(i,j)][max(i,j)]
#define delFreq(i,j) edgeFreq[max(i,j)][min(i,j)]


int **dist, n; // número de nós e matriz de distancias
int **edgeFreq;
sol s;

/* PARAMETROS DO ALGORITMO */
#define NUMITERACOES 12

int iter = 0; // interação atual do algoritmo
TRandomMersenne rg(RANDOMSEED);
sol elite[NUMITERACOES];

int main(int argc, char **argv) {
	/* PASSO 0. Lê e monta a matriz de distâncias */
	read(argv[1],dist,n);

	/* GUARDA A SOLUCAO OTIMA */
	int otimo,achou_otimo = 0;
	sscanf(argv[2],"%d",&otimo);
		
	/* CALCULA O TEMPO */
	struct tms tempoAntes, tempoDepois;
	times(&tempoAntes);
	
	/* PASSO 2. Inicializa a memória de longo prazo */
	edgeFreq = (int**) calloc(n,sizeof(int*));
	for (int i=0;i<n;i++) edgeFreq[i] = (int*) calloc(n,sizeof(int*));
	
	while (iter < NUMITERACOES) {
		initialGreedySolution(s,n,dist,rg.IRandom(0,n-1));	
		elite[iter] = reativeTabuSearch(s,n,dist,edgeFreq,rg);
		if(elite[iter].custo == otimo){
			achou_otimo = iter;
			break;
		}
		//printf("iteracao %d melhor solucao = %d\n",iter,elite[iter].custo);
		iter++;
	}
	
	/* FASE DE INTENSIFICACAO */
	sol atual, bestGlobal = elite[achou_otimo];
	if(bestGlobal.custo != otimo){
		for (int i=0;i < iter;i++) {
			for (int j=0;j<n;j++) memset(edgeFreq[j],0,sizeof(int)*n);
			atual = reativeTabuSearch(elite[i],n,dist,edgeFreq,rg);
			
			if (atual.custo < bestGlobal.custo) bestGlobal = atual;
			if (atual.custo == otimo) break;
		}
	}
	/* desaloca a memoria de longo prazo */
	for (int i=0;i<n;i++) free(edgeFreq[i]);
	free(edgeFreq);
	
	/* desaloca a matriz de distancias */
	for (int i=0;i<n;i++) free(dist[i]);
	free(dist);
	
	times(&tempoDepois);
	
	printf("%s\t%d\t%d\t%.2lf\t%.2lf\n",argv[1],bestGlobal.custo, otimo, (double)(bestGlobal.custo - otimo) / (double)otimo * 100.0,
		(double) (tempoDepois.tms_utime - tempoAntes.tms_utime) / 100.0);
	
	return 0;
}

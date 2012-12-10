#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <cmath>
#include <sys/types.h>
#include <sys/times.h>
#include "readtsplib.h"
#include "randomc.h"


#define INF 1000000000

#define new_int(tam) ((int *)malloc(sizeof(int)*tam))
#define init_int(ptr,tam) (memset(ptr,0,sizeof(int)*tam))
#define new_tour() ((Tour)malloc(sizeof(struct Tour_)))
#define PROX(c) (c==n-1 ? 0 : c+1)
#define ANT(c) (c==0 ? n-1 : c-1 )
#define ACC 0.6
#define BETA 2.5
#define MAXC 4000

#ifdef dist
#undef dist
#define dist(i,j) (d[max(i,j)][min(i,j)])
#endif

#define Rand() (r.IRandom(0,n-1))

using namespace std;

typedef struct Tour_{
	int * c,* pos;
	int cost;
}* Tour;

typedef struct{
	int from,to;	
}Aresta;

int n, **d ,* visitado,porra;
TRandomMersenne r(time(0));
Aresta Edge[MAXC];
//Aresta * Edge;

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

Tour rand_tour(){
	
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
	int melhor,menor,atual;	
	
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

void build_tour(Tour tour,Aresta * set,int custo){
	int cont=0; 
	int atual,achei = 0;
		
	tour->c[0] = set[0].from;
	tour->c[1] = set[0].to;
	atual = set[0].to;
	tour->pos[tour->c[0]] = 0;
	tour->pos[tour->c[1]] = 1;
	
	init_int(visitado,n);
	visitado[0] = 1;
	cont = 2;
	
	while(cont<n){
		for(int ind=0;ind<n;ind++){
			if(visitado[ind])
				continue;
			if(set[ind].from == atual){
				tour->c[cont] = set[ind].to;
				tour->pos[tour->c[cont]] = cont;
				atual = set[ind].to;
				achei++;
				visitado[ind] = 1;
				break;
			}	
			else if(set[ind].to == atual){
				tour->c[cont] = set[ind].from;	
				tour->pos[tour->c[cont]] = cont;
				atual = set[ind].from;
				visitado[ind] = 1;
				achei++;
				break;
			}		
		}
		cont++;
	}
	tour->cost = custo;
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

int det3Opt(Tour tour,Aresta * Edge,int i,int j, int k){
	int J=tour->c[j],PJ = tour->c[PROX(j)],I=tour->c[i],PI = tour->c[PROX(i)],K=tour->c[k],PK = tour->c[PROX(k)];
	int mincost,cost,caso = 0;
	
	mincost = dist(J,K) + dist(PJ,PK) + dist(I,PI);
	
	cost = dist(J,PJ) + dist(K,I) + dist(PK,PI);
	if(cost < mincost){
		mincost = cost;
		caso = 1;
	}
	
	cost = dist(K,PK) + dist(I,J) + dist(PI,PJ);
	if(cost < mincost){
		mincost = cost;
		caso = 2;
	}	
	
	cost = dist(J,I) + dist(PK,PJ) + dist(K,PI);
	if(cost < mincost){
		mincost = cost;
		caso = 3;
	}	
	
	cost = dist(PI,PK) + dist(J,K) + dist(PJ,I);
	if(cost < mincost){
		mincost = cost;
		caso = 4;
	}	
	
	cost = dist(PJ,I) + dist(PK,J) + dist(PI,K);
	if(cost < mincost){
		mincost = cost;
		caso = 5;
	}	
	
	cost = dist(K,I) + dist(PK,J) + dist(PI,PJ);
	if(cost < mincost){
		mincost = cost;
		caso = 6;
	}
	
	switch(caso){
		case 0: 
			Edge[0].from = tour->c[j]; Edge[0].to = tour->c[k];
			Edge[1].from = tour->c[PROX(j)]; Edge[1].to = tour->c[PROX(k)];
			Edge[2].from = tour->c[i]; Edge[2].to = tour->c[PROX(i)];
			break;
		case 1: 	
			Edge[0].from = tour->c[j]; Edge[0].to = tour->c[PROX(j)];
			Edge[1].from = tour->c[k]; Edge[1].to = tour->c[i];
			Edge[2].from = tour->c[PROX(k)]; Edge[2].to = tour->c[PROX(i)];
			break;
		case 2: 	
			Edge[0].from = tour->c[k]; Edge[0].to = tour->c[PROX(k)];
			Edge[1].from = tour->c[i]; Edge[1].to = tour->c[j];
			Edge[2].from = tour->c[PROX(i)]; Edge[2].to = tour->c[PROX(j)];
			break;
		case 3: 	
			Edge[0].from = tour->c[j]; Edge[0].to = tour->c[i];
			Edge[1].from = tour->c[PROX(k)]; Edge[1].to = tour->c[PROX(j)];
			Edge[2].from = tour->c[k]; Edge[2].to = tour->c[PROX(i)];
			break;
		case 4: 	
			Edge[0].from = tour->c[PROX(i)]; Edge[0].to = tour->c[PROX(k)];
			Edge[1].from = tour->c[j]; Edge[1].to = tour->c[k];
			Edge[2].from = tour->c[PROX(j)]; Edge[2].to = tour->c[i];
			break;
		case 5: 	
			Edge[0].from = tour->c[PROX(j)]; Edge[0].to = tour->c[i];
			Edge[1].from = tour->c[PROX(k)]; Edge[1].to = tour->c[j];
			Edge[2].from = tour->c[PROX(i)]; Edge[2].to = tour->c[k];
			break;
		case 6: 	
			Edge[0].from = tour->c[k]; Edge[0].to = tour->c[i];
			Edge[1].from = tour->c[PROX(k)]; Edge[1].to = tour->c[j];
			Edge[2].from = tour->c[PROX(i)]; Edge[2].to = tour->c[PROX(j)];
			break;
	}
	//return (dist(Edge[0].from,Edge[0].to)+dist(Edge[1].from,Edge[1].to)+dist(Edge[2].from,Edge[2].to));
	return mincost;
}

int cmp(const void * c1,const void * c2){
	return *(int *)c1 - *(int *)c2;
}

int main(int argc,char ** argv){
	
	double Tf,Temp,alfa;
	int nit,it,co,i,j,k,ganho,ind,tmp[3],aquecer,otimo;	
	struct tms before,after;
	Tour tour,melhor;
	times(&before);
	
	if(argc == 7){
		nit = atoi(argv[2]);
		alfa = atof(argv[3]);
		Tf = atof(argv[4]);
		co = atoi(argv[5]);
		otimo = atoi(argv[6]);
	}
	else{
		printf("Uso: %s 'Instancia' 'No de Iteracoes por Temperatura' 'Taxa de Resfriamento' 'Temperatura Final' 'Metodo Construtivo Inicial' 'custo otimo'\n",argv[0]);
		printf("Metodos Construtivos:\n1 - Vizinho mais proximo\n2 - Aleatorio\n3 - Economias\n");
		exit(1);
	}
	
	read(argv[1],d,n);
	visitado =  new_int(n);
	
	switch(co){
		case 1: tour = greedy_tour(Rand()); break;
		case 2: tour = rand_tour(); break;
		//case 3: tour = saving_tour(); break;
	}
	//print_tour(tour);	
	melhor = new_tour();
	copy_tour(melhor,tour);
	aquecer = 1;
	Temp = 10.0;	
	//Edge = (Aresta *) malloc(sizeof(Aresta) * n);
	
	while(aquecer || Temp>Tf){
		int aceitos = 0,maxit;
		maxit = (aquecer) ? nit/10 : nit;
		for(it=0;it<maxit;it++){
			tmp[0]= Rand();
			tmp[1] = Rand();
			while(tmp[0]==tmp[1] || tmp[0]==ANT(tmp[1]) || tmp[0]==PROX(tmp[1]))
				tmp[1] = Rand();
			tmp[2] = Rand();
			while(tmp[2]==tmp[0] || tmp[2]==tmp[1] || tmp[0]==PROX(tmp[2]) || tmp[0]==ANT(tmp[2]) || tmp[1]==PROX(tmp[2]) || tmp[1]==ANT(tmp[2]))
				tmp[2] = Rand();
			qsort(tmp,3,sizeof(int),cmp);
			i = tmp[0]; j=tmp[1]; k = tmp[2];
			ganho = dist(tour->c[i],tour->c[PROX(i)]) + dist(tour->c[j],tour->c[PROX(j)]) + dist(tour->c[k],tour->c[PROX(k)]);
			ganho -= det3Opt(tour,Edge,i,j,k);
			ind = 3;
			if( ganho > 0 || ( 1.0/ (Rand()+1) ) < exp((double)-ganho/Temp)   ){
				aceitos++;
				for(int c=0;c<n;c++){
					if(c==i || c==j || c==k)
						continue;
					Edge[ind].from = tour->c[c];
					Edge[ind].to = tour->c[PROX(c)];
					ind++;
				}
				build_tour(tour,Edge,tour->cost - ganho);
				if(tour->cost < melhor->cost){
					copy_tour(melhor,tour);
					if(melhor->cost == otimo)
						break;
				}
			}
		}
		if(melhor->cost == otimo)
			break;
		if(aquecer){
			if(((double)aceitos/(double)maxit) > ACC)
				aquecer = 0;
			else
				Temp *= BETA;
		}
		else
			Temp *= alfa;
		//printf("Temp : %lf\n",Temp);
	}
	
	times(&after);

	printf("%s\t%.2lf\t%d\t%d\t%.3lf\n",argv[1],((double)(after.tms_utime-before.tms_utime)/100.0),melhor->cost,otimo,(double)(((double)(melhor->cost - otimo)/(double) otimo)*100.0));
	//print_tour(melhor);
	
	
	return 0;
}

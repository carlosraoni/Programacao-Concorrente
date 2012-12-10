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
	int * c;
	int cost;
}* Tour;

int n, **d ,* visitado,porra;
TRandomMersenne r(time(0));

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
	
	init_int(visitado,n);
	for(int i=0;i<n;i++){
		int c = Rand();
		while(visitado[c])
			c = Rand();
		visitado[c] = 1;
		tour->c[i] = c;
	}
	tour->cost = calc_cost(tour);
	return tour;
}

Tour greedy_tour(int inicial){
	int melhor,menor,atual;	
	
	Tour tour = new_tour();
	tour->c = new_int(n);
	
	init_int(visitado,n);
	atual = inicial;
	visitado[atual] = 1;
	tour->c[0] = atual;
	
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
		visitado[atual] = 1;
	}
	tour->cost = calc_cost(tour);
	return tour;
}

int caso;

int build_block(Tour tour,int * b,int i,int j){
	int ind = 0;
	while(i!=j){
		b[ind++] = tour->c[i];
		i = PROX(i);
	}
	b[ind++] = tour->c[j];
	return ind;
}

void swap_block(int * b,int tam){
	int tmp;
	for(int i=0;i<tam/2;i++){
		tmp = b[i];
		b[i] = b[tam-1-i];
		b[tam-1-i] = tmp;
	}
}

int * b1, * b2, * b3, * tour_tmp;

void new_blocks(){
	b1 = new_int(n);
	b2 = new_int(n);
	b3 = new_int(n);
	tour_tmp = new_int(n);
}

void build_tour(Tour tour,int i,int j,int k,int custo){
	int t1,t2,t3;

	t1 = build_block(tour,b1,PROX(k),i);
	t2 = build_block(tour,b2,PROX(i),j);
	t3 = build_block(tour,b3,PROX(j),k);

	switch(caso){
		case 0: swap_block(b3,t3);
			memcpy(tour_tmp,b1,sizeof(int) * t1);
			memcpy(tour_tmp+t1,b2,sizeof(int) * t2);	
			memcpy(tour_tmp+t1+t2,b3,sizeof(int) * t3);
			break;			
	
		case 1: swap_block(b1,t1);	
			memcpy(tour_tmp,b2,sizeof(int) * t2);
			memcpy(tour_tmp+t2,b3,sizeof(int) * t3);	
			memcpy(tour_tmp+t2+t3,b1,sizeof(int) * t1);
			break;		
		
		case 2: swap_block(b2,t2);	
			memcpy(tour_tmp,b3,sizeof(int) * t3);
			memcpy(tour_tmp+t3,b1,sizeof(int) * t1);	
			memcpy(tour_tmp+t3+t1,b2,sizeof(int) * t2);
			break;		
		
		case 3: swap_block(b1,t1);
			memcpy(tour_tmp,b3,sizeof(int) * t3);
			memcpy(tour_tmp+t3,b2,sizeof(int) * t2);	
			memcpy(tour_tmp+t2+t3,b1,sizeof(int) * t1);
			break;			
		
		case 4: swap_block(b2,t2);	
			memcpy(tour_tmp,b2,sizeof(int) * t2);
			memcpy(tour_tmp+t2,b1,sizeof(int) * t1);	
			memcpy(tour_tmp+t1+t2,b3,sizeof(int) * t3);
			break;			
		
		case 5: 	
			memcpy(tour_tmp,b1,sizeof(int) * t1);
			memcpy(tour_tmp+t1,b3,sizeof(int) * t3);	
			memcpy(tour_tmp+t1+t3,b2,sizeof(int) * t2);
			break;			
		
		case 6: swap_block(b3,t3);	
			memcpy(tour_tmp,b1,sizeof(int) * t1);
			memcpy(tour_tmp+t1,b3,sizeof(int) * t3);	
			memcpy(tour_tmp+t1+t3,b2,sizeof(int) * t2);
			break;		
	}
	memcpy(tour->c,tour_tmp,sizeof(int)*n);
	tour->cost = custo;
	//printf("Esperado : %d , Real : %d , caso : %d\n",tour->cost,calc_cost(tour),caso);
}

void copy_tour(Tour dest,Tour src){
	dest->cost = src->cost;
	for(int i=0;i<n;i++){
		dest->c[i] = src->c[i];
	}
}

void free_tour(Tour tour){
	free(tour->c);
	free(tour);
}

int det3Opt(Tour tour,int i,int j, int k){
	int J=tour->c[j],PJ = tour->c[PROX(j)],I=tour->c[i],PI = tour->c[PROX(i)],K=tour->c[k],PK = tour->c[PROX(k)];
	int mincost,cost;
	
	caso = 0;
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

	return mincost;
}

int cmp(const void * c1,const void * c2){
	return *(int *)c1 - *(int *)c2;
}

int main(int argc,char ** argv){
	
	double Tf,Temp,alfa;
	int nit,it,co,i,IT,j,k,ganho,ind,tmp[3],aquecer,otimo;	
	struct tms before,after;
	Tour tour,melhor;
	times(&before);
	
	if(argc == 7){
		nit = atoi(argv[2]);
		alfa = atof(argv[3]);
		IT = atoi(argv[4]);
		co = atoi(argv[5]);
		otimo = atoi(argv[6]);
	}
	else{
		printf("Uso: %s 'Instancia' 'No de Iteracoes por Temperatura' 'Taxa de Resfriamento' 'Numero de Iteracoes' 'Metodo Construtivo Inicial' 'custo otimo'\n",argv[0]);
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
	melhor = new_tour();
	melhor->c = new_int(n);
	
	copy_tour(melhor,tour);
	aquecer = 1;
	Temp = 10.0;	
	//Edge = (Aresta *) malloc(sizeof(Aresta) * n);
	new_blocks();	
	//printf("%d\n",IT);
	int sem_melhoria = 0,c_ant = melhor->cost;
	while(IT){
		int aceitos = 0,maxit;
		maxit = (aquecer) ? 15 : nit;
		//if(aquecer == 2)
		//	maxit = 30;
		c_ant = melhor->cost;
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
			ganho -= det3Opt(tour,i,j,k);
			ind = 3;
			if( ganho > 0 || ( 1.0/ (Rand()+1) ) < exp((double)ganho/Temp)   ){
				aceitos++;
				build_tour(tour,i,j,k,tour->cost - ganho);
				if(tour->cost < melhor->cost){
					copy_tour(melhor,tour);
					if(melhor->cost == otimo)
						break;
				}
			}
		}
		if(melhor->cost == otimo)
			break;
		if(c_ant == melhor->cost)
			sem_melhoria++;
		if(aquecer){
			if(aquecer==1 && ((double)aceitos/(double)maxit) > ACC)
				aquecer = 0;
			else 
				if(aquecer==2 && ((double)aceitos/(double)maxit) > 0.01)
					aquecer = 0;
			else
				Temp *= BETA;
		}
		else{
			if(sem_melhoria >= 5000){
				aquecer = 2;
				sem_melhoria = 0;
			}
			else
				Temp *= alfa;
		}
		//if(aquecer == 2)
		//		printf("%d\n",IT);
		//printf("Temp : %lf\n, aquecer : %d\n",Temp,aquecer);
		if(!aquecer)
			IT--;
	}
	
	times(&after);

	printf("%s\t%.2lf\t%d\t%d\t%.3lf\n",argv[1],((double)(after.tms_utime-before.tms_utime)/100.0),melhor->cost,otimo,(double)(((double)(melhor->cost - otimo)/(double) otimo)*100.0));
	//print_tour(melhor);
	
	
	return 0;
}

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <hash_set.h>
#include <sys/types.h>
#include <sys/times.h>
#include "readtsplib.h"
#include "randomc.h"

#define MAXOPT 4
#define INF 1000000000
#define MAXCAND 20

#define new_int(tam) ((int *)malloc(sizeof(int)*tam))
#define init_int(ptr,tam) (memset(ptr,0,sizeof(int)*tam))
#define new_tour() ((Tour)malloc(sizeof(struct Tour_)))
#define PROX(c) (c==n-1 ? 0 : c+1)
#define ANT(c) (c==0 ? n-1 : c-1 )

#ifdef dist
#undef dist
#define dist(i,j) (d[max(i,j)][min(i,j)])
#endif

#define Rand() (r.IRandom(0,n-1))

using namespace std;

typedef struct Tour_{
	int * c,* pos;
	unsigned long long cost;
}* Tour;

typedef struct{
	int * t,opt,ganho;
}Solucao;

typedef struct{
	int from,to;	
}Aresta;

typedef struct{
	int id,cost;
}Candidate;

struct eq
{
  bool operator()(const int c1,const int c2) const
  {
	return (c1==c2);
  }
};

hash_set<int, hash<int>, eq> X,Y;
int ** d,** candidates,n;
Solucao best_sol;

int key(int i,int j){
	int from = max(i,j),to = min(i,j);
	return ((from*n)+to);
}

int lookup(const hash_set<int, hash<int>, eq>& Set,int c)
{
	hash_set<int, hash<int>, eq>::iterator it = Set.find(c);
	return (it != Set.end());
}

unsigned long long calc_cost(Tour tour){
	unsigned long long cost = 0;
	for(int i=0;i<n;i++)
		cost += dist(tour->c[i],tour->c[PROX(i)]);
	return cost;
}

void print_tour(Tour tour){
	for(int i=0;i<n;i++)
		printf("%d -> ",tour->c[i]);
	printf("%d\n",tour->c[0]);
	printf("CUSTO : %Lu\n",tour->cost);
}

Tour rand_tour(){
	int * visitado = new_int(n);	
	TRandomMersenne r(time(0));
	
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

void seq_KOpt(Tour tour,int * t,int opt){
	Aresta * set = (Aresta *)malloc(sizeof(Aresta)*n);
	int cont=0 , * visitado = new_int(n);
	init_int(visitado,n);
	
	X.clear();
	for(int i=0;i<opt*2;i+=2)
		X.insert(key(t[i],t[i+1]));
	//if(opt==4)
	//	for(int i=0;i<2*opt;i++)
	//		printf("t[%d] : %d\n",i,t[i]);
	for(int i=0;i<n;i++){
		int chave = key(tour->c[i],tour->c[PROX(i)]);  
		if(!lookup(X,chave)){
			set[cont].from = tour->c[i];
			set[cont].to = tour->c[PROX(i)];
			cont++;
		}
	}
	for(int i=1;cont<n-1;i+=2){
		set[cont].from = t[i];
		set[cont].to = t[PROX(i)];
		cont++;
	}
	set[cont].from = t[2*opt-1];
	set[cont].to = t[0];
	int atual = t[0],achei = 0;
	cont = 1;
	tour->c[0] = atual;
	while(cont<n){
		for(int ind=n-1;ind>=0;ind--){
			if(!visitado[ind]){
				if(set[ind].from == atual){
					tour->c[cont] = set[ind].to;
					atual = set[ind].to;
					achei++;
					visitado[ind] = 1;
					break;
				}	
				if(set[ind].to == atual){
					tour->c[cont] = set[ind].from;
					atual = set[ind].from;
					visitado[ind] = 1;
					achei++;
					break;
				}
				
			}
		}
		cont++;
	}
	//printf("Achei : %d , Opt : %d\n",achei,opt);
	for(int i=0;i<n;i++)
		tour->pos[tour->c[i]] = i;
	tour->cost = calc_cost(tour);	
}

int in_interval(int pos,int inicio,int fim){
	if(fim>inicio)
		return (pos>inicio && pos<fim);
	return (pos>inicio || pos<fim);
}

int recursiveLK(Tour tour,int * t,int opt,int Ganho){
	int y1 = t[opt*2-1],y2,pos,Gtmp;
	pos = tour->pos[y1];
	if(opt == MAXOPT)
		return 0;
	for(int i=0;i<MAXCAND;i++){
		y2 = candidates[y1][i];
		if(y2==y1 || y2==tour->c[PROX(pos)] || y2==tour->c[ANT(pos)])
			continue;	
		if((Ganho - dist(y1,y2))<0)
			continue;
		if(lookup(X,key(y1,y2)))
			continue;
		if(opt > 1 && (y2==tour->c[PROX(tour->pos[t[0]])] || y2==tour->c[ANT(tour->pos[t[0]])]))
			continue;		
		if(opt > 1 && (y2==t[2] || y2==tour->c[PROX(tour->pos[t[2]])] || y2==tour->c[ANT(tour->pos[t[2]])]))
			continue;
		switch(opt){
			case 1: t[3] = tour->c[ANT(tour->pos[y2])]; break;
			case 2: if(in_interval(tour->pos[y2],tour->pos[t[1]],tour->pos[t[3]]))
					t[5] = tour->c[PROX(tour->pos[y2])];
				else
					t[5] = tour->c[ANT(tour->pos[y2])];
				break;
			case 3:	if(t[5] == tour->c[PROX(tour->pos[t[4]])]){
					if(in_interval(tour->pos[y2],tour->pos[t[1]],tour->pos[t[4]]))
						t[7] = tour->c[PROX(tour->pos[y2])]; 
					else	
						t[7] = tour->c[ANT(tour->pos[y2])]; 
				}
				else{	
					if(in_interval(tour->pos[y2],tour->pos[t[2]],tour->pos[t[5]]))
						t[7] = tour->c[PROX(tour->pos[y2])]; 
					else	
						t[7] = tour->c[ANT(tour->pos[y2])]; 
				}
				break;
		}
		if(lookup(Y,key(y2,t[2*opt+1])))
			continue;
		t[opt*2] = y2;		
		Y.insert(key(y1,y2));	
		X.insert(key(y2,t[2*opt+1]));
		Ganho = Ganho - dist(y1,y2) + dist(y2,t[2*opt+1]);
		Gtmp = Ganho - dist(t[opt*2+1],t[0]);
		if(Gtmp>0 && !lookup(X,key(t[2*opt+1],t[0]))){
			if(Gtmp > best_sol.ganho){
				best_sol.ganho = Gtmp;
				best_sol.opt = opt+1;
				for(int j=0;j<2*(opt+1);j++)
					best_sol.t[j] = t[j];
				//Y.insert(key(t[opt*2+1],t[0]));
				//return 1;
			}
			//Y.insert(key(t[opt*2+1],t[0]));
			//seq_KOpt(tour,t,opt+1);
			//return 1;
		}
		//if(recursiveLK(tour,t,opt+1,Ganho))
			//return 1;
		recursiveLK(tour,t,opt+1,Ganho);
		//else{
			Ganho = Ganho + dist(y1,y2) - dist(y2,t[2*opt+1]);
			Y.erase(key(y1,y2));
			X.erase(key(y2,t[2*opt+1]));
			t[2*opt] = 0;
			t[2*opt+1] = 0;
		//}
	}
	return 0;
}

void LinKernighan(Tour tour){
	int * t = new_int(MAXOPT*2+1),melhorou=1;
	best_sol.t = new_int(2*MAXOPT+1);
	while(melhorou){
		melhorou = 0;
		best_sol.ganho = 0;
		for(int i=0;i<n;i++){
			t[0] = tour->c[i];
			t[1] = tour->c[PROX(i)];
			X.clear();
			Y.clear();
			X.insert(key(t[0],t[1]));	
			recursiveLK(tour,t,1,dist(t[0],t[1]));
			/*if(recursiveLK(tour,t,1,dist(t[0],t[1]))){
				melhorou = 1;
				print_tour(tour);
				break;
			}*/
		}
		if(best_sol.ganho>0){
			melhorou = 1;
			seq_KOpt(tour,best_sol.t,best_sol.opt);
		}
	}
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

int cmp(const void * c1,const void * c2){
	Candidate * cand1 = (Candidate *)c1,* cand2 = (Candidate *)c2;
	return cand1->cost - cand2->cost;
}

void createSet(){
	Candidate * cand = (Candidate *) malloc(sizeof(Candidate)*n);
	candidates = (int **) malloc(sizeof(int *)*n);
	for(int i=0;i<n;i++)
		candidates[i] = new_int(MAXCAND);	
	for(int i=0;i<n;i++){
		for(int j=0;j<n;j++){
			if(j==i){ 
				cand[j].cost = INF;	
				continue;
			}
			cand[j].id = j;
			cand[j].cost = dist(i,j);
		}
		qsort(cand,n,sizeof(cand[0]),cmp);
		for(int k=0;k<MAXCAND;k++)
			candidates[i][k] = cand[k].id;
	}
	free(cand);
}

int main(int argc,char ** argv){
	
	int * visitado,NRUNS;
	TRandomMersenne r(time(0));
	Tour tour,melhor;
	struct tms before,after;
	
	times(&before);
	
	read(argv[1],d,n);
	
	visitado = new_int(n);
	init_int(visitado,n);

	int num = Rand();
	melhor = greedy_tour(num);
	visitado[num] = 1;

	createSet();
	LinKernighan(melhor);
	NRUNS = atoi(argv[2]);
	for(int i=1;i<NRUNS;i++){
		num = Rand();
		while(visitado[num])
			num = Rand();
		visitado[num] = 1;
		tour = greedy_tour(num);
		LinKernighan(tour);
		if(tour->cost < melhor->cost)
			copy_tour(melhor,tour);
		free_tour(tour);
	}
	
	//print_tour(melhor);
	times(&after);
	double tempo = (double)(after.tms_utime - before.tms_utime)/(double)100;
	printf("%s : %.2lfs %Lu\n",argv[1],tempo,melhor->cost);
    	//printf("System time: %ld seconds\n", after.tms_stime - before.tms_stime);
	
	return 0;

}

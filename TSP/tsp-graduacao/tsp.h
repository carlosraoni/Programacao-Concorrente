#ifndef TSP_H
#define TSP_H

#define new_tour() ((Tour)malloc(sizeof(struct Tour_)))
#define PROX(c) (c==n-1 ? 0 : c+1)
#define ANT(c) (c==0 ? n-1 : c-1 )
#define max(x,y) ( (x > y) ? x : y )
#define min(x,y) ( (x < y) ? x : y )
#define new_int(tam) ((int *)malloc(sizeof(int)*tam))
#define init_int(ptr,tam) (memset(ptr,0,sizeof(int)*tam))
#define Rand() (r.IRandom(0,n-1))

#include "randomc.h"

#define INF 1000000000

#ifndef dist
	#define dist(i,j) (d[max(i,j)][min(i,j)])
#endif

typedef struct Tour_{
	int * c,* pos;
	int cost;
}* Tour;

extern int ** d;
extern int n;

void set_tam(int tam);
int calc_cost(Tour tour);
void print_tour(Tour tour);
Tour rand_tour();
Tour greedy_tour(int inicial);
void copy_tour(Tour dest,Tour src);
void free_tour(Tour tour);
void PMX(Tour f1,Tour f2,Tour p1,Tour p2);

#endif


#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <sys/times.h>

#include "tsp.h"
#include "LinKernighan.h"
#include "readtsplib.h"
#include "randomc.h"

#define TAMPOP 100
#define TAMREFSET 10

extern TRandomMersenne r;
extern int ** d,n;

int main(int argc,char ** argv){
	
	int * visitado,otimo;
	Tour pop[TAMPOP], * melhor;
	struct tms before,after;
	
	if(argc != 3){
		printf("USO : %s 'instancia' 'otimo' \n\n",argv[0]);
		return 1;
	}
	
	otimo = atoi(argv[2]);
	times(&before);
	
	read(argv[1],d,n);
	
	visitado = new_int(n);
	init_int(visitado,n);

	pop[0] = rand_tour();
	pop[1] = rand_tour();

	createSet();
	LinKernighan(pop[0]);	
	LinKernighan(pop[1]);

	pop[2] = new_tour(); pop[2]->c = new_int(n); pop[2]->pos = new_int(n);
	pop[3] = new_tour(); pop[3]->c = new_int(n); pop[3]->pos = new_int(n);

	PMX(pop[2],pop[3],pop[0],pop[1]);
	
	LinKernighan(pop[2]);	
	LinKernighan(pop[3]);
	
	printf("PAI 1 : \n");
	print_tour(pop[0]);
	printf("PAI 2 : \n");
	print_tour(pop[1]);	
	printf("FILHO 1 : \n");
	print_tour(pop[2]);	
	printf("FILHO 2 : \n");
	print_tour(pop[3]);
	
	melhor = &pop[0];
	//print_tour(melhor);
	times(&after);
	double tempo = (double)(after.tms_utime - before.tms_utime)/(double)100;
	double gap = ((double)((*(melhor))->cost-otimo)/(double) otimo) * 100.0;
	printf("%s\t%.2lfs\t%d\t%d\t%.3lf\n",argv[1],tempo,otimo,(*(melhor))->cost,gap);
    	//printf("System time: %ld seconds\n", after.tms_stime - before.tms_stime);
	
	return 0;

}

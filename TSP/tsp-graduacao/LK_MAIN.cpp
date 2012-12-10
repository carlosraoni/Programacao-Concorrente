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

extern TRandomMersenne r;
extern int ** d,n;

int main(int argc,char ** argv){
	
	int * visitado,NRUNS,otimo;
	Tour tour,melhor;
	struct tms before,after;
	
	if(argc != 4){
		printf("USO : %s 'instancia' 'otimo' 'numero de solucoes iniciais'\n\n",argv[0]);
		return 1;
	}
	
	otimo = atoi(argv[2]);
	times(&before);
	
	read(argv[1],d,n);
	
	visitado = new_int(n);
	init_int(visitado,n);

	int num = Rand();
	melhor = greedy_tour(num);
	visitado[num] = 1;

	createSet();
	LinKernighan(melhor);
	NRUNS = atoi(argv[3]);
	for(int i=1;i<NRUNS;i++){
		num = Rand();
		while(visitado[num])
			num = Rand();
		visitado[num] = 1;
		tour = greedy_tour(num);
		LinKernighan(tour);
		if(tour->cost < melhor->cost)
			copy_tour(melhor,tour);
		if(melhor->cost == otimo)
			break;
		free_tour(tour);
	}
	
	//print_tour(melhor);
	times(&after);
	double tempo = (double)(after.tms_utime - before.tms_utime)/(double)100;
	double gap = ((double)(melhor->cost-otimo)/(double) otimo) * 100.0;
	printf("%s\t%.2lfs\t%d\t%d\t%.3lf\n",argv[1],tempo,otimo,melhor->cost,gap);
    	//printf("System time: %ld seconds\n", after.tms_stime - before.tms_stime);
	
	return 0;

}

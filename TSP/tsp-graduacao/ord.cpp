#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>

#define MAX 100

char l[MAX][MAX];
int tam[MAX];

int i,ind=0;

int localiza(char * str){
	int k;
	for(k=0;k<ind;k++){
		if(!strcmp(l[k],str))
			return k;
	}
	return 0;
}

int cmp(const void * l1,const void * l2){
	return (tam[localiza((char *)l1)] - tam[localiza((char *) l2)]);
}

int main(){
	
	while(fgets(l[ind++],MAX,stdin)){
		char lixo[MAX];
		int nada;
		sscanf(l[ind-1],"%s %d %d",lixo,&nada,&tam[ind-1]);
	}
	qsort(l,ind,sizeof(l[0]),cmp);
	for(i=0;i<ind;i++){
		printf("%s",l[i]);
	}

}



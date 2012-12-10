#include <cstdio>
#include <cstring>
#include <cstdlib>

#define MAX 100

char inst[MAX];
int otimo;

int main(int argc, char ** argv){

	int otimo,n,nit;
	char cmd[MAX*10];
	
	while(scanf("%s %d %d",inst,&otimo,&n) != EOF){
		if(n<120)
			nit = 100;
		else if(n>=120 && n<280)
			nit = 300;
		else if(n>=280 && n<= 500)
			nit = 500;
		else 
			nit = 1000;
		sprintf(cmd,"./SA2 instancias/%s %d %s %s 1 %d",inst,nit,argv[1],argv[2],otimo);
		system(cmd);
	}
	
	return 0;
}



#include <cstdio>
#include <cstring>
#include <cmath>

#define MAX 18409401
//#define MAX 20000001
#define MAXN 100001

int resp[MAXN],cont;
char nisp[MAX];

void precalc(){
	int lim = (int) sqrt(MAX); 
	//memset(isp,1,MAX*sizeof(char));	
	nisp[0] = 1;
	nisp[1] = 1;
	nisp[2] = 0;
	cont = 0;
	/*
	for(int i=3;i<MAX;i+=2){
		isp[i] = 1;
		isp[i+1] = 0;
	}
	*/
	for(int i=2;i<=lim;i++){
		if(!nisp[i] && !nisp[i-2])
			resp[cont++] = i-2;
		if(!nisp[i]){
			for(int j=i+i;j<MAX;j+=i){
				nisp[j] = 1;
			}
		}
	}
	
	for(int i=lim+1;i<MAX && cont <MAXN ;i++)
		if(!nisp[i] && !nisp[i-2])
			resp[cont++] = i - 2;
			
}

int main(){

	precalc();

	int num;

	while(scanf("%d",&num)!=EOF){
		printf("(%d,%d)\n",resp[num-1],resp[num-1]+2);
	}

	return 0;
}


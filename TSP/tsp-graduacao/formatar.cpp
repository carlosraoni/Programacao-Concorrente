#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cstdio>

#define MAX 1000

using namespace std;

char linha[MAX],inst[MAX];
int melhor,otimo,atual;
double melhor_gap,gap,tempo,acc;

int main(){

	while( cin >> inst >> tempo >> melhor >> otimo >> melhor_gap){
		acc = tempo;
		for(int i=0;i<4;i++){
			cin >> inst >> tempo >> atual >> otimo >> gap;
			acc += tempo;
			if(atual < melhor){
				melhor_gap = gap;
				melhor = atual;
			}
		}
		printf("%s\t%.3lf\t%d\t%d\t%.3lf\n",inst+11,acc/5.0,melhor,otimo,melhor_gap);
	}
	
	return 0;
}



#include <stdio.h>

int main(){
	int i;
	char linha[100];
	while(gets(linha)){
		for(i=0;i<5;i++)
			printf("%s\n",linha);
	}
	
	return 0;
}


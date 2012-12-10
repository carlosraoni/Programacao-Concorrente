#include <cstdio>
#include <cstring>
#include <cstdlib>

int main() {
	FILE *inst = fopen("inst","r");
	char instancia[50], comando[200];
	int otimo, n, cont = 0, inter = 1;
	while (fscanf(inst,"%s %d %d",instancia,&otimo,&n) == 3) {
		if (cont++ < 16)
			inter = 3;
		else inter = 1;
		for (int i=0;i<inter;i++)
			sprintf(comando,"./RTS instancias/%s %d",instancia,otimo);
		system(comando);
	}
	return 0;
}

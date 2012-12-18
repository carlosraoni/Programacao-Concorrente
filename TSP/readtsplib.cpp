#include "readtsplib.h"

void read_euc_2d(FILE *f, int **Dist, int n, char tipo) {
	/* TIPOS:
	 * 'N' -> Distancia euclidiana normal
	 * 'C' -> Distancia euclidiana arredondada para cima
	 * 'A' -> Distancia pseudo-eclidiana */

	double **pontos;
	char lixo[LEN];
	int ind;

	pontos = (double**) malloc(sizeof(double*) * n);
	for (int i=0;i<n;i++) pontos[i] = (double*) malloc(sizeof(double)*2);

	while (fscanf(f,"%s",lixo) == 1 && strcmp(lixo,"NODE_COORD_SECTION") != 0);

	// le os pontos
	for (int i=0;i<n;i++) {
		fscanf(f,"%d",&ind);
		fscanf(f,"%lf %lf",&pontos[ind-1][0],&pontos[ind-1][1]);
	}

	// calcula a Distancia entre todos os pontos
	for (int i=0;i<n;i++) {
		Dist(i,i) = 0;
		for (int j=i+1;j<n;j++) {
			if (tipo == 'N') {
				Dist(i,j) = (int)(sqrt( (pontos[i][0]-pontos[j][0])*(pontos[i][0]-pontos[j][0]) + (pontos[i][1]-pontos[j][1])*(pontos[i][1]-pontos[j][1]) ) + 0.5);
			} else if (tipo == 'C') {
				Dist(i,j) = (int)ceil((sqrt( (pontos[i][0]-pontos[j][0])*(pontos[i][0]-pontos[j][0]) + (pontos[i][1]-pontos[j][1])*(pontos[i][1]-pontos[j][1]))));
			} else if (tipo == 'A') {
				double rij = sqrt( (pontos[i][0]-pontos[j][0])*(pontos[i][0]-pontos[j][0]) + (pontos[i][1]-pontos[j][1])*(pontos[i][1]-pontos[j][1]) ) / 10.0;
				int tij = (int)(rij + 0.5);
				Dist(i,j) = (tij < rij)? tij + 1 : tij;
			}
		}
	}

	// desaloca a memoria do vetor de pontos
	for (int i=0;i<n;i++) {
		free(pontos[i]);
	}
	free(pontos);
}

void read_explicit(FILE *f, int **Dist, int n) {
	char lixo[LEN];
	int tipo = 0;
	while (fscanf(f,"%s",lixo) == 1 && strcmp(lixo,"EDGE_WEIGHT_FORMAT") != 0);
	fscanf(f,"%s",lixo);
	fscanf(f,"%s",lixo);

	if (strcmp(lixo,"LOWER_DIAG_ROW") == 0) tipo = 1;

	while (fscanf(f,"%s",lixo) == 1 && strcmp(lixo,"EDGE_WEIGHT_SECTION") != 0);
	if (tipo == 0) {
		for (int i=0;i<n;Dist(i,i)=0,i++)
			for (int j=i+1;j<n;j++)
				fscanf(f,"%d",&(Dist(i,j)));
	} else if (tipo == 1) {
		for (int i=0;i<n;i++)
			for (int j=0;j<=i;j++)
				fscanf(f,"%d",&(Dist(i,j)));
	}
}

void read_geo(FILE *f, int **Dist, int n) {
	double RRR = 6378.388;
	double PI = 2.0 * acos(0);

	char lixo[LEN];
	int aux, deg;
	double a, b, min, *lat, *longit, q1, q2, q3;

	while (fscanf(f,"%s",lixo) == 1 && strcmp(lixo,"NODE_COORD_SECTION") != 0);

	longit = (double*) malloc(sizeof(double)*n);
	lat = (double*) malloc(sizeof(double)*n);

	for (int i=0;i<n;i++) {
		fscanf(f,"%d %lf %lf",&aux,&a,&b);
		//printf("%d %lf %lf\n", aux, a, b);

		//deg = (int)(a + 0.5);
		deg = (int)(a);
		min = a - deg;
		lat[i] = PI * (deg + 5.0 * min / 3.0) / 180.0;

		//deg = (int)(b + 0.5);
		deg = (int)(b);
		min = b - deg;
		longit[i] = PI * (deg + 5.0 * min / 3.0) / 180.0;
	}

	for (int i=0;i<n;i++) {
		for (int j=i+1;j<n;j++) {

			q1 = cos (longit[i] - longit[j]);
			q2 = cos (lat[i] - lat[j]);
			q3 = cos (lat[i] + lat[j]);

			Dist(i,j) = (int) (RRR * acos(0.5*((1.0+q1)*q2 - (1.0-q1)*q3) ) + 1.0);
			//printf("d[%d][%d] = %d\n", i, j, Dist(i, j));
		}
	}

	free(lat);
	free(longit);
}

int read(char *arq, int **&Dist, int &n) {
	char lixo[LEN];
	FILE *f = fopen(arq,"r");

	if(f == NULL){
		return 0;
	}

	/* le tudo até "DIMENSION" e depois le a dimensao */
	while (fscanf(f,"%s",lixo) == 1 && strcmp(lixo,"DIMENSION") != 0);
	fscanf(f,"%s %d",lixo,&n);

	/* aloca a matriz, que deve ser acessada Dist(i,j) */
	Dist = (int**) malloc(sizeof(int*) * n);
	for (int i=0;i<n;i++) Dist[i] = (int*) malloc(sizeof(int) * (i+1));

	while (fscanf(f,"%s",lixo) == 1 && strcmp(lixo,"EDGE_WEIGHT_TYPE") != 0);
	fscanf(f,"%s",lixo);
	fscanf(f,"%s",lixo);

	/* para cada tipo de instancia, chama a função específica */
	if (strcmp(lixo,"EUC_2D") == 0) read_euc_2d(f,Dist,n,'N');
	else if (strcmp(lixo,"EXPLICIT") == 0) read_explicit(f,Dist,n);
	else if (strcmp(lixo,"GEO") == 0) read_geo(f,Dist,n);
	else if (strcmp(lixo,"CEIL_2D") == 0) read_euc_2d(f,Dist,n,'C');
	else if (strcmp(lixo,"ATT") == 0) read_euc_2d(f,Dist,n,'A');

	/* fecha o arquivo */
	fclose(f);

	return 1;
}



#ifndef READTSPLIB_H
#define READTSPLIB_H

#define Max(a,b) (a>b)?a:b
#define Min(a,b) (a>b)?b:a

#define Dist(i,j) Dist[Max(i,j)][Min(i,j)]
#define LEN 200

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

void read_euc_2d(FILE *f, int **dist, char tipo);
void read_explicit(FILE *f, int **dist);
void read_geo(FILE *f, int **dist);
int read(char *arq, int **&dist, int &n);

#endif

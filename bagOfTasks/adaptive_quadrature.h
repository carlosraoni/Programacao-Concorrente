#ifndef ADAPTIVE_QUADRATURE_H
#define ADAPTIVE_QUADRATURE_H

// Arquivo cabeçalho que define as funções utilizadas no método de quadratura adaptativa de cálculo de integral.

#define TOLERANCE 1e-16

// Função definida para os testes do método de quadratura adaptativa.
double foo(double num);

// Função x^2.
double square (double num);

// Função do Marcio e do Bruno
double fmb(double num);

// Função do Leonardo
double fl(double num);

// Determina a área do trapézio definido pelo intervalo [a, b].
double calcTrapezoidArea(double a, double b, double fa, double fb);

// Teste do método de quadratura adaptativa que define se a área do trapézio do intervalo [a,b] está dentro da tolerância
// com a soma dos trapézios [a,m] e [m,b], onde m é o ponto médio do intervalo [a,b]. A função também retorna nos paramêtros
// m e fm, o ponto médio e o valor da função neste ponto respectivamente.
int splitQuadratureTest(double a, double b, double fa, double fb, double * m, double * fm, double * area, double (*f)(double));

// Método de quadratura adaptativa para o cálcula da integral da função f no intervalo [a, b].
double adaptiveQuadrature(double a, double b, double fa, double fb, double (*f)(double));

#endif // ADAPTIVE_QUADRATURE_H

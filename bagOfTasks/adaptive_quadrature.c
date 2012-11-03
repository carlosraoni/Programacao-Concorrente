#include <math.h>
#include <stdlib.h>
#include "adaptive_quadrature.h"

// Função de teste
double foo(double num) {
	int j;
	double acc = 0.0;

  	for (j=0;j<100000*num;j++)
    	acc += exp(sqrt(num*j)*sin(num))/log(j+2.0)*sqrt(j/25)*exp((num*j)*exp(exp(1.0/(j+1.0))));

	return acc;
}

// Função x^2
double square (double num) {
	return num * num;
}

// Determina a área do trapézio definido pelo intervalo [a, b]
inline double calcTrapezoidArea(double a, double b, double fa, double fb){
	return (b - a) * (fa + fb) / 2.0;
}

// Teste do método de quadratura adaptativa que define se a área do trapézio do intervalo [a,b] está dentro da tolerância
int splitQuadratureTest(double a, double b, double fa, double fb,
						 double * m, double * fm, double * area, double (*f)(double)){
	*m = (a + b) / 2.0; // Ponto médio do intervalo [a,b]
	*fm = f(*m); // Valora da função f no ponto médio m

	double larea = calcTrapezoidArea(a, *m, fa, *fm); // Área do trapézio da esquerda [a,m]
	double rarea = calcTrapezoidArea(*m, b, *fm, fb); // Área do trapézio da direita [m,b]
	*area = calcTrapezoidArea(a, b, fa, fb); // Área do trapézio [a,b]

 	// Retorna se a diferença da área do trapézio [a,b] com a soma das áreas dos trapézios da esquerda e da direita
 	// é maior que o valor de tolerância definido.
	return (fabs(*area - (larea + rarea)) > TOLERANCE);
}

// Método de quadratura adaptativa para o cálcula da integral da função f no intervalo [a, b].
double adaptiveQuadrature(double a, double b, double fa, double fb, double (*f)(double)){
	double m, fm, area;

	// Testa se a área do trapézio [a,b] excede a tolerância definida
	if(splitQuadratureTest(a, b, fa, fb, &m, &fm, &area, f)){
		// Em caso afirmativo, refina o cálculo dos trapézios a esquerda e a direita do ponto médio m do intervalo [a,b]
		return adaptiveQuadrature(a, m, fa, fm, f) + adaptiveQuadrature(m, b, fm, fb, f);
	}

	// Caso contrário, retorna a área do trapézio [a,b], pois o mesmo obedece a tolerância de aproximação da integral
	return area;
}

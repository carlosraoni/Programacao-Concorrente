#include <math.h>
#include <stdlib.h>
#include "adaptive_quadrature.h"

double foo(double num) {
	int j;
	double acc = 0.0;

  	for (j=0;j<100000*num;j++)
    	acc += exp(sqrt(num*j)*sin(num))/log(j+2.0)*sqrt(j/25)*exp((num*j)*exp(exp(1.0/(j+1.0))));

	return acc;
}

double square (double num) {
	return num * num;
}

inline double calcTrapezoidArea(double a, double b, double fa, double fb){
	return (b - a) * (fa + fb) / 2.0;
}

int splitQuadratureTest(double a, double b, double fa, double fb,
						 double * m, double * fm, double * area, double (*f)(double)){
	*m = (a + b) / 2.0;
	*fm = f(*m);

	double larea = calcTrapezoidArea(a, *m, fa, *fm);
	double rarea = calcTrapezoidArea(*m, b, *fm, fb);
	*area = calcTrapezoidArea(a, b, fa, fb);

	return (fabs(*area - (larea + rarea)) > TOLERANCE);
}

double adaptiveQuadrature(double a, double b, double fa, double fb, double (*f)(double)){
	double m, fm, area;

	if(splitQuadratureTest(a, b, fa, fb, &m, &fm, &area, f)){
		return adaptiveQuadrature(a, m, fa, fm, f) + adaptiveQuadrature(m, b, fm, fb, f);
	}

	return area;
}

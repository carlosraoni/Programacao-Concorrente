#ifndef ADAPTIVE_QUADRATURE_H
#define ADAPTIVE_QUADRATURE_H

#define TOLERANCE 1e-16

double foo(double num);

double square (double num);

double calcTrapezoidArea(double a, double b, double fa, double fb);

int splitQuadratureTest(double a, double b, double fa, double fb, double * m, double * fm, double * area, double (*f)(double));

double adaptiveQuadrature(double a, double b, double fa, double fb, double (*f)(double));

#endif // ADAPTIVE_QUADRATURE_H

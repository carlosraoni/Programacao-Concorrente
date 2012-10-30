#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define TOLERANCE 1e-16

double f (double num) {
	int j;
	double acc = 0.0;

  	for (j=0;j<10000000*num;j++)
    	acc += exp(sqrt(num*j*sin(num)))/log(j)*sqrt(j/25)*exp((num*j)*exp(exp(1.0/(j+1.0))));

	return acc;
}

double square (double num) {
	return num * num;
}

double calcTrapezoidArea(double a, double b, double (*f)(double)){
	double fa = f(a);
	double fb = f(b);

	return (b - a) * (fa + fb) / 2.0;
}

double trapezoidApproximation(double a, double b, double (*f)(double)){
	double m = (a + b) / 2.0;

	printf("Calculating Areas\n");
	double larea = calcTrapezoidArea(a, m, f);
	double rarea = calcTrapezoidArea(m, b, f);
	double area = calcTrapezoidArea(a, b, f);

	printf("larea = %f \n", larea);
	printf("rarea = %f \n", rarea);
	printf("area = %f \n", area);

	if(fabs(area - (larea + rarea)) >= TOLERANCE){
		printf("Dividing!\n");
		return trapezoidApproximation(a, m, f) + trapezoidApproximation(m, b, f);
	}

	printf("Returning\n");
	return area;
}

int main(){

	printf("Trapezoid Approximation = %f\n", trapezoidApproximation(0.0, 15.0, &f));
	//printf("Trapezoid Approximation = %f\n", trapezoidApproximation(0.0, 15.0, &square));

	return 0;
}

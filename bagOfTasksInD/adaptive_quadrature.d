import std.stdio, std.math, std.string, std.conv;

int NWORKERS = 2; 
int NTASKS = 1000; 
double RANGE_INI = 0.0; 
double RANGE_END = 15.0; 

double square (double num) {
	return num * num;
}

double calcTrapezoidArea(double a, double b, double fa, double fb){
	return (b - a) * (fa + fb) / 2.0;
}

int splitQuadratureTest(double a, double b, double fa, double fb,
						ref double m, ref double fm, ref double area, double function(double) f){
											  
	m = (a + b) / 2.0; 
	fm = f(m); 

	double larea = calcTrapezoidArea(a, m, fa, fm); 
	double rarea = calcTrapezoidArea(m, b, fm, fb); 
	area = calcTrapezoidArea(a, b, fa, fb); 
 	
	return (fabs(area - (larea + rarea)) > 1e-9);
}

double adaptiveQuadrature(double a, double b, double fa, double fb, double function(double) f){
	double m, fm, area;

	if(splitQuadratureTest(a, b, fa, fb, m, fm, area, f)){
		return adaptiveQuadrature(a, m, fa, fm, f) + adaptiveQuadrature(m, b, fm, fb, f);
	}

	return area;
}

void main(string[] args){

	if(args.length != 5){
        writefln("Usage: %s NWORKERS NTASKS RANGE_INI RANGE_END\n", args[0]);
        writefln("Using default values: ");
    }
    else{
        //NWORKERS = atoi(args[1]);
		NWORKERS = to!int(args[1]);
        NTASKS = to!int(args[2]);
        RANGE_INI = to!double(args[3]);
        RANGE_END = to!double(args[4]);
    }

	writefln("NWORKERS = %d", NWORKERS);
    writefln("NTASKS = %d", NTASKS);
    writefln("RANGE_INI = %.2f", RANGE_INI);
    writefln("RANGE_END = %.2f\n", RANGE_END);
	
	writeln("Resultado: ", adaptiveQuadrature(RANGE_INI, RANGE_END, square(RANGE_INI), square(RANGE_END), &square));
}

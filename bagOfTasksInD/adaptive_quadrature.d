import std.stdio, std.math, std.string, std.conv, std.container, std.concurrency;

int NWORKERS = 2;
int NTASKS = 1000;
double RANGE_INI = 0.0;
double RANGE_END = 15.0;

double square (double num) {
	return num * num;
}

double fl(double num){
	double acc = 0.0;
	for(int j = 0; j < 10*num; ++j){
    	double a = sqrt(num*j)*sin(num);
    	double b = log(j+2.0);
    	double c = sqrt(j/25.0);
    	acc += a/b*c;
  	}

  	return acc;
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

	return (fabs(area - (larea + rarea)) > 1e-16);
}

double adaptiveQuadrature(double a, double b, double fa, double fb, double function(double) f){
	double m, fm, area;

	if(splitQuadratureTest(a, b, fa, fb, m, fm, area, f)){
		return adaptiveQuadrature(a, m, fa, fm, f) + adaptiveQuadrature(m, b, fm, fb, f);
	}

	return area;
}

struct Task{
	double a, b, fa, fb;
	bool fValuesAvailable;
}

auto taskQueue = DList!Task();
auto idleQueue = DList!Tid();

void master(){
	double result = 0.0;
	int threadsIdle = NWORKERS;
	int pendingTasks = NTASKS;

	while((NWORKERS - threadsIdle) > 0 || pendingTasks > 0){

		int numTasksToSend = (threadsIdle < pendingTasks) ? threadsIdle: pendingTasks;
		for(int i=0; i<numTasksToSend; i++){
			Task t = taskQueue.front();
			taskQueue.removeFront(); pendingTasks--;

			Tid idleThread = idleQueue.front(); threadsIdle--;
			idleQueue.removeFront();

			idleThread.send(t);
		}

		receive(
			(Task t) { taskQueue.insertBack(t); pendingTasks++;},
			(Tid tid, double partialResult) { result += partialResult; idleQueue.insertBack(tid); threadsIdle++;}
		);
	}

	foreach(Tid tid; idleQueue){
		tid.send("finish");
	}
	idleQueue.clear();

	writefln("Final result: %f", result);
}

void worker(Tid master, double function(double) f){
	while(true){
		Task currentTask, pendingTask;
		bool isFinish = false;
		receive(
			(Task newTask) { currentTask = newTask; },
			(string s){ isFinish = true; }
		);
		if(isFinish)
			break;

		double result = 0.0, a = currentTask.a, b = currentTask.b;
		double fa = (currentTask.fValuesAvailable) ? currentTask.fa : f(a);
		double fb = (currentTask.fValuesAvailable) ? currentTask.fb : f(b);
		double m, fm, area;

		if(splitQuadratureTest(a, b, fa, fb, m, fm, area, f)){
			pendingTask.a = a;
			pendingTask.b = m;
			pendingTask.fa = fa;
			pendingTask.fb = fm;
			pendingTask.fValuesAvailable = true;

			master.send(pendingTask);

			result = adaptiveQuadrature(m, b, fm, fb, f); // Calcula o segundo trapézio e soma na variável local de resultados
		}
		else{
			result = area;
		}

		master.send(thisTid, result);
	}
}

void createTasks(){
	// Tamanho da fatia do intervalo para cada tarefa inicial
	double slice = (RANGE_END - RANGE_INI) / NTASKS;
	double a, b; // Limites do intervalo da tarefa
	int i;

	// Cria as tarefas iniciais e insere as mesmas na fila global de tarefas
	for(i = 0; i < NTASKS; i++){
		a = RANGE_INI + i * slice;
		b = a + slice;

		Task t;
		t.a = a; t.b = b;
		t.fValuesAvailable = false;

		taskQueue.insertFront(t);
	}
}

void createWorkers(){
	for(int i=0; i<NWORKERS; i++){
		//Tid tid = spawn(&worker, thisTid, &square);
		Tid tid = spawn(&worker, thisTid, &fl);
		idleQueue.insertFront(tid);
	}
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

	createTasks();
	createWorkers();
	master();

	//writeln("Resultado: ", adaptiveQuadrature(RANGE_INI, RANGE_END, square(RANGE_INI), square(RANGE_END), &square));
}

import std.stdio, std.math, std.string, std.conv;
import std.container, std.concurrency, core.time;

int NWORKERS = 2; // Número de threads trabalhadoras
int NTASKS = 1000; // Número de tarefas iniciais a serem criadas
double RANGE_INI = 0.0; // Início do intervalo para o qual se deseja calcular a integral da função
double RANGE_END = 15.0; // Fim do intervalo para o qual se deseja calcular a integral da função

const double TOLERANCE = 1e-16; // Tolerância admitida no método de quadratura adaptativa

// Estrutura que define uma tarefa, que representa o cálculo aproximado da integral em um determinado intervalo [a,b]
struct Task{
	double a, b, fa, fb; // Limites do intervalo e valores da função nestes limites
	bool fValuesAvailable; // Indica se fa e fb já foram calculados e portanto se estão disponíveis na tarefa.
}

// Função x*x
double square (double num) {
	return num * num;
}

// Função de teste do Leonardo
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

// Determina a área do trapézio definido pelo intervalo [a, b]
double calcTrapezoidArea(double a, double b, double fa, double fb){
	return (b - a) * (fa + fb) / 2.0;
}

// Teste do método de quadratura adaptativa que define se a área do trapézio do intervalo [a,b] está dentro da tolerância
int splitQuadratureTest(double a, double b, double fa, double fb,
						ref double m, ref double fm, ref double area, double function(double) f){

	m = (a + b) / 2.0; // Ponto médio do intervalo [a,b]
	fm = f(m); // Valora da função f no ponto médio m

	double larea = calcTrapezoidArea(a, m, fa, fm); // Área do trapézio da esquerda [a,m]
	double rarea = calcTrapezoidArea(m, b, fm, fb); // Área do trapézio da direita [m,b]
	area = calcTrapezoidArea(a, b, fa, fb); // Área do trapézio [a,b]

 	// Retorna se a diferença da área do trapézio [a,b] com a soma das áreas dos trapézios da esquerda e da direita
 	// é maior que o valor de tolerância definido.
	return (fabs(area - (larea + rarea)) > TOLERANCE);
}

// Método de quadratura adaptativa para o cálcula da integral da função f no intervalo [a, b].
double adaptiveQuadrature(double a, double b, double fa, double fb, double function(double) f){
	double m, fm, area;

	// Testa se a área do trapézio [a,b] excede a tolerância definida
	if(splitQuadratureTest(a, b, fa, fb, m, fm, area, f)){
		// Em caso afirmativo, refina o cálculo dos trapézios a esquerda e a direita do ponto médio m do intervalo [a,b]
		return adaptiveQuadrature(a, m, fa, fm, f) + adaptiveQuadrature(m, b, fm, fb, f);
	}

	// Caso contrário, retorna a área do trapézio [a,b], pois o mesmo obedece a tolerância de aproximação da integral
	return area;
}

// Função principal das threads trabalhadoras
void worker(Tid master, double function(double) f){
	// Executa em loop requisitando tarefas da thread principal até que a mesma responda que o cálculo terminou
	while(true){
		Task currentTask, pendingTask; // Tarefa atual que está sendo realizada pela thread e tarefa pendente a ser enviada para a thread principal
		bool isFinish = false; // Flag que indica se o cálculo já terminou e portanto a thread pode também terminar
		// Obtém da thread principal uma nova tarefa a ser realizada ou uma mensagem de fim de cálculo
		receive(
			(Task newTask) { currentTask = newTask; },
			(string s){ isFinish = true; }
		);
		// Caso tenha recebido a mensagem de fim, sair do loop e encerrar a thread
		if(isFinish)
			break;

		double result = 0.0; // Resultado da tarefa atual
		double a = currentTask.a; // Início do intervalo da tarefa
		double b = currentTask.b; // Fim do intervalo da tarefa

		// Detemina os valores da função nos limites do intervalo [a,b]
		// Obtém os valores diretamente da tarefa caso disponível ou então calcula os mesmos caso contrário
		double fa = (currentTask.fValuesAvailable) ? currentTask.fa : f(a);
		double fb = (currentTask.fValuesAvailable) ? currentTask.fb : f(b);

		double m, fm, area; // Ponto médio do intervalo [a,b] e valor da função f no mesmo e área do intervalo

		// Testa se a área do trapézio [a,b] está dentro da tolerância com as áreas dos trapézios [a,m] e [m,b]
		if(splitQuadratureTest(a, b, fa, fb, m, fm, area, f)){
			// Caso não esteja na tolerância é preciso refinar o calculo dos trapézios [a,m] e [m,b]
			// Cria uma nova tarefa pendente para cálculo do primeiro trapézio
			pendingTask.a = a;
			pendingTask.b = m;
			pendingTask.fa = fa;
			pendingTask.fb = fm;
			pendingTask.fValuesAvailable = true;
			// Envia tarefa do primeiro trapézio para a thread principal
			master.send(pendingTask);
			// Calcula o segundo trapézio e armazena no resultado da tarefa
			result = adaptiveQuadrature(m, b, fm, fb, f);
		}
		else{
			// Caso esteja na tolerância apenas armazena o resultado da tarefa realizada pela thread
			result = area;
		}
		// Envia o resultado da tarefa realizada pela thread trabalhadora para a thread principal
		master.send(thisTid, result);
	}
}

// Função da thread principal
// Recebe a fila de tarefas iniciais e a lista de threads trabalhadoras
void master(DList!Task taskQueue, DList!Tid idleQueue){
	double result = 0.0; // Armazena o resultado do cálculo
	int threadsIdle = NWORKERS; // Número de threads livres
	int pendingTasks = NTASKS; // Número de tarefas pendentes

	// Executa em loop enquanto existirem threads trabalhadoras ocupadas ou tarefas pendentes
	while((NWORKERS - threadsIdle) > 0 || pendingTasks > 0){
		// Determina o número máximo de tarefas pendentes que podem ser distribuídas para threads livres
		int numTasksToSend = (threadsIdle < pendingTasks) ? threadsIdle: pendingTasks;
		// Distribui o máximo de tarefas pendentes para as threads livres
		for(int i=0; i<numTasksToSend; i++){
			// Obtém uma tarefa pendente do fim da fila de tarefas
			Task t = taskQueue.front();
			taskQueue.removeFront(); pendingTasks--;

			// Obtém uma thread livre da fila de threads livres
			Tid idleThread = idleQueue.front(); threadsIdle--;
			idleQueue.removeFront();

			// Envia tarefa pendente para thread livre
			idleThread.send(t);
		}
		// Aguarda por mensagem de conclusão de tarefa ou por uma nova tarefa pendente
		receive(
			// Caso receba uma mensagem de tarefa pendente, armazena a mesma na fila de tarefas
			(Task t) { taskQueue.insertBack(t); pendingTasks++;},
			// Caso receba uma mensagem de conclusão de tarefa por uma thread trabalhadora, inclui a mesma na fila de threads livres
			(Tid tid, double partialResult) { result += partialResult; idleQueue.insertBack(tid); threadsIdle++;}
		);
	}

	// Quando não tiverem mais threads ocupadas nem tarefas pendentes, comunicar todas as threads
	// trabalhadoras do fim do cálculo
	foreach(Tid tid; idleQueue){
		tid.send("finish");
	}
	idleQueue.clear(); // Limpra fila de threads livres

	writefln("Final result: %f", result); // Exibe resultado final
}

// Cria a fila de tarefas iniciais, que será gerenciada pela thread principal
DList!Task createTasks(){
	auto taskQueue = DList!Task(); // Fila de tarefas

	// Tamanho da fatia do intervalo para cada tarefa inicial
	double slice = (RANGE_END - RANGE_INI) / NTASKS;
	double a, b; // Limites do intervalo da tarefa
	int i;

	// Cria as tarefas iniciais e insere as mesmas na fila de tarefas iniciais
	for(i = 0; i < NTASKS; i++){
		a = RANGE_INI + i * slice;
		b = a + slice;

		Task t;
		t.a = a; t.b = b;
		t.fValuesAvailable = false;

		taskQueue.insertFront(t);
	}

	return taskQueue;
}

// Cria as threads trabalhadoras
DList!Tid createWorkers(){
	// As threads trabalhadoras são inicialmente inseridas em uma fila de threads livres, que será gerenciada pela thread principal
	auto idleQueue = DList!Tid();

	for(int i=0; i<NWORKERS; i++){
		// Cria a thread trabalhadora e inseri na fila de threads livres
		//Tid tid = spawn(&worker, thisTid, &square);
		Tid tid = spawn(&worker, thisTid, &fl);
		idleQueue.insertFront(tid);
	}

	return idleQueue;
}

void main(string[] args){
	// Determina os parâmetros iniciais do programa
	if(args.length != 5){
		writefln("Usage: %s NWORKERS NTASKS RANGE_INI RANGE_END\n", args[0]);
		writefln("Using default values: ");
	}
	else{
		NWORKERS = to!int(args[1]);
		NTASKS = to!int(args[2]);
		RANGE_INI = to!double(args[3]);
		RANGE_END = to!double(args[4]);
	}

	writefln("NWORKERS = %d", NWORKERS);
	writefln("NTASKS = %d", NTASKS);
	writefln("RANGE_INI = %.2f", RANGE_INI);
	writefln("RANGE_END = %.2f\n", RANGE_END);

	DList!Task taskQueue = createTasks(); // Cria a fila de tarefas inicias

	TickDuration tb = TickDuration.currSystemTick(); // Obtém o tick antes da execução do método

	DList!Tid idleQueue = createWorkers(); // Cria as threads trabalhadoras
	master(taskQueue, idleQueue); // Executa função da thread principal

	TickDuration ta = TickDuration.currSystemTick(); // Obtém o tick de depois da execução do método

    long timeMsecs = (ta-tb).msecs(); // Determina o tempo em milisegundos

    // Exibe o tempo total de execução em segundos
	writeln("Total Execution time: ", timeMsecs / 1000, ".", timeMsecs % 1000, "s");

}

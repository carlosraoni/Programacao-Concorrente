import std.stdio, std.concurrency;

void writer(){
	for(;;){
		auto msg = receiveOnly!(Tid, int)();
		writeln("Secondary Thread: ", msg[1]);
		msg[0].send(thisTid);
	}
}

void main(){
	auto l = 0, h = 100;
	auto tid = spawn(&writer);

	foreach(i; l .. h){
		writeln("Main Thread: ", i);
		tid.send(thisTid, i);
		receiveOnly!Tid() == tid;
	}
}



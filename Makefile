problem2a: problem2a.o problem.o
	gcc -Wall -o problem2a problem2a.o problem.o -g

problem2a.o: problem2a.c
	gcc -Wall -o problem2a.o -c problem2a.c -g

problem2b: problem2b.o problem.o
	gcc -Wall -o problem2b problem2b.o problem.o -g

problem2b.o: problem2b.c
	gcc -Wall -o problem2b.o -c problem2b.c -g

problem2e: problem2e.o problem.o
	gcc -Wall -o problem2e problem2e.o problem.o -g

problem2e.o: problem2e.c
	gcc -Wall -o problem2e.o -c problem2e.c -g

problem2f: problem2f.o problem.o
	gcc -Wall -o problem2f problem2f.o problem.o -g

problem2f.o: problem2f.c
	gcc -Wall -o problem2f.o -c problem2f.c -g

problem.o: problem.h problem.c solutionStruct.c problemStruct.c
	gcc -Wall -o problem.o -c problem.c -g

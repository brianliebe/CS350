all: sort generate shared
	gcc shared.o prog1sorter.o -o prog1sorter
	gcc shared.o prog1generator.o -o prog1generator

sort: prog1sorter.c shared.h
	gcc -Wall -c prog1sorter.c

generate: prog1generator.c shared.h
	gcc -Wall -c prog1generator.c

shared: shared.c shared.h
	gcc -Wall -c shared.c

clean:
	rm *.o prog1sorter prog1generator

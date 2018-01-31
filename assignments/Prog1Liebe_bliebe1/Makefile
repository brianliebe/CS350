all: sort generate shared
	gcc shared.o prog1sorter.o -o prog1sorter
	gcc shared.o prog1generator.o -o prog1generator

sort: shared prog1sorter.c shared.h
	gcc -Wall -Wextra -c prog1sorter.c

generate: shared prog1generator.c shared.h
	gcc -Wall -Wextra -c prog1generator.c

shared: shared.c shared.h
	gcc -Wall -Wextra -c shared.c

clean:
	rm -f *.o prog1sorter prog1generator

all: sort generate
	gcc prog1sorter.o -o prog1sorter
	gcc prog1generator.o -o prog1generator

sort: prog1sorter.c
	gcc -Wall -c prog1sorter.c

generate: prog1generator.c
	gcc -Wall -c prog1generator.c

clean:
	rm *.o prog1sorter prog1generator

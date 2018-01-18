#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int max_num = -1;
int min_num = -1;

int main (int argc, char *argv[]) {
	int i;
	int start_of_input = 0;
	for (i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-u") == 0) {
			printf("THIS WILL BE A USAGE STRING\n");
			printf("VALUE IS %s\n", argv[i+1]);
			start_of_input = i + 1;
		}
		if (strcmp(argv[i], "-n") == 0) {
			// this will sort exactly <num-ints> integers
			start_of_input = i + 1;
		}
		if (strcmp(argv[i], "-m") == 0) {
			min_num = atoi(argv[i+1]);
			start_of_input = i + 1;
		}
		if (strcmp(argv[i], "-M") == 0) {
			max_num = atoi(argv[i+1]);
			start_of_input = i + 1;
		}
	}
	// Now start_of_input should be exactly at the beginning of # flow

}

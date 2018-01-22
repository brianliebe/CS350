#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

int num_ints = 5;
int min_int = 1;
int max_int = 255;
unsigned long seed;
int useSeed = 0;
char *output_file = "";

int main (int argc, char *argv[]) {
	int i;
	seed = time(NULL);
	FILE *out = stdout;

	for (i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-u") == 0) {
			fprintf(stderr, "USAGE:\tprog1generator [-u] [-n <num-integers>] [-m <min-int>] [-M <max-int>]\n\t[-s <seed>] [-o <output-file-name>]\n");
			return 0;
		}
		if (strcmp(argv[i], "-n") == 0) {
			num_ints = atoi(argv[i+1]);
			if (num_ints < 0 || num_ints > 1000000) {
				fprintf(stderr, "Error: Value for -n argument must be between 0 and 1,000,000 (value was '%s').\n", argv[i+1]);
				return 0;
			}
		}
		if (strcmp(argv[i], "-m") == 0) {
			min_int = atoi(argv[i+1]);
			if (min_int < 1) {
				fprintf(stderr, "Error: Value for -m must be at least 1 (value was '%s').\n", argv[i+1]);
				return 0;
			}
		}
		if (strcmp(argv[i], "-M") == 0) {
			max_int = atoi(argv[i+1]);
			if (max_int > 1000000) {
				fprintf(stderr, "Error: Value for -M must be less than 1,000,000 (value was '%s').\n", argv[i+1]);
				return 0;
			}
		}
		if (strcmp(argv[i], "-s") == 0) {
			seed = (unsigned long)atoi(argv[i+1]);
			useSeed = 1;
		}
		if (strcmp(argv[i], "-o") == 0) {
			output_file = argv[i+1];
			out = fopen(output_file, "w");
		}
	}

	fprintf(out, "%d\n", num_ints);
	if (useSeed) srand(seed);
	else srand(time(NULL));
	while (num_ints > 0) {
		fprintf(out, "%d\n", rand() % (max_int + 1 - min_int) + min_int);
		num_ints--;
	}
}

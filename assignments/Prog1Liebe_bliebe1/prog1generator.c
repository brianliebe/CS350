#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "shared.h"

int main (int argc, char *argv[]) {
	FILE *out = stdout;

	struct parsed_arguments args = check_arguments(argc, argv, "GENERATE");
	if (args.error_found == 1) return 0;

	if (strcmp(args.output_file, "") != 0) {
		out = fopen(args.output_file, "w");
	}

	fprintf(out, "%d\n", args.num_ints);
	if (args.useSeed) srand(args.seed);
	else srand(time(NULL));
	while (args.num_ints > 0) {
		fprintf(out, "%d\n", rand() % (args.max_int + 1 - args.min_int) + args.min_int);
		args.num_ints--;
	}
}

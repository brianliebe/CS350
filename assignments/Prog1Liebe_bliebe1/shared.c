#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "shared.h"

struct parsed_arguments check_arguments(int argc, char **argv, char *name) {
	struct parsed_arguments args;
	args = (struct parsed_arguments) {.max_int = 255, .min_int = 1, .num_ints = 100, .output_file = "",
		.input_file = "", .count_file = "", .error_found = 0, .seed = 0, .useSeed = 0};
	int i;

	// Read arguments
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-u") == 0) {
			if (strcmp(name, "SORT") == 0) {
				fprintf(stderr, "USAGE:\tprog1sorter [-u] [-n <num-integers>] [-m <min-int>] [-M <max-int>]\n\t[-i <input-file-name>] [-o <output-file-name>] [-c <count-file-name>]\n");
			}
			else {
				fprintf(stderr, "USAGE:\tprog1generator [-u] [-n <num-integers>] [-m <min-int>] [-M <max-int>]\n\t[-s <seed>] [-o <output-file-name>]\n");
			}
			args.error_found = 1;
			return args;
			i--;
		}
		if (strcmp(argv[i], "-n") == 0) {
			int num_ints = atoi(argv[i+1]);
			if (!isdigit(argv[i+1][0]) || num_ints < 0 || num_ints > 1000000) {
				fprintf(stderr, "Error: Value for -n argument must be between 0 and 1,000,000 (value was '%s').\n", argv[i+1]);
				args.error_found = 1;
				return args;
			}
			args.num_ints = num_ints;
		}
		if (strcmp(argv[i], "-m") == 0) {
			int min_int = atoi(argv[i+1]);
			if (!isdigit(argv[i+1][0]) || min_int < 1) {
				fprintf(stderr, "Error: Value for -m must be at least 1 (value was '%s').\n", argv[i+1]);
				args.error_found = 1;
				return args;
			}
			args.min_int = min_int;
		}
		if (strcmp(argv[i], "-M") == 0) {
			int max_int = atoi(argv[i+1]);
			if (!isdigit(argv[i+1][0]) || max_int > 1000000) {
				fprintf(stderr, "Error: Value for -M must be less than 1,000,000 (value was '%s').\n", argv[i+1]);
				args.error_found = 1;
				return args;
			}
			args.max_int = max_int;
		}
		if (strcmp(argv[i], "-i") == 0) {
			args.input_file = argv[i+1];
			if (args.input_file[0] == '-') {
				if (strcmp(name, "SORT") == 0) {
					fprintf(stderr, "USAGE:\tprog1sorter [-u] [-n <num-integers>] [-m <min-int>] [-M <max-int>]\n\t[-i <input-file-name>] [-o <output-file-name>] [-c <count-file-name>]\n");
				}
				else {
					fprintf(stderr, "USAGE:\tprog1generator [-u] [-n <num-integers>] [-m <min-int>] [-M <max-int>]\n\t[-s <seed>] [-o <output-file-name>]\n");
				}
				args.error_found = 1;
				return args;
			}
		}
		if (strcmp(argv[i], "-o") == 0) {
			args.output_file = argv[i+1];
			if (args.output_file[0] == '-') {
				if (strcmp(name, "SORT") == 0) {
					fprintf(stderr, "USAGE:\tprog1sorter [-u] [-n <num-integers>] [-m <min-int>] [-M <max-int>]\n\t[-i <input-file-name>] [-o <output-file-name>] [-c <count-file-name>]\n");
				}
				else {
					fprintf(stderr, "USAGE:\tprog1generator [-u] [-n <num-integers>] [-m <min-int>] [-M <max-int>]\n\t[-s <seed>] [-o <output-file-name>]\n");
				}
				args.error_found = 1;
				return args;
			}
		}
		if (strcmp(argv[i], "-c") == 0) {
			args.count_file = argv[i+1];
			if (args.count_file[0] == '-') {
				if (strcmp(name, "SORT") == 0) {
					fprintf(stderr, "USAGE:\tprog1sorter [-u] [-n <num-integers>] [-m <min-int>] [-M <max-int>]\n\t[-i <input-file-name>] [-o <output-file-name>] [-c <count-file-name>]\n");
				}
				else {
					fprintf(stderr, "USAGE:\tprog1generator [-u] [-n <num-integers>] [-m <min-int>] [-M <max-int>]\n\t[-s <seed>] [-o <output-file-name>]\n");
				}
				args.error_found = 1;
				return args;
			}
		}
		if (strcmp(argv[i], "-s") == 0) {
			args.seed = strtoul(argv[i+1], NULL, 0);
			args.useSeed = 1;
		}
		if (argv[i][0] == '-') {
			char argch = argv[i][1];
			if (argch == 'u' || argch == 'n' || argch == 'm' || argch == 'M' || argch == 'i' || argch == 'o' || argch == 'c' || argch == 's') {
				// This is fine, this means it's valid
			}
			else {
				if (strcmp(name, "SORT") == 0) {
					fprintf(stderr, "USAGE:\tprog1sorter [-u] [-n <num-integers>] [-m <min-int>] [-M <max-int>]\n\t[-i <input-file-name>] [-o <output-file-name>] [-c <count-file-name>]\n");
				}
				else {
					fprintf(stderr, "USAGE:\tprog1generator [-u] [-n <num-integers>] [-m <min-int>] [-M <max-int>]\n\t[-s <seed>] [-o <output-file-name>]\n");
				}
				args.error_found = 1;
				return args;
			}
		}
		i++;
	}
	return args;
}

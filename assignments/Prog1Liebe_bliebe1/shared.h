#ifndef SHARED_H
#define SHARED_H

struct parsed_arguments {
	// The possible values to get from the arguments (for both programs)
	int max_int;
	int min_int;
	int num_ints;
	char *output_file;
	char *input_file;
	char *count_file;
	int error_found;
	unsigned long seed;
	int useSeed;
};

// The only shared function for both programs
struct parsed_arguments check_arguments(int argc, char **argv, char *name);

#endif

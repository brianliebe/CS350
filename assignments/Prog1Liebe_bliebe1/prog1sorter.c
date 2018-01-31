#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "shared.h"

// For use with qsort, sorts the values
int comparator (const void *p, const void *q)
{
	int val1 = *(const int *)p;
	int val2 = *(const int *)q;
	if (val1 < val2) return 0;
	else return 1;
}

// Prints out an array using the either stdout or a file, based on "out"
void printArr(int arr[], int n, FILE* out)
{
	int i;
	for (i = 0; i < n; ++i) fprintf(out, "%d\n", arr[i]);
}

int main (int argc, char *argv[]) {
	int i, j;
	char *username = (char *)getlogin(); // Get linux username
	FILE* s_out = stdout;
	FILE* c_out = stdout;
	char *line = NULL;
	size_t size;

	// Parse the arguments using shared.c/.h and return if there's an error
	struct parsed_arguments args = check_arguments(argc, argv, "SORT");
	if (args.error_found == 1) return 0;

	// Start the clock
	clock_t t;
	t = clock();

	int max_int = args.max_int;
	int min_int = args.min_int;
	int num_ints = args.num_ints;
	char *input_file = args.input_file;
	char *output_file = args.output_file;
	char *count_file = args.count_file;

	// Check for irregularity between min and max
	if (max_int < min_int) {
		fprintf(stderr, "Error: Issue with -m and -M (max smaller than min).\n");
		return 0;
	}

	// Set the output file if one is specified
	if (strcmp(output_file, "") != 0) {
		s_out = fopen(output_file, "w");
	}
	// Set the count output if one is specified
	if (strcmp(count_file, "") != 0) {
		c_out = fopen(count_file, "w");
	}

	int array_length = -1;
	int starting_value = -1;
	int *unsorted = NULL;

	if (strcmp(input_file, "") == 0) {
		// No input file specified, so we use stdin
		array_length = 0;

		// The first line will be the number of total integers, so read that first
		if (getline(&line, &size, stdin) != 0) {
			// Allocate unsorted based on the first value
			starting_value = atoi(line);
			unsorted = (int *)malloc(starting_value * sizeof *unsorted);
		}
		else {
			// If there's an error with stdin, then we just stop
			fprintf(stderr, "Error: No stdin and no input file.\n");
			return 0;
		}
		// Read the remainder of the lines, and add it to the allocated unsorted array
		while (getline(&line, &size, stdin) != -1) {
			unsorted[array_length++] = atoi(line);
		}
	}
	else {
		// Read from the input file
		FILE* file = fopen(input_file, "r");
		char *line = malloc(256 * sizeof *line);
		int firstLine = 1;
		// Read each line to get total size
		while (fgets(line, sizeof(line), file)) {
			line[strlen(line) - 1] = 0;
			if (firstLine) {
				starting_value = atoi(line);
				firstLine = 0;
			}
			array_length++;
		}
		fclose(file);
		free(line);
	}

	// Check to make sure first number is correct
	if (array_length != starting_value) {
		fprintf(stderr, "Error: Total number of integers does not match the expected amount.\n");
		return 0;
	}

	// If -n was specified, make sure only the first -n values are sorted
	if (array_length > num_ints) {
		array_length = num_ints;
	}

	// Check to make sure there are numbers to sort
	if (array_length < 1) {
		fprintf(stdout, "Error: Number flow is not long enough (%d)\n", array_length);
		return 0;
	}

	// Create an array for the valid values and populate it
	int *numbers = malloc(array_length * sizeof *numbers);
	if (strcmp(input_file, "") == 0) {
		// Just add the numbers in "unsorted" to "numbers", for simplicity
		for (i = 0; i < array_length; i++) {
			numbers[i] = unsorted[i];
			// Check for bounds
			if (numbers[i] < min_int) {
				fprintf(stderr, "Error: A value was found that was less than the minimum (set by -m).\n");
				return 0;
			}
			if (numbers[i] > max_int) {
				fprintf(stderr, "Error: A value was found that was more than the maximum (set by -M).\n");
				return 0;
			}
		}
	}
	else {
		// Read the input file
		FILE* file = fopen(input_file, "r");
		char *line = malloc(256 * sizeof *line);
		int firstLine = 1;
		i = 0;
		while (fgets(line, sizeof(line), file) && array_length > i) {
			line[strlen(line) - 1] = 0;
			if (firstLine) {
				// Skip first line
				firstLine = 0;
			}
			else {
				// Check for bounds
				numbers[i] = atoi(line);
				if (numbers[i] < min_int) {
					fprintf(stderr, "Error: A value was found that was less than the minimum (set by -m).\n");
					return 0;
				}
				if (numbers[i] > max_int) {
					fprintf(stderr, "Error: A value was found that was more than the maximum (set by -M).\n");
					return 0;
				}
				i++;
			}
		}
		fclose(file);
		free(line);
	}

	// Sort the array with qsort
	qsort((void*)numbers, array_length, sizeof(numbers[0]), comparator);
	printArr(numbers, array_length, s_out);

	// Go through the username
	for (i = 0; i < (int)strlen(username); i++) {
		int c = username[i];
		int count = 0;
		for (j = 0; j < array_length; j++) {
			if (numbers[j] == c) count++;
		}
		fprintf(c_out, "%c\t%d\t%d\n",c,c,count);
	}

	// Print the clock to stderr
	t = clock() - t;
	double time_taken = ((double)t)/CLOCKS_PER_SEC;
	fprintf(stderr, "%f\n", time_taken);
	free(numbers);
}

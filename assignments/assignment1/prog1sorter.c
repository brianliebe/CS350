#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

int max_int = 255;
int min_int = 1;
int num_ints = 100;
char *input_file = "";
char *output_file = "";
char *count_file = "";

int comparator (const void *p, const void *q)
{
    int val1 = *(const int *)p;
    int val2 = *(const int *)q;
	if (val1 < val2) return 0;
	else return 1;
}

void printArr(int arr[], int n, FILE* out)
{
    int i;
    for (i = 0; i < n; ++i) fprintf(out, "%d\n", arr[i]);
}

int main (int argc, char *argv[]) {
	clock_t t;
	t = clock();
	int i, j;
	char *username = (char *)getlogin();
	int start_of_input = 1;
	FILE* s_out = stdout;
	FILE* c_out = stdout;

	// Read arguments 
	for (i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-u") == 0) {
			fprintf(stderr, "USAGE:\tprog1sorter [-u] [-n <num-integers>] [-m <min-int>] [-M <max-int>]\n\t[-i <input-file-name>] [-o <output-file-name>] [-c <count-file-name>]\n");
			return 0;
		}
		if (strcmp(argv[i], "-n") == 0) {
			num_ints = atoi(argv[i+1]);
			if (num_ints < 0 || num_ints > 1000000) {
				fprintf(stderr, "Error: Value for -n argument must be between 0 and 1,000,000 (value was '%s').\n", argv[i+1]);
				return 0;
			}
			start_of_input = i++ + 2;
		}
		if (strcmp(argv[i], "-m") == 0) {
			min_int = atoi(argv[i+1]);
			if (min_int < 1) {
				fprintf(stderr, "Error: Value for -m must be at least 1 (value was '%s').\n", argv[i+1]);
				return 0;
			}
			start_of_input = i++ + 2;
		}
		if (strcmp(argv[i], "-M") == 0) {
			max_int = atoi(argv[i+1]);
			if (max_int > 1000000) {
				fprintf(stderr, "Error: Value for -M must be less than 1,000,000 (value was '%s').\n", argv[i+1]);
				return 0;
			}
			start_of_input = i++ + 2;
		}
		if (strcmp(argv[i], "-i") == 0) {
			input_file = argv[i+1];
			start_of_input = i++ + 2;
		}
		if (strcmp(argv[i], "-o") == 0) {
			output_file = argv[i+1];
			start_of_input = i++ + 2;
		}
		if (strcmp(argv[i], "-c") == 0) {
			count_file = argv[i+1];
			start_of_input = i++ + 2;
		}
	}

	// Check for irregularity between min and max
	if (max_int < min_int) {
		fprintf(stderr, "Error: Issue with -m and -M (max smaller than min).\n");
		return 0;
	}

	if (strcmp(output_file, "") != 0) {
		s_out = fopen(output_file, "w");
	}
	if (strcmp(count_file, "") != 0) {
		c_out = fopen(count_file, "w");
	}

	int array_length = -1;
	int starting_value = -1;

	if (strcmp(input_file, "") == 0) {
		// No input file specified, so we use stdin
		array_length = argc - start_of_input - 1;
		starting_value = atoi(argv[start_of_input]);

		// Check to make sure first number is correct
		if (array_length != starting_value) {
			fprintf(stderr, "Error: Total number of values does not match the expected number.\n");
			return 0;
		}
	}
	else {
		FILE* file = fopen(input_file, "r");
		char line[256];
		int firstLine = 1;
		while (fgets(line, sizeof(line), file)) {
			line[strlen(line) - 1] = 0;
			if (firstLine) {
				starting_value = atoi(line);
				firstLine = 0;
			}
			array_length++;
		}
		fclose(file);
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
	int numbers[array_length];
	if (strcmp(input_file, "") == 0) {
		for (i = 0, j = start_of_input + 1; j < start_of_input + 1 + array_length; i++, j++) {
			numbers[i] = atoi(argv[j]);
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
		FILE* file = fopen(input_file, "r");
		char line[256];
		int firstLine = 1;
		i = 0;
		while (fgets(line, sizeof(line), file)) {
			line[strlen(line) - 1] = 0;
			if (firstLine) {
				firstLine = 0;
			}
			else {
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
	}
	
	// Sort the array with qsort
    qsort((void*)numbers, array_length, sizeof(numbers[0]), comparator);
	printArr(numbers, array_length, s_out);

	for (i = 0; i < strlen(username); i++) {
		int c = username[i];
		int count = 0;
		for (j = 0; j < array_length; j++) {
			if (numbers[j] == c) count++;
		}
		fprintf(c_out, "%c\t%d\t%d\n",c,c,count);
	}


	t = clock() - t;
	double time_taken = ((double)t)/CLOCKS_PER_SEC;
	fprintf(stderr, "%f\n", time_taken);
}

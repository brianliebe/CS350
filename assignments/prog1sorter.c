#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

int max_int = -1;
int min_int = -1;
int num_ints = -1;
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

void printArr(int arr[], int n)
{
    int i;
    for (i = 0; i < n; ++i) fprintf(stdout, "%d\n", arr[i]);
}

int main (int argc, char *argv[]) {
	clock_t t;
	t = clock();
	int i, j;
	char *username = (char *)getlogin();
	int start_of_input = 1;

	// Read arguments 
	for (i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-u") == 0) {
			printf("THIS WILL BE A USAGE STRING\n");
			return 0;
		}
		if (strcmp(argv[i], "-n") == 0) {
			num_ints = atoi(argv[i+1]);
			start_of_input = i++ + 2;
		}
		if (strcmp(argv[i], "-m") == 0) {
			min_int = atoi(argv[i+1]);
			start_of_input = i++ + 2;
		}
		if (strcmp(argv[i], "-M") == 0) {
			max_int = atoi(argv[i+1]);
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

	// Now start_of_input should be exactly at the beginning of # flow
	printf("Index value: %d, total: %d, and actual value: %s\n", start_of_input, argc, argv[start_of_input]);

	int array_length = argc - start_of_input;
	int numbers[array_length];
	for (i = 0, j = start_of_input; j < argc; i++, j++) {
		numbers[i] = atoi(argv[j]);
	}
    qsort((void*)numbers, array_length, sizeof(numbers[0]), comparator);
	printArr(numbers, array_length);

	for (i = 0; i < strlen(username); i++) {
		int c = username[i];
		int count = 0;
		for (j = 0; j < array_length; j++) {
			if (numbers[j] == c) count++;
		}
		fprintf(stdout, "%c\t%d\t%d\n",c,c,count);
	}


	t = clock() - t;
	double time_taken = ((double)t)/CLOCKS_PER_SEC;
	fprintf(stderr, "%f\n", time_taken);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
				2
		1				1

*/

int getpid();
int getppid();
pid_t fork();

void build_tree(int num_levels, int num_children, int pause, int sleep_time) {
	int current_level = num_levels - 1;
	int i;
	fprintf(stdout, "ALIVE:\t\tLevel %d process with pid=%d, child of ppid=%d\n", current_level, getpid(), getppid());
	/*
	while (num_levels > 0) {
		num_levels--;
		printf("children: %d\n", num_children);
		for (i = 0; i < num_children; i++) {
			pid_t pid = fork();
			if (pid < 0) {
				fprintf(stderr, "Error: Forking failed\n");
				return;
			}
			else if (pid == 0) {
				// Child!
				i = num_children;
				current_level--;
			}
			else {
				// Parent!
				num_levels = 0;
			}
		}
	}
	*/
	int make_children = 1;
	while (current_level > 0 && make_children) {
		fprintf(stdout, "ALIVE:\t\tLevel %d process with pid=%d, child of ppid=%d\n", current_level, getpid(), getppid());
		for (i = 0; i < num_children; i++) {
			pid_t pid = fork();
			if (pid < 0) {
				fprintf(stderr, "Error: Forking failed\n");
				return 0;
			}
			else if (pid == 0) {
				// Child
				current_level--;
				i = num_children;
			}
			else {
				// Parent
				make_children = 0;
			}
		}
	}
	fprintf(stdout, "EXITING:\tLevel %d process with pid=%d, child of ppid=%d\n", current_level, getpid(), getppid());
}

int main (int argc, char **argv) {
	int i;
	int num_levels = 0;
	int num_children = 1;
	int pause = 0;
	int sleep_time = 1;
	int sleep_time_specified = 0;

	char *usage_string = "USAGE:\t./prog2tree [-u] [-N <num-levels>] [-M <num-children>] [-p] [-s <sleep-time]\n";
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-u") == 0) {
			fprintf(stderr, "%s", usage_string);
			return 0;
		}
		else if (strcmp(argv[i], "-N") == 0) {
			if (i + 1 < argc) {
				num_levels = atoi(argv[i+1]);
				i++;
				if (num_levels > 4) {
					fprintf(stderr, "Error: Number of levels must be at most 4\n");
					return 0;
				}
			}
			else {
				fprintf(stderr, "Error: No value for -N\n");
				return 0;
			}
		}
		else if (strcmp(argv[i], "-M") == 0) {
			if (i + 1 < argc) {
				num_children = atoi(argv[i+1]);
				i++;
				if (num_children > 3) {
					fprintf(stderr, "Error: Number of children must be at most 3\n");
					return 0;
				}
			}
			else {
				fprintf(stderr, "Error: No value for -M\n");
				return 0;
			}
		}
		else if (strcmp(argv[i], "-p") == 0) {
			pause = 1;
		}
		else if (strcmp(argv[i], "-s") == 0) {
			sleep_time_specified = 1;
			if (i + 1 < argc) {
				sleep_time = atoi(argv[i+1]);
				i++;
			}
			else {
				fprintf(stderr, "Error: No value for -s\n");
				return 0;
			}
		}
		else {
			fprintf(stderr, "Error: Unknown argument '%s'\n", argv[i]);
			fprintf(stderr, "%s", usage_string);
			return 0;
		}
	}
	if (sleep_time_specified == 1 && pause == 1) {
		fprintf(stderr, "Error: -p and -s both specified\n");
		return 0;
	}
	build_tree(num_levels, num_children, pause, sleep_time);
	return 0;
}

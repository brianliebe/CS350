#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int getpid();
int getppid();
pid_t fork();

void build_tree(int num_levels, int num_children, int pause, int sleep_time) {
	int i;
	int current_level = num_levels - 1;

	char children_string[50];
	char level_string[50];
	char sleep_string[50];

	sprintf(children_string, "%d", num_children);
	sprintf(level_string, "%d", num_levels - 1);
	sprintf(sleep_string, "%d", sleep_time);

	char *command = "./prog2tree";
	char *args[8];
	args[0] = command;
	args[1] = "-N";
	args[2] = level_string;
	args[3] = "-M";
	args[4] = children_string;
	args[5] = "-s";
	args[6] = sleep_string;
	args[7] = NULL;

	// char *command[] = {"./prog2tree", "-N", children_string, "-M", level_string };
	
	fprintf(stdout, "ALIVE:\t\tLevel %d process with pid=%d, child of ppid=%d\n", current_level, getpid(), getppid());
	if (num_levels > 1) {
		for (i = 0; i < num_children; i++) {
			pid_t pid = fork();
			if (pid < 0) {
				fprintf(stderr, "Error: Forking failed\n");
				return;
			}
			else if (pid == 0) {
				// Child
				current_level--;
				execvp(command, args);
				printf("THIS SHOULD NOT RUN\n");
				break;
			}
			else {
				// Parent
			}
		}
	}
	else {
		// LEAF PROCESS
		sleep(sleep_time);
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
	if (pause == 1) {
		sleep_time = 0;
	}
	build_tree(num_levels, num_children, pause, sleep_time);
	return 0;
}

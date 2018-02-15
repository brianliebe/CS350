#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

// int shm_open(const char *, int, mode_t);

pid_t fork();
int getpid();

int main (int argc, char **argv) {
	int i;
	if (argc != 2) {
		printf("Usage: ./prog3ipc <num-procs>\n");
		return 0;
	}
	int procs = atoi(argv[1]);
	if (procs < 1 || procs > 32) {
		printf("Number of process IDs must be between 1 and 32 inclusive.\n");
		return 0;
	}

	// Create shared memory
	const int SIZE = procs * sizeof(int);
	const char *NAME = "bliebe1";
	int shm_fd = shm_open(NAME, O_CREAT | O_RDWR, 0666);
	ftruncate(shm_fd, SIZE);
	void *ptr = mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);

	// Create unnamed pipe for passing process number (not pid)
	pid_t pid;
	int process_number = 0;
	int help = 30;

	while (help > 0) {
		// Create new unnamed pipe
		int pfds[2];
		if (pipe(pfds) == -1) {
			printf("Piping error.\n");
			return 0;
		}
		int temp_num = process_number + 1;

		// Fork
		pid = fork();

		if (pid < 0) {
			printf("Error forking.\n");
			return 0;
		}
		else if (pid == 0) {
			// Child
			close(pfds[1]);
			if (read(pfds[0], &process_number, sizeof(process_number)) <= 0) {
				printf("Error reading from pipe.\n");
				return 0;
			}
			if (process_number == procs - 1) break;
		}
		else {
			// Parent
			close(pfds[0]);
			if (write(pfds[1], &temp_num, sizeof(temp_num)) <= 0) {
				printf("Error writing to pipe\n");
				return 0;
			}
			break;
		}
		help--;
	}
	
	printf("My PID is %d and my number is %d.\n", getpid(), process_number);
	void *relative_ptr = ptr + (process_number * sizeof(int));
	sprintf(relative_ptr, "%d", getpid());

	shm_fd = shm_open(NAME, O_RDONLY, 0666);
	ptr = mmap(0, SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);

	if (process_number == 0) {
		for (i = 0; i < procs; i++) {
			printf("%s", (char *)ptr);
			ptr += sizeof(int);
		}
	}
	
	shm_unlink(NAME);
	return 0;
}

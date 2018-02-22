#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/signal.h>
#include <wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

pid_t fork();
int getpid();

void signal_handler(int sig) {
	printf("Child reached handler\n");
	exit(0);
}

int main (int argc, char **argv) {
	int i, fd;
	pid_t pid;
	int process_number = 0;

	if (argc != 2) {
		printf("Usage: ./prog3ipc <num-procs>\n");
		return 0;
	}
	int procs = atoi(argv[1]);
	if (procs < 1 || procs > 32) {
		printf("Number of process IDs must be between 1 and 32 inclusive.\n");
		return 0;
	}
	// Create named pipe
	char *myfifo = "/tmp/myfifo";
	mkfifo(myfifo, 0666);
	char buf[5];

	// Create shared memory
	const int SIZE = procs * sizeof(int);
	const char *NAME = "bliebe1";
	int shm_fd = shm_open(NAME, O_CREAT | O_RDWR, 0666);
	ftruncate(shm_fd, SIZE);
	void *ptr = mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);

	// Top-level process writes into memory first
	printf("My PID is %d and my number is %d.\n", getpid(), process_number);
	void *relative_ptr = ptr + (process_number * sizeof(int));
	sprintf(relative_ptr, "%d", getpid());

	while (1) {
		// Create unnamed pipe for passing process number (not pid)
		int pfds[2];
		if (pipe(pfds) == -1) {
			printf("Piping error.\n");
			return 0;
		}

		// Fork
		pid = fork();
		if (pid < 0) {
			printf("Error forking.\n");
			return 0;
		}
		else if (pid == 0) {
			// Child, first read from unnamed pipe then write to shared memory
			close(pfds[1]);
			if (read(pfds[0], &process_number, sizeof(process_number)) <= 0) {
				printf("Error reading from pipe.\n");
				return 0;
			}

			printf("My PID is %d and my number is %d.\n", getpid(), process_number);
			void *relative_ptr = ptr + (process_number * sizeof(int));
			sprintf(relative_ptr, "%d", getpid());

			if (process_number == procs - 1) {
				sleep(2);
				fd = open(myfifo, O_WRONLY);
				write(fd, "done", sizeof("done"));
				close(fd);
				break;
			}
		}
		else {
			// Parent, write into the unnamed pipe
			close(pfds[0]);
			int temp_num = process_number + 1;
			if (write(pfds[1], &temp_num, sizeof(temp_num)) <= 0) {
				printf("Error writing to pipe\n");
				return 0;
			}
			if (process_number == 0) {
				fd = open(myfifo, O_RDONLY);
				read(fd, buf, 5);
				close(fd);
			}
			break;
		}
	}
	if (process_number != 0) {
		signal(SIGUSR1, signal_handler);
		pause();
	}
	else {
		shm_fd = shm_open(NAME, O_RDONLY, 0666);
		ptr = mmap(0, SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
		if (process_number == 0) {
			for (i = 0; i < procs; i++) {
				// printf("%s\n", (char *)ptr);
				printf("%.*s\n", 5, (char *)ptr);
				ptr += sizeof(int);
			}
		}
		// Kill not working...
		kill(pid, SIGUSR1);
	}

	shm_unlink(NAME);
	unlink(myfifo);
	return 0;
}

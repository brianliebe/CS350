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
#include <errno.h>

pid_t fork();
int getpid();

int process_number;

void signal_handler(int sig) {
	// wait(NULL);
	printf("EXITING: Level %d process with pid=%d, child of ppid=%d.\n", process_number, getpid(), getppid());
	exit(0);
}

int main (int argc, char **argv) {
	int i, fd, procs;
	pid_t pid;

	// Check arguments for validity
	if (argc != 2) {
		printf("Usage: ./prog3ipc <num-procs>\n");
		return 0;
	}
	procs = atoi(argv[1]);
	if (procs < 1 || procs > 32) {
		printf("Number of process IDs must be between 1 and 32 inclusive.\n");
		return 0;
	}
	process_number = procs;

	// Create named pipe, naming it "brianfifo"
	char *myfifo = "/tmp/brianfifo";
	mkfifo(myfifo, 0666);
	char buf[5];

	// Create shared memory, named "bliebe1"
	const int SIZE = procs * sizeof(int);
	const char *NAME = "bliebe1";
	int shm_fd = shm_open(NAME, O_CREAT | O_RDWR, 0666);
	ftruncate(shm_fd, SIZE);
	void *ptr = mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);

	// Top-level process writes into memory first
	void *relative_ptr = ptr + ((procs - process_number) * sizeof(int));
	int process_id = getpid();
	memcpy(relative_ptr, &process_id, sizeof(process_id));
	printf("ALIVE: Level %d process with pid=%d, child of ppid=%d.\n", process_number, getpid(), getppid());

	while (1) {
		// Create unnamed pipe for passing assigned process number
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
			// Child

			// First read from unnamed pipe and place into process_number integer
			close(pfds[1]);
			if (read(pfds[0], &process_number, sizeof(process_number)) <= 0) {
				printf("Error reading from pipe.\n");
				return 0;
			}

			// Write to shared memory
			printf("ALIVE: Level %d process with pid=%d, child of ppid=%d.\n", process_number, getpid(), getppid());
			void *relative_ptr = ptr + ((procs - process_number) * sizeof(int));
			process_id = getpid();
			memcpy(relative_ptr, &process_id, sizeof(process_id));
			
			// If it's the last process created, write "done" to named pipe and break.
			if (process_number == 1) {
				fd = open(myfifo, O_WRONLY);
				write(fd, "done", sizeof("done"));
				close(fd);
				break;
			}
		}
		else {
			// Parent

			// Write into the unnamed pipe for the child
			close(pfds[0]);
			int temp_num = process_number - 1;
			if (write(pfds[1], &temp_num, sizeof(temp_num)) <= 0) {
				printf("Error writing to pipe\n");
				return 0;
			}
			break;
		}
	}
	if (process_number != procs) {
		// This means it's any process except the first, then we should signal and pause
		signal(SIGUSR1, signal_handler);
		pause();
	}
	else {
		// If it's the first process created, read "done" from the last process
		fd = open(myfifo, O_RDONLY);
		read(fd, buf, sizeof("done"));
		close(fd);

		// Set the shared memory to be read
		shm_fd = shm_open(NAME, O_RDONLY, 0666);
		ptr = mmap(0, SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
		relative_ptr = ptr;

		// For each process, read the pid from shared memory
		for (i = 0; i < procs; i++) {
			int value;
			memcpy(&value, relative_ptr, sizeof(value));
			relative_ptr += sizeof(int);
			printf("%d\n", value);
		}

		// For each process, starting with the second (top level - 1), get the value and kill the process
		relative_ptr = ptr + sizeof(int);
		for (i = 0; i < procs - 1; i++) {
			int value;
			memcpy(&value, relative_ptr, sizeof(value));
			relative_ptr += sizeof(int);
			int ret = kill(value, SIGUSR1);
			while (kill(value, 0)) {
				if (errno == ESRCH) break;
			}
		}
		relative_ptr = ptr + sizeof(int);
		for (i = 0; i < procs - 1; i++) {
			int value;
			memcpy(&value, relative_ptr, sizeof(value));
			relative_ptr += sizeof(int);
			int ret = kill(value, 0);
			// printf("Ret: %d, group: %d, pid: %d\n", ret, getpgid(value), value);
		}
	}

	// Unlink the named pipe and shared memory, then (for simplicity) just call the signal handler to print/exit.
	shm_unlink(NAME);
	unlink(myfifo);
	signal_handler(10);
	return 0;
}












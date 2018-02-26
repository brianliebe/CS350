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

void print_exiting() {
	printf("EXITING: Level %d process with pid=%d, child of ppid=%d.\n", process_number, getpid(), getppid());
	exit(0);
}

void print_alive() {
	printf("ALIVE: Level %d process with pid=%d, child of ppid=%d.\n", process_number, getpid(), getppid());
}

int main (int argc, char **argv) {
	int i, fd, total_processes, value;
	pid_t pid;

	// Check arguments for validity
	if (argc != 2) {
		printf("Usage: ./prog3ipc <num-total_processes>\n");
		return 0;
	}
	total_processes = atoi(argv[1]);
	if (total_processes < 1 || total_processes > 32) {
		printf("Number of process IDs must be between 1 and 32 inclusive.\n");
		return 0;
	}
	process_number = total_processes;

	// Create named pipe, naming it "bliebe1_fifo_prog3"
	const char *myfifo = "/tmp/bliebe1_fifo_prog3";
	mkfifo(myfifo, 0666);
	char buf[5];

	// Create shared memory, named "bliebe1_shared_prog3"
	const int SIZE = total_processes * sizeof(int);
	const char *NAME = "bliebe1_shared_prog3";
	int shm_fd = shm_open(NAME, O_CREAT | O_RDWR, 0666);
	ftruncate(shm_fd, SIZE);
	void *ptr = mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);

	// Top-level process writes into memory first
	void *relative_ptr = ptr + ((total_processes - process_number) * sizeof(int));
	int process_id = getpid();
	memcpy(relative_ptr, &process_id, sizeof(process_id));
	print_alive();

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
			// CHILD - First read from unnamed pipe and place into process_number integer
			close(pfds[1]);
			if (read(pfds[0], &process_number, sizeof(process_number)) <= 0) {
				printf("Error reading from pipe.\n");
				return 0;
			}

			// Write to shared memory
			print_alive();
			void *relative_ptr = ptr + ((total_processes - process_number) * sizeof(int));
			process_id = getpid();
			memcpy(relative_ptr, &process_id, sizeof(process_id));
			
			// If it's the last process created, write "done" to named pipe and break
			if (process_number == 1) {
				fd = open(myfifo, O_WRONLY);
				write(fd, "done", sizeof("done"));
				close(fd);
				break;
			}
		}
		else {
			// PARENT - Write into the unnamed pipe for the child
			close(pfds[0]);
			int temp_num = process_number - 1;
			if (write(pfds[1], &temp_num, sizeof(temp_num)) <= 0) {
				printf("Error writing to pipe\n");
				return 0;
			}
			break;
		}
	}
	if (process_number != total_processes) {
		// This means it's any process except the first, then we should signal and pause
		signal(SIGUSR1, print_exiting);
		pause();
	}
	else {
		// If it's the first process created, read "done" from the last process
		fd = open(myfifo, O_RDONLY);
		read(fd, buf, sizeof("done"));
		close(fd);

		// Set the shared memory to be read-only
		shm_fd = shm_open(NAME, O_RDONLY, 0666);
		ptr = mmap(0, SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
		relative_ptr = ptr;

		// For each process, read the pid from shared memory
		for (i = 0; i < total_processes; i++) {
			memcpy(&value, relative_ptr, sizeof(value));
			relative_ptr += sizeof(int);
			printf("%d\n", value);
		}

		// For each process, starting with the second (top level - 1), get the value and kill the process
		relative_ptr = ptr + sizeof(int); 
		for (i = 0; i < total_processes - 1; i++) {
			memcpy(&value, relative_ptr, sizeof(value));
			relative_ptr += sizeof(int);
			kill(value, SIGUSR1);
			sleep(0); // Not sure why, but adding this makes it print in the correct order
		}
	}

	// Unlink the named pipe and shared memory, then (for simplicity) just call the signal handler to print/exit.
	shm_unlink(NAME);
	unlink(myfifo);
	print_exiting();
	return 0;
}












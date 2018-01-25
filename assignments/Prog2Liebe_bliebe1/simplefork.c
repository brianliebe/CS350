#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

void wait();

int main() {
	pid_t pid;
	pid = fork();

	if (pid < 0) {
		// error
		return 1;
	}
	if (pid == 0) {
		// child
		// execlp("/bin/ls","ls",NULL);
		execlp("./sleep", "./sleep", NULL);
	}
	else {
		wait(NULL);
		printf("Child complete\n");
	}
	return 0;
}

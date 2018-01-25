#include <stdio.h>
#include <stdlib.h>

int getpid();
int getppid();
void sleep();

int main() {
	printf("First message - pid:%d ppid:%d\n", getpid(), getppid());
	sleep(5);
	printf("Second message.\n");
	return 0;
}

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

void *child (void *arg) {
	printf("child\n");

	return NULL;
}

int main (int argc, char **argv) {
	printf("parent: begin\n");
	pthread_t c;
	pthread_create(&c, NULL, child, NULL);
	printf("parent: end\n");
	return 0;
}

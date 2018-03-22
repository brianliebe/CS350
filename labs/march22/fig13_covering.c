#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_cond_t c;
pthread_mutex_t m;

int bytesLeft = 100;

void *allocate (int size) {
	pthread_mutex_lock(&m);
	while (bytesLeft < size) pthread_cond_wait(&c, &m);
	void *ptr = malloc(size);
	bytesLeft -= size;
	pthread_mutex_unlock(&m);
	return ptr;
}

void free_mem (void *ptr, int size) {
	pthread_mutex_lock(&m);
	bytesLeft += size;
	pthread_cond_signal(&c);
	pthread_mutex_unlock(&m);
}

int main () {

}

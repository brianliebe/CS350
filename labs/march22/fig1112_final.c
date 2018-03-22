#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#define MAX 10

int buffer[MAX];
int fill_ptr = 0;
int use_ptr = 0;
int count = 0;
int g_loops = 0;
pthread_cond_t empty, fill;
pthread_mutex_t mutex, lp;

void put (int value) {
	buffer[fill_ptr] = value;
	fill_ptr = (fill_ptr + 1) % MAX;
	count ++;
}

int get() {
	int tmp = buffer[use_ptr];
	use_ptr = (use_ptr + 1) % MAX;
	count--;
	return tmp;
}

void *producer (void *arg) {
	int i;
	pthread_mutex_lock(&lp);
	int loops = g_loops;
	pthread_mutex_unlock(&lp);

	for (i = 0; i < loops; i++) {
		pthread_mutex_lock(&mutex);
		while (count == MAX) pthread_cond_wait(&empty, &mutex);
		put(i);
		pthread_cond_signal(&fill);
		pthread_mutex_unlock(&mutex);
	}
}

void *consumer (void *arg) {
	int i;
	pthread_mutex_lock(&lp);
	int loops = g_loops;
	pthread_mutex_unlock(&lp);

	for (i = 0; i < loops; i++) {
		pthread_mutex_lock(&mutex);
		while (count == 0) pthread_cond_wait(&fill, &mutex);
		int tmp = get();
		pthread_cond_signal(&empty);
		pthread_mutex_unlock(&mutex);
		printf("%d\n", tmp);
	}
}

int main() {
	pthread_t p, c;
	g_loops = 10;
	pthread_create(&p, NULL, producer, NULL);
	pthread_create(&c, NULL, consumer, NULL);
	return 0;
}

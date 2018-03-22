#include <stdio.h>
#include <pthread.h>
#include <assert.h>

int buffer;
int count = 0;
pthread_cond_t cond;
pthread_mutex_t mutex;

void put(int value) {
	printf("putting\n");
	assert (count == 0);
	count = 1;
	buffer = value;
}

int get() {
	printf("getting\n");
	assert (count == 1);
	count = 0;
	return buffer;
}

void *producer (void *arg) {
	int i;
	int loops = *(int *)arg;
	for (i = 0; i < loops; i++) {
		pthread_mutex_lock(&mutex);
		if (count == 1) pthread_cond_wait(&cond, &mutex);
		put(i);
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&mutex);
	}
}

void *consumer (void *arg) {
	int i;
	int loops = *(int *)arg;
	for (i = 0; i < loops; i++) {
		pthread_mutex_lock(&mutex);
		if (count == 0) pthread_cond_wait(&cond, &mutex);
		int tmp = get();
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&mutex);
		printf("%d\n", tmp);
	}
}

int main() {
	pthread_t p, c;
	int loops = 10;
	pthread_create(&p, NULL, producer, &loops);
	pthread_create(&c, NULL, consumer, &loops);
	return 0;
}

#include <stdio.h>
#include <pthread.h>
#include <assert.h>

int buffer;
int count = 0;

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
	printf("producer\n");
	int i;
	int loops = *(int*)arg;
	for (i = 0; i < loops; i++) put(i);
	return NULL;
}

void *consumer (void *arg) {
	printf("consumer\n");
	int i;
	while (1) {
		int tmp = get();
		printf("%d\n", tmp);
	}
	return NULL;
}

int main() {
	pthread_t p, c;
	int loops = 10;
	pthread_create(&p, NULL, producer, &loops);
	pthread_create(&c, NULL, consumer, NULL);
	return 0;
}

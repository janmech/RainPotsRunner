#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h> 
#include <pthread.h>
#include "src/test.h"
#include "src/subdir2/test2.h"


void* test_1_thread(void * arg) {
	while (true)
	{
		print_test();
		usleep(500 * 1000);
	}
	return NULL;
}

void* test_2_thread(void * arg) {
	while (true)
	{
		print_test_2();
		usleep(1000 * 1000);
	}
	return NULL;
}

int main()
{
	pid_t pid = getpid();
	printf("started: %d\n", pid);
	pthread_t serial_connector;
	pthread_create(&serial_connector, NULL, &test_1_thread, NULL);
	test_2_thread(NULL);
	return 0;
}

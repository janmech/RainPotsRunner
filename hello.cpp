#include "hello.h"
#include <iostream>

// using namespace std;

bool keep_running = true;
bool *p_keep_running = &keep_running;
pthread_t serial_connector;
pthread_t osc_listener;

void handle_sigint()
{
	pid_t pid = getpid();
	printf("\nterminating: %d\n", pid);
	*p_keep_running = false;
}

void *serial_connector_thread(void *arg)
{
	bool *keep_running = (bool *)arg;
	while (*keep_running)
	{
		// print_test();
		usleep(500 * 1000);
	}
	printf("\tEnding: serial_connector_thread\n");
	return NULL;
}

void *test_2_thread(void *arg)
{
	bool *keep_running = (bool *)arg;
	while (*keep_running)
	{
		// print_test_2();
		usleep(1000 * 1000);
	}
	return NULL;
}

int main()
{
	char sample[] = "GeeksforGeeks";

	std::cout << sample << " - A computer science portal for geeks";

	signal(SIGINT, (void (*)(int))handle_sigint);
	pid_t pid = getpid();
	printf("started: %d\n", pid);
	pthread_create(&serial_connector, NULL, &serial_connector_thread, p_keep_running);
	pthread_create(&osc_listener, NULL, &osc_lisenter_thread, p_keep_running);
	while (keep_running)
	{
		// printf("main\n");
		sleep(2);
	}
	pthread_join(serial_connector, NULL);
	pthread_join(osc_listener, NULL);
	printf("main done!\n");

	return 0;
}

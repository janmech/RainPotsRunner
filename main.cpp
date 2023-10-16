#include "main.h"

// using namespace std;

bool keep_running = true;
bool *p_keep_running = &keep_running;
pthread_t serial_connector_thread;
pthread_t osc_listener_thread;
pthread_t osc_sender_thread;

OscSender osc_sender;

void handle_sigint()
{
	pid_t pid = getpid();
	printf("\nterminating: %d\n", pid);
	osc_sender.stop();
	*p_keep_running = false;
}

void *serial_connector_run(void *arg)
{
	bool *keep_running = (bool *)arg;
	while (*keep_running)
	{
		// print_test();
		usleep(500 * 1000);
	}
	printf("\tEnding: serial_connector_run\n");
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
	signal(SIGINT, (void (*)(int))handle_sigint);
	pid_t pid = getpid();
	printf("started: %d\n", pid);
	osc_sender.start();
	std::cout << ">>>>>>>>started\n"
			  << std::endl;
	pthread_create(&serial_connector_thread, NULL, &serial_connector_run, p_keep_running);
	pthread_create(&osc_listener_thread, NULL, &osc_lisenter_run, p_keep_running);
	// pthread_create(&osc_sender_thread, NULL, &OscSender::run, p_keep_running);

	while (keep_running)
	{
		printf("main\n");
		osc_sender.inc_val();
		sleep(2);
	}
	pthread_join(serial_connector_thread, NULL);
	pthread_join(osc_listener_thread, NULL);
	pthread_join(osc_sender_thread, NULL);
	printf("main done!\n");

	return 0;
}

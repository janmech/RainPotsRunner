#include "main.hpp"

bool running = true;
OscSender osc_sender;
OscListener osc_listener;
DataHandler data_handler;

void handle_sigint()
{
	pid_t pid = getpid();
	printf("\nterminating: %d\n", pid);
	running = false;
	osc_sender.stop();
	osc_listener.stop();
}

int main()
{
	signal(SIGINT, (void (*)(int))handle_sigint);
	pid_t pid = getpid();
	printf("started: %d\n", pid);
	data_handler.getParams(true);
	data_handler.printParamConfig();
	osc_listener.start();
	sleep(1);
	osc_sender.start();

	osc_sender.addRNBOListenter();

	while (running)
	{
		printf("main\n");
		osc_sender.inc_val();
		sleep(2);
	}
	printf("main done!\n");

	return 0;
}

#include "main.hpp"

bool running = true;
OscSender *ptr_osc_sender;
OscListener *ptr_osc_listener;
DataHandler *ptr_data_handler;

void handle_sigint()
{
	pid_t pid = getpid();
	printf("\nterminating: %d\n", pid);
	running = false;
	ptr_osc_sender->stop();
	ptr_osc_listener->stop();
}

int main()
{
	signal(SIGINT, (void (*)(int))handle_sigint);
	pid_t pid = getpid();
	printf("started: %d\n", pid);
	DataHandler data_handler;
	ptr_data_handler = &data_handler;
	data_handler.getParams(true);
	data_handler.printParamConfig();
	OscListener osc_listener(&data_handler);
	ptr_osc_listener = &osc_listener;
	osc_listener.start();
	OscSender osc_sender;
	ptr_osc_sender = &osc_sender;
	osc_sender.start();

	osc_sender.addRNBOListenter();

	while (running)
	{
	
		// printf("main\n");
		// osc_sender.inc_val();
		sleep(2);
	}
	printf("main done!\n");

	return 0;
}

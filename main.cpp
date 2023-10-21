#include "main.hpp"

bool running = true;
OscSender *ptr_osc_sender;
OscListener *ptr_osc_listener;
DataHandler *ptr_data_handler;
SerialConnector *ptr_serial_connector;

void handle_sigint()
{
	pid_t pid = getpid();
	printf("\nterminating: %d\n", pid);
	running = false;
	ptr_osc_sender->stop();
	ptr_osc_listener->stop();
	ptr_serial_connector->stop();
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

	SerialConnector serial_connector(&osc_sender);
	ptr_serial_connector = &serial_connector;
	serial_connector.start();

	osc_sender.addRNBOListenter();

	while (running)
	{
		std::cout << "MAIN THREAD" << std::endl;
		sleep(1);
	}
	std::cout << "\nmain done!" << std::endl;

	return 0;
}
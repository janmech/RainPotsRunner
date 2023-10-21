#include "main.hpp"

bool running = true;
OscSender *ptr_osc_sender;
OscListener *ptr_osc_listener;
DataHandler *ptr_data_handler;

#define SERIAL_PORT_PATH "/dev/ttyS0"
#define SERIAL_IN_BUFFER_LEN 10

#define MSG_BUFFER_SIZE 12
#define MSG_CC_STATUS_BIT_MASK 0x0B
#define MSG_CC_PACKET_SIZE 4
#define MSG_PGM_STATUS_BIT_MASK 0x0C
#define MSG_PGM_PACKET_SIZE 2
#define MSG_SAVE 0xF4
#define MSG_SAVE_PACKET_SIZE 2

char serial_in_buffer[SERIAL_IN_BUFFER_LEN];

struct termios options;

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
	int fd = open(SERIAL_PORT_PATH, O_RDWR | O_NDELAY);

	tcgetattr(fd, &options);

	cfmakeraw(&options);
	cfsetospeed(&options, B230400);
	options.c_cc[VMIN] = 2;
	options.c_cc[VTIME] = 1;
	options.c_cflag &= ~CRTSCTS;
	tcflush(fd, TCIOFLUSH);
	tcsetattr(fd, TCSANOW, &options);

	if (tcsetattr(fd, TCSANOW, &options) == -1)
	{
		std::cout << "tcsetattr failed" << std::endl;
		return -1;
	}
	tcflush(fd, TCIOFLUSH); // Clear IO buffer

	memset(serial_in_buffer, 0, SERIAL_IN_BUFFER_LEN);

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

	char msg_packet_buffer[MSG_BUFFER_SIZE];
	uint8_t msg_packet_index = 0;
	uint8_t msg_packet_size = 0;
	uint8_t msg_type = 0;
	bool is_parsing = false;
	memset(msg_packet_buffer, 0, MSG_BUFFER_SIZE);

	while (running)
	{
		if (int received = read(fd, serial_in_buffer, 1) > 0)
		{
			if (!is_parsing)
			{
				// We use an undefiged System Common Messages as a status byte foe 'save', hence there is no channel information
				if (serial_in_buffer[0] == MSG_SAVE)
				{
					msg_packet_size = MSG_SAVE_PACKET_SIZE;
					msg_packet_index = 0;
					is_parsing = true;
					memset(msg_packet_buffer, 0, MSG_BUFFER_SIZE);
				}
				else
				{
					msg_type = serial_in_buffer[0] >> 4;
					switch (msg_type)
					{
					case MSG_CC_STATUS_BIT_MASK:
						msg_packet_size = MSG_CC_PACKET_SIZE;
						msg_packet_index = 0;
						is_parsing = true;
						memset(msg_packet_buffer, 0, MSG_BUFFER_SIZE);
						break;
					case MSG_PGM_STATUS_BIT_MASK:
						msg_packet_size = MSG_PGM_PACKET_SIZE;
						msg_packet_index = 0;
						is_parsing = true;
						memset(msg_packet_buffer, 0, MSG_BUFFER_SIZE);
						break;
					default:
						break;
					}
				}
			}
			if (is_parsing)
			{
				msg_packet_buffer[msg_packet_index] = serial_in_buffer[0];
				msg_packet_index++;
			}
			if (msg_packet_index == msg_packet_size)
			{
				for (size_t i = 0; i < msg_packet_size; i++)
				{
					printf("0x%02X ", msg_packet_buffer[i]);
				}
				printf("\n");
				msg_packet_size = 0;
				msg_packet_index = 0;
				is_parsing = false;
			}
		};
	}
	close(fd);
	printf("main done!\n");

	return 0;
}

// // cfsetospeed(&options, B115200);
// // cfmakeraw(&options);

// cfsetospeed(&options, 380400);
// options.c_cc[VMIN] = 2;
// options.c_cc[VTIME] = 1;
// options.c_cflag &= ~CRTSCTS;
// tcflush(fd, TCIOFLUSH);
// tcsetattr(fd, TCSANOW, &options);
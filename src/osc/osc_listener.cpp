#include "osc_listener.h"

void *osc_lisenter_thread(void *arg)
{
	// const char *osc_[4] = {"Blue", "Red", "Orange", "Yellow"};

	bool *keep_running = (bool *)arg;
	char buffer[2048]; // declare a 2Kb buffer to read packet data into

	printf("Starting write tests:\n");

	const int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_fd < 0)
	{
		perror("cannot open socket");
	}
	else
	{
		fcntl(socket_fd, F_SETFL, O_NONBLOCK); // set the socket to non-blocking
		struct sockaddr_in out_addr;
		out_addr.sin_family = AF_INET;
		out_addr.sin_port = htons(5678);
		// sout.sin_addr.s_addr = inet_addr("localhost");
		//  store this IP address in sa:
		int res = inet_pton(AF_INET, "192.168.1.132", &out_addr.sin_addr);
		printf("%d\n", res);

		bind(socket_fd, (struct sockaddr *)&out_addr, sizeof(struct sockaddr_in));
		int len = 0;
		// char blob[8] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
		len = tosc_writeMessage(buffer, sizeof(buffer), "/address", "fi", 1.0f, 88);
		tosc_printOscBuffer(buffer, len);

		sendto(socket_fd, buffer, len, 0,(struct sockaddr *)&out_addr, sizeof(out_addr));
		close(socket_fd);

		printf("done.\n");
	}

	// register the SIGINT handler (Ctrl+C)
	// signal(SIGINT, &sigintHandler);

	// open a socket to listen for datagrams (i.e. UDP packets) on port 9000
	const int fd = socket(AF_INET, SOCK_DGRAM, 0);

	fcntl(fd, F_SETFL, O_NONBLOCK); // set the socket to non-blocking
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(9000);
	sin.sin_addr.s_addr = INADDR_ANY;
	bind(fd, (struct sockaddr *)&sin, sizeof(struct sockaddr_in));
	printf("tinyosc is now listening on port 9000.\n");
	printf("Press Ctrl+C to stop.\n");

	while (*keep_running)
	{
		fd_set readSet;
		FD_ZERO(&readSet);
		FD_SET(fd, &readSet);
		struct timeval timeout = {1, 0}; // select times out after 1 second
		if (select(fd + 1, &readSet, NULL, NULL, &timeout) > 0)
		{
			struct sockaddr sa; // can be safely cast to sockaddr_in
			socklen_t sa_len = sizeof(struct sockaddr_in);
			int len = 0;
			while ((len = (int)recvfrom(fd, buffer, sizeof(buffer), 0, &sa, &sa_len)) > 0)
			{
				if (tosc_isBundle(buffer))
				{
					tosc_bundle bundle;
					tosc_parseBundle(&bundle, buffer, len);
					// const uint64_t timetag = tosc_getTimetag(&bundle);
					tosc_message osc;
					while (tosc_getNextMessage(&bundle, &osc))
					{
						tosc_printMessage(&osc);
					}
				}
				else
				{
					tosc_message osc;
					tosc_parseMessage(&osc, buffer, len);
					tosc_printMessage(&osc);
					// Printing Strings stored in 2D array
					// for (int i = 0; i < 4; i++)
					// 	printf("%s\n", colour[i]);
				}
			}
		}
	}

	// close the UDP socket
	close(fd);
	printf("\tClosing UDP socket\n");
	return 0;
}
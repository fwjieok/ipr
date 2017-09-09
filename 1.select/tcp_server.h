#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

class TCP_Server {
private:
	int  listen_port;
	int  server_sockfd;
	struct sockaddr_in server_addr;
	struct timeval timeout_val;
	fd_set readfds;

	void on_accept_process();

public:
	TCP_Server();
	~TCP_Server();

	virtual void on_new_connection(int socket_fd) = 0;

	int  begin(char *ip, int port);
	void loop();
	void tick_1s();
};

#endif
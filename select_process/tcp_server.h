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

#include "list.h"


#define LOCAL_HOST_IP "127.0.0.1"
#define CLIENT_NUM 10

#define MAX(a, b)  a>b?a:b

#define DATA_BUF_SIZE 10240

typedef struct _session {
	int  socket_fd;
	char remote_addr[20];
	int  remote_port;

	char data_buf[DATA_BUF_SIZE];
	char package_buf[4096];
	
} Session;

class TCP_Server
{
private:
	int listen_port;
	int server_sockfd;
	struct sockaddr_in server_addr;
	struct timeval timeout_val;
	fd_set readfds;

	List *session_list; 

	void on_connection();
	void on_session_data(Session *ss);
	void on_data_process(Session *ss);
	void on_session_close(Session *ss);


public:
	TCP_Server();
	~TCP_Server();

	void begin(int port);
	void loop();
	void tick_1s();

	void close_all_session();
	
};

#endif
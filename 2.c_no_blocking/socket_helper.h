#ifndef SOCKET_HELPER_H
#define SOCKET_HELPER_H

class Socket_Helper {
public:
	int  socket_fd;
	char remote_addr[50];
	int  remote_port;
};

#endif
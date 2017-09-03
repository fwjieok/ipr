#ifndef DAEMON_TCP_H
#define DAEMON_TCP_H

#include "list.h"
#include "socket_helper.h"
#include "tcp_server.h"

class Daemon_Tcp: public TCP_Server {

private:	
	int line;
	int port;

	struct timeval timeout_val;
	fd_set readfds;

	List *session_list;	

public:

public:
	Daemon_Tcp();
	~Daemon_Tcp();

	void on_new_connection(Socket_Helper *socket_helper);
	
	void on_session_data(Socket_Helper *socket_helper);
	void on_session_close(Socket_Helper *socket_helper);
	void close_all_session();
	
	void session_list_loop();

	int  start();
	void loop();
	void tick_1s();
};


#endif
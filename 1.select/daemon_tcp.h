#ifndef DAEMON_TCP_H
#define DAEMON_TCP_H

#include "list.h"
#include "session.h"
#include "tcp_server.h"

class Daemon_Tcp: public TCP_Server {

private:	
	int line;
	int port;

	struct timeval timeout_val;
	fd_set readfds;

	List *session_list;	

public:
	Daemon_Tcp();
	~Daemon_Tcp();

	void on_new_connection(int socket_fd);
	
	void on_session_data(Session *session);
	void on_session_close(Session *session);
	void close_all_session();
	
	void session_list_loop();

	int  start();
	void loop();
	void tick_1s();

    void debug_session_list();
};


#endif

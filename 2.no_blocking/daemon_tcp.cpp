#ifndef DAEMON_TCP_CPP
#define DAEMON_TCP_CPP

#include "tools.h"
#include "daemon_tcp.h"

#define MAX(a, b)  a>b?a:b

Daemon_Tcp::Daemon_Tcp() {
}

Daemon_Tcp::~Daemon_Tcp() {
	close_all_session();
}

int Daemon_Tcp::start() {

	if (TCP_Server::begin((char *)"0.0.0.0", 1980) < 0) {
		printf("TCP Server Listen Error\n");
		return -1;
	}
	

	session_list = list_create();
	if (!session_list) {
		printf("session list create error!");
		fflush(stdout);
		return -1;
	}

	timeout_val.tv_sec  = 0;
	timeout_val.tv_usec = 0;

	return 0;
}


void Daemon_Tcp::on_new_connection(Socket_Helper *socket_helper) {
	if (socket_helper) {
		printf("new connection: %s:%d, socket_fd: %d\n", 
			socket_helper->remote_addr, socket_helper->remote_port, socket_helper->socket_fd);

		set_non_blocking(socket_helper->socket_fd);

		list_push(session_list, (void *)socket_helper);
	}
}

void Daemon_Tcp::close_all_session() {
	LIST_FOREACH_SAFE(session_list, cur) {
        if (cur->value) {
            Socket_Helper *sh = (Socket_Helper *)cur->value;
    		shutdown(sh->socket_fd, SHUT_RDWR);
			free(list_remove(session_list, cur));
        }
	}

	list_destroy(session_list);
}

void Daemon_Tcp::on_session_close(Socket_Helper *sh) {
	printf("socket close, fd: %d\n", sh->socket_fd);	
	LIST_FOREACH_SAFE(session_list, cur) {
        if (cur->value) {
            Socket_Helper *cur_sh = (Socket_Helper *)cur->value;
            if (sh->socket_fd == cur_sh->socket_fd) {
            	shutdown(sh->socket_fd, SHUT_RDWR);
				free(list_remove(session_list, cur));
				break;			
			}			
        }
	}
}

void Daemon_Tcp::on_session_data(Socket_Helper *sh) {
	if (!sh) { return; }

	char socket_buf[2048] = {0};
	int  buf_len = sizeof(socket_buf);

	int ret = recv(sh->socket_fd, socket_buf, buf_len, 0);
	if (ret < 0) {					//出错			
		return;
	} else if (ret == 0) {			//Socket断开
		on_session_close(sh);
	} else {		
		printf("on_session_data: %s", socket_buf);

		send(sh->socket_fd, (void *)socket_buf, ret, 0);
	}
}

void Daemon_Tcp::session_list_loop() {
	LIST_FOREACH_SAFE(session_list, node) {
        if (node->value) {
            Socket_Helper *sh = (Socket_Helper *)node->value;
			on_session_data(sh);
        }
	}
}

void Daemon_Tcp::loop() {
	TCP_Server::loop();

	session_list_loop();
}

void Daemon_Tcp::tick_1s() {
	TCP_Server::tick_1s();
}


#endif
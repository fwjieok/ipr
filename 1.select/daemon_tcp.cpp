#ifndef DAEMON_TCP_CPP
#define DAEMON_TCP_CPP

#include "daemon_tcp.h"
#include "session_ipr_2.h"

#define MAX(a, b)  a>b?a:b

Daemon_Tcp::Daemon_Tcp() {
}

Daemon_Tcp::~Daemon_Tcp() {
	close_all_session();

    if (session_list) {
        list_destroy(session_list);
    }
}

int Daemon_Tcp::start() {

	if (TCP_Server::begin((char *)"0.0.0.0", 2017) < 0) {
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


void Daemon_Tcp::on_new_connection(int socket_fd) {
	if (socket_fd <= 0) { return; }

	Session *session = new Session_ipr_2(socket_fd);

	struct sockaddr_in client_addr;
	socklen_t addr_len = sizeof(client_addr);
	if (0 == getpeername(socket_fd, (struct sockaddr *)&client_addr, &addr_len)) {
		session->remote_addr = strdup(inet_ntoa(client_addr.sin_addr));
		session->remote_port = ntohs(client_addr.sin_port);
		printf("new connection: %s:%d, socket_fd: %d\n", session->remote_addr, ntohs(client_addr.sin_port), socket_fd);
	}
	
	list_push(session_list, (void *)session);
}

void Daemon_Tcp::close_all_session() {
	LIST_FOREACH_SAFE(session_list, cur) {
        if (cur->value) {
            Session *session = (Session *)cur->value;
    		session->session_close();
			list_remove(session_list, cur);
			delete session;
        }
	}
}

void Daemon_Tcp::on_session_close(Session *session) {
	if (!session) { return; }

	printf("socket close, fd: %d\n", session->socket_fd);	
	LIST_FOREACH_SAFE(session_list, cur) {
        if (cur->value) {
            Session *cur_ss = (Session *)cur->value;
            if (session->socket_fd == cur_ss->socket_fd) {
            	session->session_close();		//session socket close
            	list_remove(session_list, cur); //daemon list remove
            	delete session;					//object delete
				break;			
			}
        }
	}
}

void Daemon_Tcp::on_session_data(Session *session) {
	if (!session) { return; }

	char buf[4096];
	int len = read(session->socket_fd, buf, sizeof(buf));
	if (len < 0) {
		if (errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK) {

		} else {
			on_session_close(session);
		}
	} else if (len == 0) {
		on_session_close(session);
	} else {
		session->do_process_data(buf, len);
	}

}

void Daemon_Tcp::session_list_loop() {
	int max_fd = 0;
	FD_ZERO(&readfds);
	LIST_FOREACH(session_list, first, next, cur) {
        if (cur->value) {
            Session *session = (Session *)cur->value;
            if (session && session->socket_fd > 0) {
				FD_SET(session->socket_fd, &readfds);
				max_fd = MAX(max_fd, session->socket_fd);
			}
        }
    }
    int ret = select(max_fd + 1, &readfds, NULL, NULL, &timeout_val);
	if (ret < 0) {
		perror("Daemon_Tcp select");
	} else if (ret == 0) {
		//select timeout
	} else {
		LIST_FOREACH_SAFE(session_list, node) {
	        if (node->value) {
	            Session *session = (Session *)node->value;
	            if (session && FD_ISSET(session->socket_fd, &readfds)) {
					on_session_data(session);
				}
	        }
    	}
	}
}

void Daemon_Tcp::loop() {
	TCP_Server::loop();        //处理TCP Server accept事件

	session_list_loop();       //处理Session data read事件
}

void Daemon_Tcp::tick_1s() {
	TCP_Server::tick_1s();

	LIST_FOREACH(session_list, first, next, cur) {
        if (cur->value) {
            Session *session = (Session *)cur->value;
            if (session) {
				session->tick_1s();
			}
        }
    }
}

void Daemon_Tcp::debug_session_list() {
    printf("------------------- session list -------------------\n");
    printf("    acct \t model \t\t tid \t\t\t  netif  \t    ver   \t level \t  remote_addr\n");
	LIST_FOREACH(session_list, first, next, cur) {
        if (cur->value) {
            Session *session = (Session *)cur->value;
            if (session) {
                printf("%8s \t %s \t %s \t %s \t %s \t %d \t %s\n",
                       session->runtime.areas.acct,
                       session->runtime.profile.model,
                       session->runtime.profile.tid,
                       session->runtime.netif,
                       session->runtime.profile.ver,
                       session->runtime.level,
                       session->remote_addr);
			}
        }
    }
}


#endif

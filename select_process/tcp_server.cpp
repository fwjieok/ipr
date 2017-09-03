#include "tcp_server.h"

TCP_Server::TCP_Server() {

}

TCP_Server::~TCP_Server() {

}

void TCP_Server::begin(int port) {
	listen_port   = port;
	server_sockfd = -1;
	timeout_val.tv_sec  = 0;
	timeout_val.tv_usec = 0;
	FD_ZERO(&readfds);

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = PF_INET;
	server_addr.sin_port   = htons(listen_port);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	
	if ((server_sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		fflush(stdout);
		exit(-1);
	}

	// 设置套接字选项避免地址使用错误  
    int opt = 1;  
    if(setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {  
        perror("setsockopt");
        fflush(stdout);
        exit(-1);  
    }  

	if (bind(server_sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
		perror("bind");
		fflush(stdout);
		exit(-1);
	}

	if (listen(server_sockfd, CLIENT_NUM) == -1) {
		perror("listen");
		fflush(stdout);
		exit(-1);
	}

	session_list = list_create();
	if (!session_list) {
		printf("ERROR: session list create error!");
		fflush(stdout);
		exit(-1);
	}

}

/*
char *TCP_Server::create_session_id(int client_fd) {
	char sid[20];
	sprintf(sid, "%05d", client_fd);

	return sid;
}
*/

void TCP_Server::on_connection() {
	int client_fd = -1;
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);
	bzero(&client_addr, sizeof(client_addr));

	if ((client_fd = accept(server_sockfd, (struct sockaddr *)&client_addr, &len)) == -1) {
		perror("ERROR: accept");
		fflush(stdout);
		exit(-1);
	} else {
		char *addr = inet_ntoa(client_addr.sin_addr);
		if (strcmp(addr, LOCAL_HOST_IP) != 0) {			
			shutdown(client_fd, SHUT_RDWR);
			return;
		}
		printf("new connection: %s:%d, socket_fd: %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), client_fd);
		Session *session = (Session *)malloc(sizeof(Session));
		if (session) {
			session->socket_fd   = client_fd;
			session->remote_port = ntohs(client_addr.sin_port);
			sprintf(session->remote_addr, "%s", inet_ntoa(client_addr.sin_addr));

			list_push(session_list, (void *)session);
		}		
	}
}

void TCP_Server::close_all_session() {
	LIST_FOREACH_SAFE(session_list, cur) {
        if (cur->value) {
            Session *ss = (Session *)cur->value;
    		shutdown(ss->socket_fd, SHUT_RDWR);
			free(list_remove(session_list, cur));
        }
	}

	list_destroy(session_list);
}

void TCP_Server::on_session_close(Session *ss) {
	printf("socket close, fd: %d\n", ss->socket_fd);	
	LIST_FOREACH_SAFE(session_list, cur) {
        if (cur->value) {
            Session *cur_ss = (Session *)cur->value;
            if (ss->socket_fd == cur_ss->socket_fd) {
            	shutdown(ss->socket_fd, SHUT_RDWR);
				free(list_remove(session_list, cur));
				break;			
			}			
        }
	}
}

void TCP_Server::on_data_process(Session *ss) {
	char *package_buf = ss->data_buf;
	
	send(ss->socket_fd, (void *)package_buf, strlen(package_buf), 0);
}

void TCP_Server::on_session_data(Session *ss) {
	if (!ss) { return; }

	char socket_buf[2048] = {0};
	int  buf_len = sizeof(socket_buf);

	int ret = recv(ss->socket_fd, socket_buf, buf_len, 0);
	if (ret < 0) {					
		return;
	} else if (ret == 0) {			//Socket断开
		on_session_close(ss);
	} else {		
		printf("on_session_data: %s", socket_buf);
		memset(ss->data_buf, 0, sizeof(ss->data_buf));
		memcpy((void *)ss->data_buf, (void *)socket_buf, ret);

		on_data_process(ss);
	}
}

void TCP_Server::loop() {
	if (server_sockfd <= 0) { return; }

	FD_ZERO(&readfds);

	FD_SET(server_sockfd, &readfds);
	int max_fd = server_sockfd + 1;

	LIST_FOREACH(session_list, first, next, cur) {
        if (cur->value) {
            Session *ss = (Session *)cur->value;
            if (ss && ss->socket_fd > 0) {
				FD_SET(ss->socket_fd, &readfds);
				max_fd = MAX(max_fd, ss->socket_fd);
			}
        }
    }
	
	int ret = select(max_fd + 1, &readfds, NULL, NULL, &timeout_val);
	if (ret < 0) {
		perror("TCP Server select");
	} else if (ret == 0) {

	} else {			
		if (FD_ISSET(server_sockfd, &readfds)) {
			on_connection();
		} else {
			LIST_FOREACH_SAFE(session_list, node) {
		        if (node->value) {
		            Session *ss = (Session *)node->value;
		            if (ss && FD_ISSET(ss->socket_fd, &readfds)) {
						on_session_data(ss);
					}
		        }
	    	}
		}
	}

	
}

void TCP_Server::tick_1s() {

}
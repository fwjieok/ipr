#include "tcp_server.h"
#include "socket_helper.h"

TCP_Server::TCP_Server() {

}

TCP_Server::~TCP_Server() {
	shutdown(server_sockfd, SHUT_RDWR);
	close(server_sockfd);
	server_sockfd     = -1;
}

int TCP_Server::begin(char *ip, int port) {
	if (port <= 0) {
		printf("listen error, the parameter port is not equals 0\n");
		return -1;
	}

	server_sockfd = -1;
	listen_port   = port;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = PF_INET;
	server_addr.sin_port   = htons(listen_port);
	if (ip) {
		if (INADDR_NONE == inet_addr(ip)) {
			printf("the parameter ip %s is not legal\n", ip);
			return -1;
		}
		server_addr.sin_addr.s_addr = inet_addr(ip);
	} else {
		server_addr.sin_addr.s_addr = INADDR_ANY;
	}	
	
	timeout_val.tv_sec  = 0;
	timeout_val.tv_usec = 0;

	if ((server_sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		fflush(stdout);
		return -1;
	}

	// 设置套接字选项避免地址使用错误  
    int opt = 1;  
    if(setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {  
        perror("setsockopt");
        fflush(stdout);
        return -1;
    }  

	if (bind(server_sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
		perror("bind");
		fflush(stdout);
		exit(-1);
	}

	if (listen(server_sockfd, 65535) == -1) {
		perror("listen");
		fflush(stdout);
		return -1;
	}

	printf("tcp listening on %s:%d, server_sockfd: %d\n", ip, port, server_sockfd);

	return 0;
}

void TCP_Server::on_accept_process() {
	int client_fd = -1;
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);
	bzero(&client_addr, sizeof(client_addr));

	if ((client_fd = accept(server_sockfd, (struct sockaddr *)&client_addr, &len)) == -1) {
		perror("ERROR: accept");
		fflush(stdout);
		exit(-1);
	} else {		
		//if (on_new_connection) {
			Socket_Helper *socket_helper = new Socket_Helper();
			strcpy(socket_helper->remote_addr, inet_ntoa(client_addr.sin_addr));
			socket_helper->remote_port = ntohs(client_addr.sin_port);
			socket_helper->socket_fd   = client_fd;
			on_new_connection(socket_helper);
		//}
	}
}

void TCP_Server::loop() {
	if (server_sockfd <= 0) { return; }

	FD_ZERO(&readfds);
	FD_SET(server_sockfd, &readfds);
	int max_fd = server_sockfd + 1;

	int ret = select(max_fd, &readfds, NULL, NULL, &timeout_val);
	if (ret < 0) {
		perror("TCP Server select");
	} else if (ret == 0) {
		//select timeout
	} else {			
		if (FD_ISSET(server_sockfd, &readfds)) {
			on_accept_process();
		}
	}
}

void TCP_Server::tick_1s() {

}
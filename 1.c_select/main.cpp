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
#include "ringbuf.h"

#include "cJSON.h"
#include "cJSON_Utils.h"

#define MAX(a, b)  a>b?a:b

#define SERVER_PORT 8888
#define CLIENT_NUM  10

#define DATA_BUF_SIZE 2048

typedef struct _session {
	int  socket_fd;
	char remote_addr[20];
	int  remote_port;

	char data_buf[DATA_BUF_SIZE];
	char package_buf[1024];
	
} Session;

List *session_list = NULL; 

char *create_session_id(int client_fd) {
	char sid[20];
	sprintf(sid, "%05d", client_fd);

	return sid;
}

void on_connection(int server_sockfd, fd_set *p_fdsets) {
	static int client_index = 0;

	int client_fd = -1;
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);
	bzero(&client_addr, sizeof(client_addr));

	if ((client_fd = accept(server_sockfd, (struct sockaddr *)&client_addr, &len)) == -1) {
		perror("accept");
		exit(-1);
	} else {
		printf("new connection: %s:%d, socket_fd: %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), client_fd);
		Session *session = (Session *)malloc(sizeof(Session));
		if (session) {
			session->socket_fd = client_fd;
			session->remote_port = ntohs(client_addr.sin_port);
			sprintf(session->remote_addr, "%s", inet_ntoa(client_addr.sin_addr));

			list_push(session_list, (void *)session);

			printf("new session, p: %p\n", session);
		}		
	}
}

void on_session_close(Session *ss) {
	printf("socket close, fd: %d\n", ss->socket_fd);	
	LIST_FOREACH(session_list, first, next, cur) {
        if (cur->value) {
            Session *cur_ss = (Session *)cur->value;
            if (ss->socket_fd == cur_ss->socket_fd) {            	
            	shutdown(ss->socket_fd, SHUT_RDWR);
				//list_remove(session_list, cur);
				printf("p: %p\n", cur->value);
				free(cur_ss);	
				break;			
			}			
        }
	}
}

void on_data_process(Session *ss) {
	char *package_buf = ss->data_buf;
	
	send(ss->socket_fd, (void *)package_buf, strlen(package_buf), 0);
}

void on_session_data(Session *ss) {
	char socket_buf[1024] = {0};
	int  buf_len = sizeof(socket_buf);

	while(true) {
		int ret = recv(ss->socket_fd, socket_buf, buf_len, 0);
		if (ret < 0) {
			break;
		} else if (ret == 0) {			//Socket断开
			on_session_close(ss);
			break;
		} else {
			if (ret < buf_len) {
				printf("on_session_data: %s", socket_buf);
				memset(ss->data_buf, 0, sizeof(ss->data_buf));
				memcpy((void *)ss->data_buf, (void *)socket_buf, ret);
				break;				
			} else {

			}
			on_data_process(ss);
		}
	}
}

void on_stdin_process() {
	char buf[1024];
	char *p = buf;

	if (fgets(buf, sizeof(buf), stdin)) {		
		char *cmd = strsep(&p, ",");
		if(0 == strncmp(cmd, "list", 4)) {
			int i = 0;
			LIST_FOREACH(session_list, first, next, cur) {
				if (cur->value) {
					Session *ss = (Session *)cur->value;
					printf("%d, %3d, %s:%d\n", ++i, ss->socket_fd, ss->remote_addr, ss->remote_port);
				}
			}
			if (i == 0) {
				printf("no data!\n");
			}
		}
	}

}

void singal_handler(int sig_no) {

	list_clear_destroy(session_list);
}

int main (int argc, char *argv[]) {

	session_list = list_create();

	int server_sockfd = -1;
	struct sockaddr_in server_addr;

	if ((server_sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(-1);
	}

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = PF_INET;
	server_addr.sin_port   = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	// 设置套接字选项避免地址使用错误  
    int opt = 1;  
    if(setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {  
        perror("setsockopt");  
        exit(-1);  
    }  

	if (bind(server_sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
		perror("bind");
		exit(-1);
	}

	if (listen(server_sockfd, CLIENT_NUM) == -1) {
		perror("listen");
		exit(-1);
	}

	time_t time_now, time_last;
	int counter = 0;

	struct timeval val = {0, 0};
	fd_set fdsets;
	FD_ZERO(&fdsets);

	int max_fd = 0;

	int stdin_fd = fileno(stdin);
	while(true) {

		LIST_FOREACH(session_list, first, next, cur) {
	        if (cur->value) {
	            Session *ss = (Session *)cur->value;
	            if (ss && ss->socket_fd > 0) {
					FD_SET(ss->socket_fd, &fdsets);
					max_fd = MAX(max_fd, ss->socket_fd);
				}
	        }
	    }

	    FD_SET(stdin_fd, &fdsets);					//stdin

		FD_SET(server_sockfd, &fdsets);		//TCP Server
		max_fd = MAX(max_fd, server_sockfd);
		int ret = select(max_fd + 1, &fdsets, NULL, NULL, &val);
		if (ret < 0) {
			perror("select");
		} else if (ret == 0) {

		} else {			
			if (FD_ISSET(server_sockfd, &fdsets)) {
				on_connection(server_sockfd, &fdsets);
			} else if (FD_ISSET(stdin_fd, &fdsets)) {
				on_stdin_process();
			} else {
				LIST_FOREACH(session_list, first, next, cur) {
			        if (cur->value) {
			            Session *ss = (Session *)cur->value;
			            if (ss && FD_ISSET(ss->socket_fd, &fdsets)) {
							on_session_data(ss);
						}
			        }
		    	}
			}
		}

		time_now = time(NULL);
		if ((time_now - time_last) >= 1) {
			time_last = time_now;

			printf(".");
			if (counter++ > 10) {
				printf("\n");
				counter = 0;
			}
			fflush(stdout);

		}
	}

}


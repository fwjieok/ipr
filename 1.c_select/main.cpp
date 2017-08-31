#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#include "stdin_util.h"
#include "tcp_server.h"
#include "list.h"

#define MAX(a, b)  a>b?a:b

int on_stdin_process(char *data) {
	char *cmd  = strsep(&data, ",");
	char *ddd  = strsep(&data, ",");

	printf("cmd: %s, data: %s\n", cmd, ddd);

}

TCP_Server *tcp_server = NULL;

void signal_handler(int sig_no) {
	printf("on singal: %d\n", sig_no);
	fflush(stdout);

    if(sig_no == SIGINT) {
    	if(tcp_server) {
    		tcp_server->close_all_session();
    		delete tcp_server;
    	}

        exit(-1);
    }
}

int main (int argc, char *argv[]) {

	signal(SIGINT, signal_handler);

	time_t time_now, time_last;
	int counter = 0;

	TCP_Server *tcp_server = new TCP_Server();
	if (!tcp_server) {
		printf("TCP Server Create Error\n");
		exit(-1);
	}

	tcp_server->begin(1980);

	printf("TCP_Server listening on 1980\n");
	fflush(stdout);

	Stdin_Util *stdin_util = new Stdin_Util();
	stdin_util->set_data_process_function(on_stdin_process);

	while(true) {

		stdin_util->loop();

		tcp_server->loop();
		
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


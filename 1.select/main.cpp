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
#include "daemon_tcp.h"
#include "list.h"

#define MAX(a, b)  a>b?a:b

Daemon_Tcp *daemon_tcp = NULL;
Stdin_Util *stdin_util = NULL;

void signal_handler(int sig_no) {
	printf("on singal: %d\n", sig_no);
	fflush(stdout);

    if(sig_no == SIGINT) {
    	if(daemon_tcp) {
    		delete daemon_tcp;
    	}
    	if (stdin_util) {
    		delete stdin_util;
    	}

        exit(-1);
    }
}

int on_stdin_process(char *data) {
	char *cmd   = strsep(&data, ",");
	char *param = data;

    printf("on_stdion_process, cmd: %s\n", cmd);
    if (0 == strncmp(cmd, "debug", 5)) {

    	if(daemon_tcp) {
            daemon_tcp->debug_session_list();
    	}
    }
}

int main (int argc, char *argv[]) {

	signal(SIGINT, signal_handler);

	time_t time_now, time_last;
	int counter = 0;

	stdin_util = new Stdin_Util();
	stdin_util->set_data_process_function(on_stdin_process);

	daemon_tcp = new Daemon_Tcp();
	if (daemon_tcp->start() < 0) {
		printf("daemon_tcp start error!\n");
	}

	while(true) {

		stdin_util->loop();

		daemon_tcp->loop();
		
		time_now = time(NULL);
		if ((time_now - time_last) >= 1) {
			time_last = time_now;

			daemon_tcp->tick_1s();

			printf(".");	
			if (counter++ > 10) {
				printf("\n");
				counter = 0;
			}		
			fflush(stdout);
			
		}
	}

}


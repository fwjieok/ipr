#include "stdin_util.h"

Stdin_Util::Stdin_Util() {
	timeout_val.tv_sec  = 0;
	timeout_val.tv_usec = 0;
	stdin_data_process  = NULL;
    stdin_fd            = STDIN_FILENO;
}

Stdin_Util::~Stdin_Util() {

}

void Stdin_Util::set_data_process_function(FUNCTION_STDIN_DATA_PROCESS function) {
	stdin_data_process = function;
}

void Stdin_Util::on_stdin_process() {
	char *p = buf;
	if (fgets(buf, sizeof(buf), stdin)) {
		if (stdin_data_process) {
			stdin_data_process(p);
		} else {
			char *cmd   = strsep(&p, ",");
			char *param = p;
            printf("on_stdin_process: cmd: %s, param: %s\n", cmd, param);
		}
	}
}

void Stdin_Util::loop() {
	FD_ZERO(&readfds);		
    FD_SET(stdin_fd, &readfds);          //stdin
	int max_fd = stdin_fd + 1;
	int ret = select(max_fd, &readfds, NULL, NULL, &timeout_val);
	if (ret < 0) {          //select error
		perror("select");
	} else if (ret == 0) {	//timeout

	} else {			
		if (FD_ISSET(stdin_fd, &readfds)) {
			on_stdin_process();
		}
	}
}


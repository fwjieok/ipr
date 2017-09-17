#ifndef STDIN_UTIL_H
#define STDIN_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/select.h>

typedef	int (*FUNCTION_STDIN_DATA_PROCESS)(char *data);

class Stdin_Util {
private:
	char buf[4096];
	struct timeval timeout_val;
	fd_set readfds;
	int stdin_fd;

	FUNCTION_STDIN_DATA_PROCESS stdin_data_process;

	void on_stdin_process();

public:
	Stdin_Util();
	~Stdin_Util();

	void set_data_process_function(FUNCTION_STDIN_DATA_PROCESS function);

	void loop();
	
};

#endif

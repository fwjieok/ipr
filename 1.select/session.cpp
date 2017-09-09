#include <stdio.h>
#include <stdlib.h>

#include "session.h"

Session::Session() {
	remote_addr = NULL;
	remote_port = -1;
}

Session::~Session() {
	if (remote_addr) {
		free(remote_addr);
	}
}
#include <stdio.h>
#include "session_ipr_2.h"

Session_ipr_2::Session_ipr_2(int socket_fd):Session_tcp(socket_fd) {
}

Session_ipr_2::~Session_ipr_2() {
}

void Session_ipr_2::do_process_package() {	
	char *package = get_package_buf();
	printf("package_buf: %s\n", get_package_buf());
}

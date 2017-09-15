#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "session_ipr_2.h"

Session_ipr_2::Session_ipr_2(int socket_fd):Session_tcp(socket_fd) {
}

Session_ipr_2::~Session_ipr_2() {
}

void Session_ipr_2::do_process_package() {	
	char *package = get_package_buf();
	printf("package_len: %d, package_buf: %s\n", strlen(package), package);

    if (1 == strlen(package)) {
        printf("%d %d\n", package[0], package[1]);
    }

    char *pkg_type = strsep(&package, ",");
    char *pkg_seq  = strsep(&package, ",");
    
    //ACK
    if (0 == strcmp(pkg_type, "ack")) {

        return;
    }

    //d
    char buf[1024];
    sprintf(buf, "ack,%s\r\n", pkg_seq);
    write(socket_fd, buf, strlen(buf));
     

}

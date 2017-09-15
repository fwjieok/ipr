#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <errno.h>

#include "session_tcp.h"

Session_tcp::Session_tcp(int socket_fd) {
	Session_tcp::socket_fd = socket_fd;

	memset(ring_buf,    0, sizeof(ring_buf));
	ringbuf_init(&rb, ring_buf, sizeof(ring_buf));

	package_len = 0;
	memset(package_buf, 0, sizeof(package_buf));
}

Session_tcp::~Session_tcp() {

}

char *Session_tcp::get_package_buf() {
	return (char *)package_buf;
}

void Session_tcp::do_process_data(char *buf, int len) {
	char ch = 0;
	ringbuf_in(&rb, buf, len);
	
	while (ringbuf_data_available(&rb) > 0) {
		ringbuf_out(&rb, &ch, 1);

		if ((ch == '\r' || ch == '\n') && package_len > 0) {
			do_process_package();
			memset(package_buf, 0, sizeof(package_buf));
			package_len = 0;
		} else {
			package_buf[package_len ++] = ch;
			package_buf[package_len]    = 0;
		}
	}
}

void Session_tcp::session_close() {
	shutdown(socket_fd, SHUT_RDWR);
	close(socket_fd);
	socket_fd = -1;
}

void Session_tcp::tick_1s() {

}
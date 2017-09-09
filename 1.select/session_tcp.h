#ifndef SESSION_TCP_H
#define SESSION_TCP_H

#include "ringbuf.h"
#include "session.h"

#define RING_BUF_SIZE    (8192*2)
#define PACKAGE_BUF_SIZE 4096

class Session_tcp: public Session {
private:

	struct ringbuf rb;
	char ring_buf[RING_BUF_SIZE];			//ringbuf

	int  package_len;
	char package_buf[PACKAGE_BUF_SIZE];		//网络接收缓冲

public:
	Session_tcp(int socket_fd);
	~Session_tcp();

	char *get_package_buf();
	virtual void do_process_package() = 0;

	void do_process_data(char *buf, int len);
	//void on_session_data();
	void session_close();
	void tick_1s();
};

#endif

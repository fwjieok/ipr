#ifndef SESSION_IPR_2_H
#define SESSION_IPR_2_H

#include "session_tcp.h"

class Session_ipr_2: public Session_tcp {
private:

public:
	Session_ipr_2(int socket_fd);
	~Session_ipr_2();

	void do_process_package();
};

#endif

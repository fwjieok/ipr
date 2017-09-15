#ifndef SESSION_H
#define SESSION_H

struct profile {
	char *tid;
	char *brand;
	char *model;
	char *ver;
	char *sn;

};

struct areas {
	char *acct;
	char *stat;
};

struct flags {
	char *ac_fail;
	char *low_battery;
	char *line_fail;
};

struct runtime {
	struct profile profile;
	struct areas   areas;
	struct flags   flags;
};

class Session {
private:

public:
	Session();
	virtual ~Session();

	int socket_fd;

	char *remote_addr;
	int   remote_port;
	
	struct runtime runtime;

	virtual void do_process_data(char *buf, int len) = 0;
	virtual void session_close()   = 0;
	virtual void tick_1s()         = 0;   

};

#endif
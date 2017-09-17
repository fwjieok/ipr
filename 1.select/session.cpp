#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "session.h"

Session::Session() {
	remote_addr = NULL;
	remote_port = -1;

    memset(&runtime, 0, sizeof(runtime));
}

Session::~Session() {
	if (remote_addr) {
		free(remote_addr);
	}

    if (runtime.areas.acct) {
        free(runtime.areas.acct);
    }
    if (runtime.profile.model) {
        free(runtime.profile.model);
    }
    if (runtime.profile.tid) {
        free(runtime.profile.tid);
    }
    if (runtime.profile.ver) {
        free(runtime.profile.ver);
    }
    if (runtime.netif) {
        free(runtime.netif);
    }
}

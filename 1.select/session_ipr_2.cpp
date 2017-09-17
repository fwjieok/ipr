#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "session_ipr_2.h"

Session_ipr_2::Session_ipr_2(int socket_fd):Session_tcp(socket_fd) {
}

Session_ipr_2::~Session_ipr_2() {
}

void Session_ipr_2::do_process_package() {	
	char *package = get_package_buf();
    if (package[0] == 0) {
        return;
    }
    // printf("[recv] %s\n", package);
    
    char *pkg_type = strsep(&package, ",");
    char *pkg_seq  = strsep(&package, ",");
    
    //ACK
    if (0 == strcmp(pkg_type, "ack")) {
    
    } else if ( 0 == strcmp(pkg_type, "d")) {
        char *cmd   = strsep(&package, ",");
        char *param = package;
        on_dev_cmd(cmd, param);
        
        char ack_buf[1024];
        sprintf(ack_buf, "\nack,%s\r", pkg_seq);
        write(socket_fd, ack_buf, strlen(ack_buf));
        printf("[send] %s\n", ack_buf + 1);
    }
}

void Session_ipr_2::on_dev_cmd(char *cmd, char *param) {
    printf("%s --- %s\n", cmd, param);

    if (0 == strcmp(cmd, "hi")) {
        on_dev_cmd_hi(param);
    } else if (0 == strcmp(cmd, "event")) {
        on_dev_cmd_event(param);
    } else if (0 == strcmp(cmd, "report")) {
        on_dev_cmd_report(param);
    }
}

void Session_ipr_2::on_dev_cmd_hi(char *param) {
    while(param != NULL) {
        char *field = strsep(&param, ",");
        char *name  = strsep(&field, ":");
        char *value = field;

        if (0 == strcmp(name, "id")) {
            if (runtime.areas.acct) {
                //ACCT改变
                if (0 != strcmp(runtime.areas.acct, value)) {
                    free(runtime.areas.acct);
                    runtime.areas.acct = value;
                }
            } else {
                runtime.areas.acct = strdup(value);
            }
        } else if (0 == strcmp(name, "sn")) {
            if (!runtime.profile.tid) {
                runtime.profile.tid = strdup(value);
            }
        } else if (0 == strcmp(name, "ver")) {
            if (!runtime.profile.ver) {
                runtime.profile.ver = strdup(value);
            }
        } else if (0 == strcmp(name, "netif")) {
            if (!runtime.netif) {
                runtime.netif = strdup(value);
            }
        } else if (0 == strcmp(name, "level")) {
            if (!runtime.level) {
                runtime.level = atoi(value);
            }
        }
    }
}

void Session_ipr_2::on_dev_cmd_event(char *param) {
    //printf("%s\n", param);

}

void Session_ipr_2::on_dev_cmd_report(char *param) {
    //printf("%s\n", param);

}


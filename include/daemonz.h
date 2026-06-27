#ifndef DAEMONZ_H
#define DAEMONZ_H

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

static pid_t pid;
#define DAEMONIZE(void)                            \
    do {                                           \
        pid = fork();                              \
        if (pid < 0) exit(EXIT_FAILURE);           \
        if (pid > 0) exit(EXIT_SUCCESS);           \
        if (setsid() < 0) exit(EXIT_FAILURE);      \
        pid = fork();                              \
        if (pid < 0) exit(EXIT_FAILURE);           \
        if (pid > 0) exit(EXIT_SUCCESS);           \
        umask(0);                                  \
        openlog("h4shfsmon", LOG_PID, LOG_DAEMON); \
    } while (0)

#endif /* ifndef DAEMONZ_H */

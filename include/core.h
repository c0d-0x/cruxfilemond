#ifndef CORE_H

#define CORE_H
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

#define LOG_FILE "cf_log.json"
#define LOCK_FILE "cf.lock"
#define CUSTOM_ERR (-1)

typedef enum {
    H4SH_OK = 0,
    H4SH_ERR = -1,
    H4SH_ERR_TIMEOUT = -2,
    H4SH_ERR_PROTOCOL = -3
} h4sh_status_t;

void fan_event_handler(int fan_fd, FILE *fp_log);
size_t check_lock(char *path_lock);
#endif

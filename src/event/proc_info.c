#include "logger.h"

#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syslog.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#include "json_gen.h"

char *get_user(const uid_t uid) {
    struct passwd *pws;
    if ((pws = getpwuid(uid)) != NULL) return (pws->pw_name);
    return NULL;
}

void get_proc_info(pid_t pid, char *buffer[], size_t buf_max) {
    char procfd_path[64] = {0x0};
    char buf_temp[64] = {0x0};
    FILE *proc_fd = NULL;
    size_t i = 0;

    snprintf(procfd_path, sizeof(procfd_path), "/proc/%d/status", pid);
    if (access(procfd_path, F_OK) != 0) {
        log_error("Effective process has terminated");
        return;
    }

    if ((proc_fd = fopen(procfd_path, "r")) == NULL) {
        log_error("Failed to open proc_fd: %s", strerror(errno));
        return;
    }

    while (fgets(buf_temp, sizeof(buf_temp), proc_fd) != NULL && i < buf_max) {
        buf_temp[strlen(buf_temp) - 1] = '\0';
        if (buf_temp[0] != '\0') buffer[i] = strdup(buf_temp);
        i++;
    }
    fclose(proc_fd);
}

json_obj_t *tokenizer(char *buffer[]) {
    size_t i = 0;
    char *saveptr = NULL;
    char *token = NULL;
    json_obj_t *json_obj = NULL;
    if ((json_obj = calloc(0x1, sizeof(json_obj_t))) == NULL) {
        log_error("Failed to allocate memory: %s", strerror(errno));
        return NULL;
    }

    while (i < 11) {
        if (buffer[i] != NULL) {
            token = strtok_r(buffer[i], ":\t\r ", &saveptr);
            if (token == NULL) {
                log_debug("Failed to load proc info\n");
                raise(SIGTERM);
            }

            if (strncmp(token, "Name", 4) == 0) {
                token = strtok_r(NULL, "\t ", &saveptr);
                json_obj->e_process = strdup(token);
            }

            if (strncmp(token, "Umask", 5) == 0) {
                token = strtok_r(NULL, "\t ", &saveptr);
                json_obj->e_p_Umask = strdup(token);
            }

            if (strncmp(token, "State", 5) == 0) {
                token = strtok_r(NULL, "\t ", &saveptr);
                json_obj->e_p_state = strdup(saveptr);
            }

            if (strncmp(token, "Uid", 3) == 0) {
                token = strtok_r(NULL, "\t", &saveptr);
                json_obj->e_username = strdup(get_user(atoi(token)));
            }
            // log_debug("&buffer[%ld]: %p", i, buffer[i]);
            // log_debug("&token: %p\n", token);
            free(buffer[i]);
            buffer[i] = NULL;
        }
        i++;
    }
    return json_obj;
}

void cleanup_procinfo(json_obj_t *log) {
    if (log != NULL) {
        if (log->date != NULL) free(log->date);
        if (log->e_username != NULL) free(log->e_username);
        if (log->e_process != NULL) free(log->e_process);
        if (log->e_p_state != NULL) free(log->e_p_state);
        if (log->e_p_Umask != NULL) free(log->e_p_Umask);
        free(log);
    }
}

char *get_locale_time(void) {
    char *buffer = NULL;
    if ((buffer = calloc(26, sizeof(char))) == NULL) {
        log_error("Failed to allocate memory: %s", strerror(errno));
        return NULL;
    }

    struct tm tm = *localtime(&(time_t){time(NULL)});
    asctime_r(&tm, buffer);
    if (buffer[0] == '\0') {
        free(buffer);
        return NULL;
    }

    buffer[strnlen(buffer, 26) - 1] = '\0';
    return buffer;
}

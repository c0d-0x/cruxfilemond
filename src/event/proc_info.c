#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syslog.h>
#include <syslog.h>
#include <time.h>

#include "debug.h"
#include "json_gen.h"

char *get_user(const uid_t uid) {
    struct passwd *pws;
    if ((pws = getpwuid(uid)) != NULL) return (pws->pw_name);
    return NULL;
}

json_obj_t *tokenizer(char *buffer[]) {
    size_t i = 0;
    char *saveptr;
    char *token;
    json_obj_t *json_obj = NULL;
    if ((json_obj = calloc(0x1, sizeof(json_obj_t))) == NULL) {
        DEBUG("Calloc Failed: %s", strerror(errno));
        return NULL;
    }

    while (i < 11) {
        if (buffer[i] != NULL) {
            /*DEBUG("proc_buffer[%ld]-> %s", i, buffer[i]);*/
            token = strtok_r(buffer[i], ":\t\r ", &saveptr);
            if (token == NULL) {
                DEBUG("Failed to load proc info\n");
                raise(SIGTERM);
            }

            if (strncmp(token, "Name", sizeof("Name")) == 0) {
                token = strtok_r(NULL, "\t ", &saveptr);
                json_obj->e_process = strdup(token);
            }

            if (strncmp(token, "Umask", sizeof("Umask")) == 0) {
                token = strtok_r(NULL, "\t ", &saveptr);
                json_obj->e_p_Umask = strdup(token);
            }

            if (strncmp(token, "State", sizeof("State")) == 0) {
                token = strtok_r(NULL, "\t ", &saveptr);
                json_obj->e_p_state = strdup(saveptr);
            }

            if (strncmp(token, "Uid", sizeof("Uid")) == 0) {
                token = strtok_r(NULL, "\t", &saveptr);
                json_obj->e_username = strdup(get_user(atoi(token)));
            }
            // DEBUG("&buffer[%ld]: %p", i, buffer[i]);
            // DEBUG("&token: %p\n", token);
            free(buffer[i]);
            buffer[i] = NULL;
        }
        i++;
    }
    return json_obj;
}

void cleanup_procinfo(json_obj_t *json_obj) {
    if (json_obj != NULL) {
        if (json_obj->date != NULL) free(json_obj->date);
        if (json_obj->e_username != NULL) free(json_obj->e_username);
        if (json_obj->e_process != NULL) free(json_obj->e_process);
        if (json_obj->e_p_state != NULL) free(json_obj->e_p_state);
        if (json_obj->e_p_Umask != NULL) free(json_obj->e_p_Umask);
        free(json_obj);
    }
}

char *get_locale_time(void) {
    char *buffer = NULL;
    if ((buffer = calloc(26, sizeof(char))) == NULL) {
        DEBUG("Failed to allocate memory: %s", strerror(errno));
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

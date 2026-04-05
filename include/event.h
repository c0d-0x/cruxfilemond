#ifndef EVENT_H
#include <stdbool.h>
#include <sys/epoll.h>

#include "json_gen.h"

void get_proc_info(pid_t pid, char *buffer[], size_t buf_max);
void cleanup_procinfo(json_obj_t *json_obj);
json_obj_t *tokenizer(char *buffer[]);

char *get_user(const uid_t uid);
bool modify_poll(int *epoll_fd, int fd, struct epoll_event *ev);
bool append_poll(int *epoll_fd, int fd, struct epoll_event *ev);
bool remove_poll(int *epoll_fd, int fd);
bool set_nonblocking(int fd);

#endif  // !EVENT_H

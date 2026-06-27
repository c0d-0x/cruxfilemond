#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>
#include "logger.h"

#include "core.h"
#include "event.h"

// TODO: migration to epoll for more extendibility
bool set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == H4SH_ERR) {
        log_error("Failed to get fd flags: %s", strerror(errno));
        return false;
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == H4SH_ERR) {
        log_error("Failed to set fd to non-blocking: %s", strerror(errno));
        return false;
    }
    return true;
}

bool append_poll(int *epoll_fd, int fd, struct epoll_event *ev) {
    if (epoll_ctl(*epoll_fd, EPOLL_CTL_ADD, fd, ev) == H4SH_ERR) {
        log_error("Failed to add fd to epoll list: %s", strerror(errno));
        return false;
    }
    return true;
}

bool remove_poll(int *epoll_fd, int fd) {
    if (epoll_ctl(*epoll_fd, EPOLL_CTL_DEL, fd, NULL) == H4SH_ERR) {
        log_error("Failed to remove fd to epoll list: %s", strerror(errno));
        return false;
    }
    return true;
}

bool modify_poll(int *epoll_fd, int fd, struct epoll_event *ev) {
    if (epoll_ctl(*epoll_fd, EPOLL_CTL_MOD, fd, ev) == H4SH_ERR) {
        log_error("Failed to modify fd in epoll list: %s", strerror(errno));
        return false;
    }
    return true;
}

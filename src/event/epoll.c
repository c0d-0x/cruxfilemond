#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "core.h"
#include "debug.h"
#include "event.h"

// TODO: migrate to epoll, finally
bool set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == H4SH_ERR) {
        DEBUG("Failed to get fd flags: %s", strerror(errno));
        return false;
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == H4SH_ERR) {
        DEBUG("Failed to set fd to non-blocking: %s", strerror(errno));
        return false;
    }
    return true;
}

bool append_poll(int *epoll_fd, int fd, struct epoll_event *ev) {
    if (epoll_ctl(*epoll_fd, EPOLL_CTL_ADD, fd, ev) == H4SH_ERR) {
        DEBUG("Failed to add fd to epoll list: %s", strerror(errno));
        return false;
    }
    return true;
}

bool remove_poll(int *epoll_fd, int fd) {
    if (epoll_ctl(*epoll_fd, EPOLL_CTL_DEL, fd, NULL) == H4SH_ERR) {
        DEBUG("Failed to remove fd to epoll list: %s", strerror(errno));
        return false;
    }
    return true;
}

bool sm3t_modify_poll(int *epoll_fd, int fd, struct epoll_event *ev) {
    if (epoll_ctl(*epoll_fd, EPOLL_CTL_MOD, fd, ev) == H4SH_ERR) {
        DEBUG("Failed to modify fd in epoll list: %s", strerror(errno));
        return false;
    }
    return true;
}

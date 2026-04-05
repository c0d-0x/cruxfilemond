#include "conf.h"
#include "debug.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

config_t *conf_parser(int conf_fd) {
    DEBUG("Loading watchlist from the CONFIG_FILE: %s", CONFIG_FILE);

    char cc;
    struct stat path_stat;
    size_t i = 0, watch_len = 0, len;
    uint8_t flag = 0;
    char buffer[PATH_MAX];
    config_t *conf;

    if (lseek(conf_fd, 0, SEEK_SET) == -1) return NULL;
    if ((conf = calloc(0x1, sizeof(config_t))) == NULL) {
        DEBUG("Calloc Failed: %s", strerror(errno));
        raise(SIGTERM);
    }

    while (watch_len < MAX_WATCH) {
        len = read(conf_fd, &cc, sizeof(char));
        if (len <= 0) {
            break;
        }

        if (cc == 0x20 || (cc == '\n' && i == 0)) continue;
        if (cc == '\n') {
            if (stat(buffer, &path_stat) != 0) {
                DEBUG("%s -> Invalid File or Folder", buffer);
                i = 0;
                continue;
            }

            if (path_stat.st_mode & S_IFDIR) flag = F_IS_DIR;
            else if (path_stat.st_mode & S_IFREG) {
                flag = F_IS_FILE;
            }

            i = 0;
            (conf->watchlist[watch_len].path) = strdup(buffer);
            conf->watchlist[watch_len].f_type = flag;
            conf->watchlist_len = ++watch_len;
            memset(buffer, '\0', strnlen(buffer, PATH_MAX));
        } else buffer[i++] = cc;
    }

    if (conf->watchlist_len == 0) {
        free(conf);
        conf = NULL;
        return NULL;
    }

    return conf;
}

void conf_cleanup(config_t *conf) {
    DEBUG("watchlist clean up");
    for (size_t i = 0; i < conf->watchlist_len; i++) {
        free(conf->watchlist[i].path);
    }
    free(conf);
    conf = NULL;
}

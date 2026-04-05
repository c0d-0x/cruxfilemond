#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <sys/types.h>

#define CONFIG_FILE ".config/cf.conf"
#define CF_HOME_DIR ".config"
#define VERSION "v1.0.0"
#define MAX_WATCH 200

#define F_NT_FND -1
#define F_IS_FILE 0
#define F_IS_DIR 1

typedef struct {
    size_t f_type;
    char *path;
} watch_t;

typedef struct {
    size_t watchlist_len;
    watch_t watchlist[MAX_WATCH];
} config_t;

int init_inotify(char *file_path);
config_t *inotify_event_handler(int inotify_fd, int config_fd, config_t *(*handler)(int config_fd));

config_t *conf_parser(int config_fd);
void conf_cleanup(config_t *config_obj);

#endif  // !CONFIG_H

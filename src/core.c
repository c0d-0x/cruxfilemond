#include "core.h"

#include <linux/fanotify.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "debug.h"
#include "json_gen.h"
#include "main.h"
#include "proc_info.h"

config_t *parse_config_file(int config_fd) {
  /*DEBUG("Loading watchlist from the CONFIG_FILE: ", CONFIG_FILE);*/

  char CC;
  struct stat path_stat;
  size_t i = 0, F_Flag, watch_len = 0, len;
  char buffer[PATH_MAX];
  config_t *config_obj;

  if (lseek(config_fd, 0, SEEK_SET) == -1) return NULL;
  if ((config_obj = calloc(0x1, sizeof(config_t))) == NULL) {
    DEBUG("Calloc Failed: %s", strerror(errno));
    kill(getpid(), SIGTERM);
  }
  /*[TODO]: fix with fgets() instead of read() */
  while (watch_len < MAX_WATCH) {
    len = read(config_fd, &CC, sizeof(char));
    if (len <= 0) {
      break;
    }
    if (CC == 0x20 || (CC == '\n' && i == 0)) continue;
    if (CC == '\n') {
      if (stat(buffer, &path_stat) != 0) {
        DEBUG("%s -> Invalid File or Folder", buffer);
        i = 0;
        continue;
      }

      if (path_stat.st_mode & S_IFDIR) {
        F_Flag = F_IS_DIR;
      } else if (path_stat.st_mode & S_IFREG) {
        F_Flag = F_IS_FILE;
      }

      i = 0;
      (config_obj->watchlist[watch_len].path) = strdup(buffer);
      config_obj->watchlist[watch_len].f_type = F_Flag;
      config_obj->watchlist_len = ++watch_len;
      memset(buffer, '\0', strnlen(buffer, PATH_MAX));
    } else
      buffer[i++] = CC;
  }

  if (config_obj->watchlist_len == 0) {
    free(config_obj);
    config_obj = NULL;
    return NULL;
  }
  return config_obj;
}

void config_obj_cleanup(config_t *config_obj) {
  DEBUG("watchlist clean up");
  for (size_t i = 0; i < config_obj->watchlist_len; i++) {
    free(config_obj->watchlist[i].path);
  }
  free(config_obj);
  config_obj = NULL;
}

size_t check_lock(char *path_lock) {
  if (access(path_lock, F_OK) == 0) {
    fprintf(stderr, "An instance of cruxfilemond is already running\n");
    fprintf(stderr,
            "If no cruxfilemond instance is running, Delete '%s' file \n",
            LOCK_FILE);
    return CUSTOM_ERR;
  }

  DEBUG("No instance of cruxfilemond running");
  FILE *fp_lock = NULL;
  if ((fp_lock = fopen(path_lock, "w")) == NULL) {
    perror("Could not create lock file");
    return CUSTOM_ERR;
  }
  fclose(fp_lock);
  return EXIT_SUCCESS;
}

void fan_event_handler(int fan_fd, FILE *fp_log) {
  const struct fanotify_event_metadata *metadata;
  struct fanotify_event_metadata buf[200] = {0x0};
  char *buffer[11] = {NULL};
  ssize_t len;
  char path[PATH_MAX] = {0x0};
  json_obj_t *json_obj;
  ssize_t path_len, p_event;
  char procfd_path[PATH_MAX] = {0x0};
  // struct fanotify_response response;

  while (true) {
    /* Read some events. */
    len = read(fan_fd, buf, sizeof(buf));
    if (len == -1 && errno != EAGAIN) {
      DEBUG("Failed to read fan_events");
      kill(getpid(), SIGTERM);
    }

    /* Check if end of available data reached. */
    if (len <= 0) break;

    /* Point to the first event in the buffer. */

    metadata = buf;

    /* Loop over all events in the buffer. */

    while (FAN_EVENT_OK(metadata, len)) {
      /* Check that run-time and compile-time structures match. */

      if (metadata->vers != FANOTIFY_METADATA_VERSION) {
        DEBUG("Mismatch of fanotify metadata version");
        kill(getpid(), SIGTERM);
      }

      /* metadata->fd contains either FAN_NOFD, indicating a
         queue overflow, or a file descriptor (a nonnegative
         integer). Here, queue overflow is simply ignored. */

      /*if (metadata->fd == FAN_NOFD) {*/
      /*TODO: Handle queue overflow !!*/
      /*}*/

      if (metadata->fd >= 0) {
        /* Handle open permission event. */
        if (metadata->mask & FAN_OPEN) {
          p_event = FAN_OPEN;
          get_proc_info(metadata->pid, buffer, 11);

        } else if (metadata->mask & FAN_MODIFY) {
          p_event = FAN_MODIFY;
          get_proc_info(metadata->pid, buffer, 11);
        }

        /* Retrieve and print pathname of the accessed file. */
        snprintf(procfd_path, sizeof(procfd_path), "/proc/self/fd/%d",
                 metadata->fd);
        path_len = readlink(procfd_path, path, sizeof(path) - 1);
        if (path_len == -1) {
          DEBUG("readlink: %s", strerror(errno));
          kill(getpid(), SIGTERM);
        }

        path[path_len] = '\0';

        if ((json_obj = tokenizer(buffer)) == NULL) {
          DEBUG("Failed to load effective process's info");
          kill(getpid(), SIGTERM);
        }

        json_obj->file = path;
        json_obj->e_p_event =
            (p_event == FAN_MODIFY) ? "FILE MODIFIED" : "FILE ACCESSED";

        json_obj->date = get_locale_time();
        DEBUG("umask: %s", json_obj->e_p_Umask);
        DEBUG("Process: %s", json_obj->e_process);
        DEBUG("Process State: %s", json_obj->e_p_state);
        DEBUG("Username: %s", json_obj->e_username);
        DEBUG("Event registered: %s", json_obj->e_p_event);
        DEBUG("FILE: %s\n", json_obj->file);

        /*[TODO:] Logging procinfo to a json format is really slow. To be
         * FIXED*/
        /*write_json_obj(fp_log, json_obj, json_constructor);*/
        write_json_fmt(fp_log,
                       "{\"date\":\"%s\",\"file\":\"%s\","
                       "\"process\":\"%s\",\"event\":\"%s\",\"state\":\"%s\","
                       "\"umask\":\"%s\",\"username\":\"%s\"}",
                       json_obj->date, json_obj->file, json_obj->e_process,
                       json_obj->e_p_event, json_obj->e_p_state,
                       json_obj->e_p_Umask, json_obj->e_username);
        cleanup_procinfo(json_obj);
        close(metadata->fd);
      }
      /* Advance to next event. */
      metadata = FAN_EVENT_NEXT(metadata, len);
    }
  }
  /*flushing the file buffer, after writing.*/
  fflush(fp_log);
}

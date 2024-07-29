#include "filemond.h"
#include <err.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

config_t *load_config_file(char *file_Path) {
  struct stat path_status;
  size_t i = 0, index_n = -1, F_Flag;
  char buffer[PATH_MAX], *tok;
  config_t *config_obj;
  FILE *fp_config = NULL;

  if (access(file_Path, F_OK) != 0) {
    fprintf(stderr, "Config file not found!!\n");
    exit(EXIT_FAILURE);
  }

  if ((fp_config = fopen(file_Path, "r")) == NULL) {
    perror("Fail to open Config file\n");
    return NULL;
  }

  config_obj = malloc(sizeof(config_t));
  while (fgets(buffer, sizeof(buffer), fp_config) != NULL && errno != EOF &&
         i < MAX_WATCH) {

    if ((tok = strchr(buffer, '\n')) != NULL) {
      index_n = tok - buffer;
      buffer[index_n] = '\0';
    }

    if (stat(buffer, &path_status) == 0) {
      if (path_status.st_mode & S_IFDIR) {
        F_Flag = F_IS_DIR;
      } else if (path_status.st_mode & S_IFREG) {
        F_Flag = F_IS_FILE;
      } else {
        continue;
      }
    } else {
      fclose(fp_config);
      fprintf(stderr, "%s\n -> Invalid input from the config\n", buffer);
      exit(EXIT_FAILURE);
    }

    config_obj->watchlist_len = i + 1;
    config_obj->watchlist[i].F_TYPE = F_Flag;
    (config_obj->watchlist[i].path) = strdup(buffer);
    memset(buffer, '\0', strlen(buffer));
    i++;
  }
  fclose(fp_config);
  return config_obj;
}

void config_obj_cleanup(config_t *config_obj) {
  for (size_t s = 0; s < config_obj->watchlist_len; s++) {
    free(config_obj->watchlist[s].path);
  }
  free(config_obj);
}

size_t check_lock(char *path_lock) {
  if (access(path_lock, F_OK) != 0) {
    pid_t pid = getpid();
    FILE *fp_lock = NULL;
    if ((fp_lock = fopen(path_lock, "w")) == NULL) {
      perror("Could not create lock file");
      return CUSTOM_ERR;
    }
    fprintf(fp_lock, "%d", pid);
    fclose(fp_lock);
    return EXIT_SUCCESS;
  }

  fprintf(stderr,
          "An instance of cruxfilemond is already running\n"
          "If no cruxfilemond instance is running, Delete '%s' file \n",
          LOCK_FILE);
  return CUSTOM_ERR;
}

// [TODO]: To be implemented

static int write_log(int log_fd, char *path_log, char **buf) {
  // tokenize buf and structure it for writing to the log file.
  // - process name and uid for the user.
  // -file path as well

  return 0;
}

void fan_event_handler(int fan_fd) {}

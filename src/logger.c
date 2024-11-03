#include "logger.h"

#include <stdio.h>

#include "json_gen.h"

// #include "config.h"

void proc_info(pid_t pid, char *buffer[], size_t buf_max) {
  char procfd_path[32] = {0x0};
  char buf_temp[64] = {0x0};
  char *tok = NULL;
  FILE *proc_fd = NULL;
  size_t index_n, i = 0;

  snprintf(procfd_path, sizeof(procfd_path), "/proc/%d/status", pid);
  if (access(procfd_path, F_OK) == 0) {
    if ((proc_fd = fopen(procfd_path, "r")) == NULL) {
      syslog(LOG_ERR, "Failed to open proc_fd: %s", strerror(errno));
      return;
    }

    while (fgets(buf_temp, sizeof(buf_temp), proc_fd) != NULL && i < buf_max) {
      if ((tok = strchr(buf_temp, '\n')) != NULL) {
        index_n = tok - buf_temp;
        buf_temp[index_n] = '\0';
      }
      buffer[i] = strdup(buf_temp);
      i++;
    }
    fclose(proc_fd);
  }
}

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
    syslog(LOG_ERR, "Calloc Failed: %s", strerror(errno));
    return NULL;
  }

  while (buffer[i] != NULL && i < 11) {
    printf("%s\n", buffer[i]);
    token = strtok_r(buffer[i], ":\t\r ", &saveptr);
    if (token == NULL) {
      syslog(LOG_ERR, "Failed to load_proc_info\n");
      exit(CUSTOM_ERR);
    }

    printf(" token: %s\n", token);
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
    free(buffer[i]);
    i++;
  }
  return json_obj;
}

void cleanup_procinfo(json_obj_t *json_obj) {
  if (json_obj != NULL) {
    if (json_obj->e_username != NULL) free(json_obj->e_username);
    if (json_obj->e_process != NULL) free(json_obj->e_process);
    if (json_obj->e_p_state != NULL) free(json_obj->e_p_state);
    if (json_obj->e_p_Umask != NULL) free(json_obj->e_p_Umask);
  }
}

int push_stk(cus_stack_t **head, json_obj_t *data) {
  cus_stack_t *node = NULL;
  if ((node = (cus_stack_t *)malloc(sizeof(cus_stack_t))) == NULL) {
    perror("Malloc Failed");
    return -1;
  }

  node->data = data;
  node->next = (*head);
  (*head) = node;
  return 0;
}

cus_stack_t *pop_stk(cus_stack_t **head) {
  if ((*head) == NULL) {
    return NULL;
  }

  cus_stack_t *node = (*head);
  (*head) = (*head)->next;
  node->next = NULL;
  return node;
}

void get_locale_time(char *buffer) {
  if (buffer == NULL) return;
  struct tm tm = *localtime(&(time_t){time(NULL)});
  asctime_r(&tm, buffer);
  buffer[strnlen(buffer, 26) - 1] = '\0';
}

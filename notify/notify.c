#define _DEFAULT_SOURCE
#include "notify.h"

#include <libnotify/notification.h>
#include <libnotify/notify.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/*#include "config.h"*/
/*#include "debug.h"*/
/*#include "filemond.h"*/
/*#include "logger.h"*/
/**/
/*static char* notification_icon;*/
static NotifyNotification* notification;

static char* get_user_env_var(const char* username, const char* var_name) {
  FILE* fp;
  char command[256] = {0x0};
  static char result[1024] = {0x0};

  snprintf(command, sizeof(command),
           "ps -u %s -o pid= | while read pid; do tr '\\0' '\\n' < "
           "/proc/$pid/environ 2>/dev/null | grep '^%s='; done | head -n 1 | "
           "cut -d '=' -f2-",
           username, var_name);

  /*bash command injection to retrieve env_var*/
  if ((fp = popen(command, "r")) == NULL) {
    perror("Failed to run command");
    return NULL;
  }

  if (fgets(result, sizeof(result), fp) == NULL) {
    pclose(fp);
    return NULL;
  }

  pclose(fp);
  result[strcspn(result, "\n")] = 0;  // Remove newline
  return result;
}

void initialize_notify(char* appname, char* icon, size_t expires) {
  if (!notify_is_initted()) {
    // Get user information
    /*struct passwd* pw = getpwnam(USERNAME);*/
    /*if (pw == NULL) {*/
    /*  fprintf(stderr, "User %s not found.\n", USERNAME);*/
    /*  exit(CUSTOM_ERR);*/
    /*}*/
    /**/
    // Get the UID and GID of the active user
    /*uid_t user_uid = pw->pw_uid;*/
    /*gid_t user_gid = pw->pw_gid;*/

    // Get the DISPLAY and DBUS_SESSION_BUS_ADDRESS environment variables for
    // the user
    /*char* display = get_user_env_var(USERNAME, "DISPLAY");*/
    /*char* dbus_session = get_user_env_var(USERNAME,
     * "DBUS_SESSION_BUS_ADDRESS");*/

    /*if (display == NULL || dbus_session == NULL) {*/
    /*  fprintf(stderr, "Failed to get environment variables for user %s.\n",*/
    /*          USERNAME);*/
    /*  exit(CUSTOM_ERR);*/
    /*}*/
    /**/
    // Print the extracted environment variables for debugging purposes
    /*DEBUG("DISPLAY-> ", display);*/
    /*DEBUG("DBUS_SESSION_BUS_ADDRESS-> ", dbus_session);*/
    /**/
    // Set the environment variables for the root process
    /*setenv("DISPLAY", display, 1);*/
    /*setenv("DBUS_SESSION_BUS_ADDRESS", dbus_session, 1);*/
    /**/
    // Switch to the non-root user's UID and GID
    /*if (setgid(user_gid) != 0 || setuid(user_uid) != 0) {*/
    /*  perror("Failed to switch user");*/
    /*  exit(CUSTOM_ERR);*/
    /*}*/
    /**/
    if (!notify_init("Cruxfilemond")) {
      /*DEBUG("Failed to initialize libnotify\n", NULL);*/
      fprintf(stderr, "Failed to initialize libnotify\n");
      exit(EXIT_FAILURE);
    }

    notification = notify_notification_new("", NULL, icon);
    if (notification == NULL) {
      fprintf(stderr, "Fail to create a notification instance\n");
      exit(EXIT_FAILURE);
    }
    notify_notification_set_timeout(notification, expires);
  }
}

// static char * get_file_name(char * file_path){
// TODO: extracts a file name from a file path
// }

void notify_send_msg(const char* file_path, const char* p_event,
                     ssize_t ugency) {
  if (notification == NULL) {
    /*DEBUG("Failed to create a notify instance\n", NULL);*/
    fprintf(stderr, "Failed to create a notify instance\n");
    kill(getpid(), SIGTERM);  // Terminates the process safely on failure
    /*return;  // Ensure function exits if kill doesn't stop the process*/
  }

  char* file_name = NULL;
  char buff[32] = {0};

  // Ensure that procinfo->file_path is not NULL before attempting to use it
  if (p_event == NULL || file_path == NULL) {
    /*DEBUG("Invalid procinfo or file path\n", NULL);*/
    fprintf(stderr, "Invalid procinfo or file path\n");
    kill(getpid(), SIGTERM);  // Terminates the process safely on failure
  }

  // Extract the filename from the path without modifying the original string
  file_name = strrchr(file_path, '/');
  if (file_name == NULL) {
    /*DEBUG("Failed to extract a valid file name from the path\n", NULL);*/
    fprintf(stderr, "Failed to extract a valid file name from the path\n");
    /*kill(getpid(), SIGTERM);  // Terminates the process safely on failure*/
    /*}*/

    snprintf(buff, sizeof(buff), "%s: %s", p_event, file_name + 1);
    notify_notification_update(notification, buff,
                               "Check cf.log for Cruxfilemond event",
                               "dialog-information");

    notify_notification_set_urgency(notification, ugency);
    if (!notify_notification_show(notification, NULL)) {
      kill(getpid(), SIGTERM);  // Terminates the process safely on failure
    }
  }
}
void cleanup_notify() {
  if (notify_is_initted()) {
    g_object_unref(G_OBJECT(notification));
    notify_uninit();
  }
}

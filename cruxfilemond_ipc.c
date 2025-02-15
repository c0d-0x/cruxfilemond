#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LOG_FILE "./cf_log.json"
#define LOCK_FILE "cf.lock"
#define CUSTOM_ERR (-1)
#define UPDATE "-u"
#define TERMT "-t"
#define DUMP "-d"

void help(char *prog) {
  fprintf(stderr, "usage:%s < -option >\n", prog);
  printf(
      "\tOptions\n\t %s: Sends a sighup to update cruxfilemond "
      "config\n\t %s: sends a sigterm to terminate "
      "cruxfilemond\n\t %s: dumps cf.log\n",
      UPDATE, TERMT, DUMP);
}

size_t option(char *opt) {
  ssize_t valid_opt = -1;
  if (strncmp(opt, UPDATE, strnlen(UPDATE, 3)) == 0) valid_opt = SIGHUP;
  if (strncmp(opt, TERMT, strnlen(TERMT, 3)) == 0) valid_opt = SIGTERM;
  if (strncmp(opt, DUMP, strnlen(DUMP, 3)) == 0) valid_opt = SIGUSR1;

  return valid_opt;
}

void dump_log(void) {
  FILE *fp_log = fopen(LOG_FILE, "rb");
  if (!fp_log) {
    fprintf(stderr, "Failed to open '%s' log file: %s\n", LOG_FILE,
            strerror(errno));
    return;
  }
  while (1) {
    int cc = getc(fp_log);
    if (cc == EOF) break;
    putc(cc, stdout);
  }
  fclose(fp_log);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    help(argv[0]);
    return EXIT_FAILURE;
  }

  ssize_t sig_to_send = -1;
  char buf[64] = {0x0};
  int fd_lock;
  int pid_cruxfilemond;
  fd_lock = open(LOCK_FILE, O_RDONLY);
  if (fd_lock == -1) {
    perror("Fail to open LOCK_FILE");
    return EXIT_FAILURE;
  }

  if (read(fd_lock, buf, sizeof(int64_t)) <= 0) {
    fprintf(stderr, "Failed to read, %s: %s", LOCK_FILE, strerror(errno));
    exit(EXIT_FAILURE);
  }

  close(fd_lock);
  pid_cruxfilemond = atoi(buf);
  if (pid_cruxfilemond < 2) {
    help(argv[0]);
    return EXIT_FAILURE;
  }

  sig_to_send = option(argv[1]);
  if (sig_to_send == -1) {
    fprintf(stderr, "Invalid option\n");
    help(argv[0]);
    exit(EXIT_FAILURE);
  }

  if (sig_to_send == SIGUSR1) {
    dump_log();
    return EXIT_SUCCESS;
  }
  if (kill(pid_cruxfilemond, sig_to_send) != 0) {
    fprintf(stderr, "Failed to send signal: %s\n", strerror(errno));
    return CUSTOM_ERR;
  }

  fprintf(stdout, "signal %ld sent to PID: %d!!\n", sig_to_send,
          pid_cruxfilemond);
  return EXIT_SUCCESS;
}

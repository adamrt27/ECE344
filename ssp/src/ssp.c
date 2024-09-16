#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <unistd.h>

void ssp_init() {
}

int ssp_create(char *const *argv, int fd0, int fd1, int fd2) {
    return -1;
}

int ssp_get_status(int ssp_id) {
    return -1;
}

void ssp_send_signal(int ssp_id, int signum) {
}

void ssp_wait() {
}

void ssp_print() {
}

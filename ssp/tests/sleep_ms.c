#include "sleep_ms.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void sleep_ms(uint32_t milliseconds) {
    struct timespec spec = {
        .tv_sec = milliseconds / 1000,
        .tv_nsec = (milliseconds % 1000) * 1000000,
    };
    int ret;
    while ((ret = nanosleep(&spec, &spec)) != 0) {
        if (ret == -1) {
            if (errno == EINTR) {
                continue;
            }
            int error = errno;
            perror("nanosleep");
            exit(error);
        }
    }
}

#include "ssp.h"

#include "sleep_ms.h"

#include <assert.h>
#include <fcntl.h>
#include <stddef.h>

int main() {
    ssp_init();

    int fd = open("/dev/null", O_RDWR);

    char *const ls_argv[] = {
        "ls",
        "-l",
        NULL,
    };
    int ls_id[4];
    for (int i = 0; i < 4; ++i) {
        ls_id[i] = ssp_create(ls_argv, fd, fd, fd);
    }

    ssp_wait();
    ssp_print();

    for (int i = 0; i < 4; ++i) {
        assert(ssp_get_status(ls_id[i]) == 0);
    }

    return 0;
}

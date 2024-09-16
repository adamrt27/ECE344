#include "ssp.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <unistd.h>

int main() {
    ssp_init();

    if (signal(SIGPIPE, SIG_DFL) == SIG_ERR) {
        exit(errno);
    }

    int p1[2];
    int ret = pipe(p1);
    if (ret == -1) {
        exit(errno);
    }

    int p2[2];
    ret = pipe(p2);
    if (ret == -1) {
        exit(errno);
    }

    char *const cat_argv[] = {
        "cat",
        NULL,
    };
    int cat_id = ssp_create(cat_argv, p1[0], p2[1], 2);

    close(p2[0]);
    char bye[] = "Bye";
    write(p1[1], bye, sizeof(bye));

    ssp_wait();
    int status = ssp_get_status(cat_id);
    ssp_print();
    assert(status == 141);

    return 0;
}

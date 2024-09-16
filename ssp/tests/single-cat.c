#include "ssp.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    ssp_init();

    char *const cat_argv[] = {
        "cat",
        NULL,
    };

    int pipedes[2];
    int err = pipe(pipedes);
    if (err == -1) {
        exit(errno);
    }
    int cat_id = ssp_create(cat_argv, pipedes[0], 1, 2);
    close(pipedes[0]);

    int status = ssp_get_status(cat_id);
    ssp_print();
    assert(status == -1);

    close(pipedes[1]);

    ssp_wait();
    status = ssp_get_status(cat_id);
    ssp_print();
    assert(status == 0);

    return 0;
}

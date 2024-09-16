#include "ssp.h"

#include "sleep_ms.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
    ssp_init();

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

    int p3[2];
    ret = pipe(p3);
    if (ret == -1) {
        exit(errno);
    }

    char *const echo_argv[] = {
        "echo",
        "Testing",
        NULL,
    };
    int echo_id = ssp_create(echo_argv, 0, p1[1], 2);

    char *const cat_argv[] = {
        "cat",
        NULL,
    };
    int cat1_id = ssp_create(cat_argv, p1[0], p2[1], 2);
    int cat2_id = ssp_create(cat_argv, p2[0], p3[1], 2);


    char expected[] = { 'T', 'e', 's', 't', 'i', 'n', 'g', '\n'};

    close(p1[0]);
    close(p1[1]);
    close(p2[0]);
    close(p2[1]);
    close(p3[1]);

    char buffer[4096];
    int bytes_read = read(p3[0], buffer, sizeof(buffer));
    assert(bytes_read == sizeof(expected));
    assert(memcmp(buffer, expected, sizeof(expected)) == 0);
    bytes_read = read(p3[0], buffer, sizeof(buffer));
    assert(bytes_read == 0);
    close(p3[0]);

    ssp_wait();
    ssp_print();

    assert(ssp_get_status(echo_id) == 0);
    assert(ssp_get_status(cat1_id) == 0);
    assert(ssp_get_status(cat2_id) == 0);

    return 0;
}

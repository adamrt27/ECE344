#include "ssp.h"

#include <assert.h>
#include <stddef.h>

int main() {
    ssp_init();

    char *const sleep_argv[] = {
        "sleep",
        "30",
        NULL,
    };
    int sleep_id = ssp_create(sleep_argv, 0, 1, 2);

    ssp_print();

    assert(ssp_get_status(sleep_id) == -1);

    ssp_send_signal(sleep_id, 9);
    ssp_wait();
    ssp_print();

    assert(ssp_get_status(sleep_id) == 137);

    return 0;
}

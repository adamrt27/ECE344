#include "ssp.h"

#include "sleep_ms.h"

#include <assert.h>
#include <stddef.h>

int main() {
    ssp_init();

    char *const true_argv[] = {
        "true",
        NULL,
    };
    int true_id = ssp_create(true_argv, 0, 1, 2);

    char *const false_argv[] = {
        "false",
        NULL,
    };
    int false_id = ssp_create(false_argv, 0, 1, 2);

    ssp_wait();
    ssp_print();

    assert(ssp_get_status(true_id) == 0);
    assert(ssp_get_status(false_id) == 1);

    return 0;
}

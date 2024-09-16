#include "ssp.h"

#include "sleep_ms.h"

#include <assert.h>
#include <stddef.h>

int main() {
    ssp_init();

    char *const sleep_argv[] = {
        "sleep",
        "0.01",
        NULL,
    };
    int sleep_id = ssp_create(sleep_argv, 0, 1, 2);

    int status = ssp_get_status(sleep_id);

    ssp_print();

    assert(status == -1);

    sleep_ms(100);

    status = ssp_get_status(sleep_id);

    ssp_print();

    assert(status == 0);

    return 0;
}

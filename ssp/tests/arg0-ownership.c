#include "ssp.h"

#include "sleep_ms.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

int main() {
    ssp_init();

    char arg0[] = "true";
    char *const true_argv[] = {
        arg0,
        NULL,
    };
    ssp_create(true_argv, 0, 1, 2);

    strcpy(arg0, "haha");

    ssp_wait();
    ssp_print();

    return 0;
}

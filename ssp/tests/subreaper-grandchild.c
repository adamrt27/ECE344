#include "ssp.h"

#include "sleep_ms.h"

#include <unistd.h>

int main() {
    ssp_init();

    pid_t pid = fork();
    if (pid == 0) {
        pid = fork();
        if (pid == 0) {
            sleep_ms(100);
            return 65;
        }
        return 64;
    }

    sleep_ms(50);

    ssp_print();

    sleep_ms(100);

    ssp_print();

    return 0;
}

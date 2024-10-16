#include "test.h"

#include <errno.h> // errno
#include <stdio.h> // dprintf
#include <sys/mman.h> // mmap
#include <sys/wait.h> // wait
#include <unistd.h> // fork

int* shared_memory = NULL;
static int failed_check = 0;

void expect(int got, int expected, const char* message) {
    if (got == expected) {
        return;
    }

    if (got == TEST_MAGIC) {
        dprintf(2, "%s\n    line never reached\n", message);
        failed_check = 1;
    }
    else if (got != expected) {
        dprintf(2, "%s\n    got: %d\n    expected: %d\n",
                message, got, expected);
        failed_check = 1;
    }
}

int main(void) {
    shared_memory = mmap(NULL, 4096 * 10, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if (shared_memory == MAP_FAILED) {
        exit(errno);
    }

    for (int i = 0; i < 10240; ++i) {
        shared_memory[i] = TEST_MAGIC;
    }

    pid_t pid = fork();
    if (pid == 0) {
        test();
        exit(0);
    }
    else if (pid == -1) {
        exit(errno);
    }
    int wstatus;
    wait(&wstatus);
    check();
    munmap(shared_memory, 4096 * 10);
    if (failed_check) {
        exit(1);
    }
    assert(WIFEXITED(wstatus));
    assert(WEXITSTATUS(wstatus) == 0);
    return 0;
}

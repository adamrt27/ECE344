#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int expected_exit_status(void);
void test(void);

int main(void) {
    int pid = fork();
    if (pid == -1) {
        exit(errno);
    }
    else if (pid == 0) {
        test();
        exit(0);
    }
    int wstatus;
    if (wait(&wstatus) != pid) {
        exit(1);
    }
    assert(WIFEXITED(wstatus));
    assert(WEXITSTATUS(wstatus) == expected_exit_status());
    return 0;
}

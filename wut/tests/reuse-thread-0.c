#include "test.h"

#include "wut.h"

static int x = 0;

void t0_run(void) {
    ++x;
}

void t1_run(void) {
    shared_memory[3] = wut_join(0);
    int id = wut_create(t0_run);
    shared_memory[4] = id;
    shared_memory[5] = wut_join(id);
    shared_memory[6] = x;
}

void test(void) {
    wut_init();
    shared_memory[0] = wut_id();
    int id = wut_create(t1_run);
    shared_memory[1] = id;
    shared_memory[2] = wut_yield();
    wut_exit(0);
}

void check(void) {
    expect(
        shared_memory[0], 0, "wut_id of the main thread is wrong"
    );
    expect(
        shared_memory[1], 1, "wut_id of the first thread is wrong"
    );
    expect(
        shared_memory[2], 0, "wut_yield should be successful"
    );
    expect(
        shared_memory[3], 0, "wut_join should be successful"
    );
    expect(
        shared_memory[4], 0, "wut_id of the second thread is wrong"
    );
    expect(
        shared_memory[5], 0, "wut_join should be successful"
    );
    expect(
        shared_memory[6], 1, "one thread should write to x"
    );
}

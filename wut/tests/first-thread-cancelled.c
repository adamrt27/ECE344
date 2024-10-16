#include "test.h"

#include "wut.h"

static int x = 0;

void run(void) {
    x = 1;
}

void test(void) {
    wut_init();
    shared_memory[0] = wut_id();
    int id = wut_create(run);
    shared_memory[1] = id;
    shared_memory[2] = wut_cancel(id);
    shared_memory[3] = wut_yield();
    shared_memory[4] = x;
}

void check(void) {
    expect(
        shared_memory[0], 0, "wut_id of the main thread is wrong"
    );
    expect(
        shared_memory[1], 1, "wut_id of the first thread is wrong"
    );
    expect(
        shared_memory[2], 0, "wut_cancel should be successful"
    );
    expect(
        shared_memory[3], -1, "wut_yield should fail if there's only one thread"
    );
    expect(
        shared_memory[4], 0, "the thread should never write to x"
    );
}


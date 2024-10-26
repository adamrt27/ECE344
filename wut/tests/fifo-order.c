#include "test.h"

#include "wut.h"

#define NUM_THREADS 3

static int x = 0;

void run_t2(void) {
    if (x == 2) {
        ++x;
    }
    wut_exit(2);
}

void run_t1(void) {
    if (x == 1) {
        ++x;
    }
    shared_memory[3] = wut_join(2);
    shared_memory[4] = wut_cancel(2);
    if (x == 4) {
        ++x;
    }
}

void test(void) {
    wut_init();
    shared_memory[1] = wut_create(run_t1);
    shared_memory[2] = wut_create(run_t2);
    if (x == 0) {
        ++x;
    }
    wut_yield();
    if (x == 3) {
        ++x;
    }
    wut_yield();
    if (x == 5) {
        ++x;
    }
    shared_memory[5] = wut_yield();

    shared_memory[0] = x;
}

void check(void) {
    expect(
        shared_memory[0], 6, "threads should be fifo order"
    );
    expect(
        shared_memory[1], 1, "thread 1 should be id 1"
    );
    expect(
        shared_memory[2], 2, "thread 2 should be id 2"
    );
    expect(
        shared_memory[3], 2, "join should return 2"
    );
    expect(
        shared_memory[4], -1, "thread 2 already joined"
    );
    expect(
        shared_memory[5], -1, "final yield should be -1"
    );
}

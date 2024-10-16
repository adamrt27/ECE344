#include "test.h"

#include "wut.h"

static int x = 0;

void t3_run(void);

void t1_run(void) {
    if (x == 0) {
        x = 1;
        shared_memory[3] = x;
    }
    shared_memory[4] = wut_yield();
}

void t2_run(void) {
    if (x == 1) {
        x = 2;
        shared_memory[5] = x;
    }
    shared_memory[6] = wut_cancel(1);
    shared_memory[7] = wut_create(t3_run);
}

void t3_run(void) {
    if (x == 3) {
        x = 4;
        shared_memory[10] = x;
    }
}

void test(void) {
    wut_init();
    shared_memory[0] = wut_create(t1_run);
    shared_memory[1] = wut_create(t2_run);
    shared_memory[2] = wut_join(1);
    if (x == 2) {
        x = 3;
        shared_memory[8] = x;
    }
    shared_memory[9] = wut_yield();
    if (x == 4) {
        x = 5;
        shared_memory[11] = x;
    }
}

void check(void) {
    expect(
        shared_memory[0], 1, "wut_id of the first created thread is wrong"
    );
    expect(
        shared_memory[1], 2, "wut_id of the second created thread is wrong"
    );
    expect(
        shared_memory[2], 128, "thread 1 got cancelled, status should be 128"
    );
    expect(
        shared_memory[3], 1, "thread 1 should change x"
    );
    expect(
        shared_memory[4], TEST_MAGIC, "wut_yield should never return (cancelled)"
    );
    expect(
        shared_memory[5], 2, "thread 2 should change x"
    );
    expect(
        shared_memory[6], 0, "wut_cancel should be successful"
    );
    expect(
        shared_memory[7], 3, "wut_id of the third created thread is wrong"
    );
    expect(
        shared_memory[8], 3, "thread 0 should change x"
    );
    expect(
        shared_memory[9], 0, "wut_yield should be successful"
    );
    expect(
        shared_memory[10], 4, "thread 3 should change x"
    );
    expect(
        shared_memory[11], 5, "thread 0 should change x"
    );
}

#include "test.h"

#include "wut.h"

static int x = 0;

void t2_run(void) {
    ++x;
}

void t1_run(void) {
    int id2 = wut_create(t2_run);
    shared_memory[2] = id2;
    ++x;
}

void test(void) {
    wut_init();
    shared_memory[0] = wut_id();
    int id1 = wut_create(t1_run);
    shared_memory[1] = id1;
    shared_memory[3] = wut_yield(); /* Let thread 1 run */
    shared_memory[4] = wut_yield(); /* After thread 1 exits, it's me again,
                                       yield to thread 2 */
    shared_memory[5] = x;
}

void check(void) {
    expect(
        shared_memory[0], 0, "wut_id of the main thread is wrong"
    );
    expect(
        shared_memory[1], 1, "wut_id of the first thread is wrong"
    );
    expect(
        shared_memory[2], 2, "wut_id of the second thread is wrong"
    );
    expect(
        shared_memory[3], 0, "first wut_yield should be successful"
    );
    expect(
        shared_memory[4], 0, "second wut_yield should be successful"
    );
    expect(
        shared_memory[5], 2, "two threads should write to x"
    );
}

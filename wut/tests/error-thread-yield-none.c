#include "test.h"

#include "wut.h"

void run(void) {
    return;
}

void test(void) {
    wut_init();
    shared_memory[0] = wut_id();
    shared_memory[1] = wut_yield();
}

void check(void) {
    expect(
        shared_memory[0], 0, "wut_id of the main thread is wrong"
    );
    expect(
        shared_memory[1], -1, "wut_yield should fail with no other threads"
    );
}

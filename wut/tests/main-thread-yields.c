#include "test.h"

#include "wut.h"

void run(void) {
    shared_memory[2] = wut_id();
    exit(0);
}

void test(void) {
    wut_init();
    shared_memory[0] = wut_id();
    shared_memory[1] = wut_create(run);
    wut_yield();
}

void check(void) {
    expect(
        shared_memory[0], 0, "wut_id of the main thread is wrong"
    );
    expect(
        shared_memory[1], 1, "wut_id of the first thread is wrong"
    );
    expect(
        shared_memory[2], 1, "wut_id of the thread in run is wrong"
    );
}

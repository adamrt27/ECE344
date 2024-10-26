#include "test.h"

#include "wut.h"

void run(void) {
    return;
}

void test(void) {
    wut_init();
    int id = wut_id();
    shared_memory[0] = id;
    shared_memory[1] = wut_join(id);
}

void check(void) {
    expect(
        shared_memory[0], 0, "wut_id of the main thread is wrong"
    );
    expect(
        shared_memory[1], -1, "wut_join should fail on self"
    );
}

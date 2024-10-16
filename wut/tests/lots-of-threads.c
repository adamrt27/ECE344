#include "test.h"

#include "wut.h"

#define NUM_THREADS 255

void run(void) {
    int i = wut_id();
    if (i == NUM_THREADS) {
        wut_exit(i);
        return;
    }
    int id = wut_create(run);
    shared_memory[i] = wut_join(id);
    wut_exit(i);
}

void test(void) {
    wut_init();
    int id = wut_create(run);
    shared_memory[0] = wut_join(id);
}

void check(void) {
    for (int i = 0; i < NUM_THREADS; ++i) {
        expect(
            shared_memory[i], i + 1, "loop check"
        );
    }
}

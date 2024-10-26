#include "test.h"

#include "wut.h"

int x=100;

void t3_run(void) {
    x = x+1;
}

void t2_run(void) {
    int id3 = wut_create(t3_run); 
    shared_memory[3] = id3;

    shared_memory[6] = wut_join(id3);

    x = x + 1;
}

void t1_run(void) {
    int id2 = wut_create(t2_run);
    shared_memory[2] = id2;

    x = x + 1;
}


void test(void) {
    wut_init();

    shared_memory[0] = wut_id();

    int id1 = wut_create(t1_run);
    shared_memory[1] = id1;

    shared_memory[4] = wut_join(id1);

    shared_memory[5] = wut_yield();

    shared_memory[7] = wut_yield();

    shared_memory[8] = x;
} 

void check(void) {
    expect(
        shared_memory[0], 0, "wut_id of the main thread is wrong"
    );
    expect(
        shared_memory[1], 1, "id of the created thread is wrong"
    );
    expect(
        shared_memory[2], 2, "id of the created thread is wrong"
    );
    expect(
        shared_memory[3], 3, "id of the created thread is wrong"
    );
    expect(
        shared_memory[4], 0, "main thread did not join"
    );
    expect(
        shared_memory[5], 0, "main thread should yield"
    );
    expect(
        shared_memory[6], 0, "t2 should join t3"
    );
    expect(
        shared_memory[7], 0, "main thread should yield"
    );
    expect(
        shared_memory[8], 103, "final value of x does not match"
    );
}

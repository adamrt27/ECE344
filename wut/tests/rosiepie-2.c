#include "test.h"

#include "wut.h"

int x = 0; 

void null_run(void) {
    return;
}

void test(void) {
    wut_init();
    shared_memory[0] = wut_id(); // should equal 0
    shared_memory[1] = wut_create(null_run); // should equal 1
    shared_memory[2] = wut_create(null_run); // should equal 2, queue = {0, 1, 2}
    shared_memory[3] = wut_create(null_run); // should equal 3, queue = {0, 1, 2, 3}
    shared_memory[4] = wut_cancel(2); // 0
    shared_memory[5] = wut_cancel(1); // 0 
    shared_memory[6] = wut_join(2); // 128
    shared_memory[7] = wut_join(1);// 128
    shared_memory[8] = wut_create(null_run); // should equal 1
}

void check(void) {
    expect(
        shared_memory[0], 0, "wut_id of the main thread is wrong"
    );
    expect(
        shared_memory[1], 1, "wut_id of the second thread is wrong"
    );
    expect(
        shared_memory[2], 2, "wut_id of the third thread is wrong"
    );
    expect(
        shared_memory[3], 3, "wut_id of the fourth thread is wrong"
    );
    expect(
        shared_memory[4], 0, "first cancel should return 0"
    );
    expect(
        shared_memory[5], 0, "second cancel should return 0"
    );
    expect(
        shared_memory[6], 128, "first wut_join should return 0"
    );
    expect(
        shared_memory[7], 128, "second wut_join should return the status of cancelled thread"
    );
    expect(
        shared_memory[8], 1, "new thread should equal 1"
    );
}

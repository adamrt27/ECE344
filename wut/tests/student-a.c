#include "test.h"

#include "wut.h"

void t1_run(void){
    // Order: 2
    shared_memory[3] = wut_join(1);
    shared_memory[4] = wut_join(3);
    shared_memory[5] = wut_join(10);
    shared_memory[6] = wut_join(-1);
}

void dummy_t1_run(void){
    return; // THIS SHOULD NOT RUN
}

void t2_run(void){
   // Order: 3
   shared_memory[7] = wut_join(1);
   int another_id1 = wut_create(dummy_t1_run); // Before: {2, 0} -> After: {2, 0, 1}
   shared_memory[8] = another_id1;

   // Order: 5
   shared_memory[11] = wut_join(1); // Before {2, 0, 1} -> After {0, 1}
   shared_memory[12] = wut_join(0);
}

void test(void) {
    wut_init();
    // Order: 1
    shared_memory[0] = wut_id();
    int id1 = wut_create(t1_run); // Before: {0} -> After: {0, 1}
    shared_memory[1] = id1;
    int id2 = wut_create(t2_run); // Before: {0, 1} -> After: {0, 1, 2}
    shared_memory[2] = id2;

    // Our Queue should look like {0, 1, 2} here

    // Order: 4
    shared_memory[9] = wut_yield(); // Before: {1, 2, 0} -> After: {0, 1}
    shared_memory[10] = wut_cancel(1); //  Before: {0, 1} -> After: {0, 2}
    wut_exit(0);
}

void check(void) {
    expect(shared_memory[0], 0, "Should return 0: main thread (1)");
    expect(shared_memory[1], 1, "Should return 1: thread 1 (2)");
    expect(shared_memory[2], 2, "Should return 2: thread 2 (3)");
    expect(shared_memory[3], -1, "Should return -1: Cannot join the same thread (4)");
    expect(shared_memory[4], -1, "Should return -1: Cannot join the invalid thread 3 (5)");
    expect(shared_memory[5], -1, "Should return -1: Cannot join the invalid thread 10 (6)");
    expect(shared_memory[6], -1, "Should return -1: Cannot join the invalid thread -1 (7)");
    expect(shared_memory[7], 0, "Should return 0: Thread 1 is terminated hence now freed (8)");
    expect(shared_memory[8], 1, "Should return 1: Since id1 is freed, should be able to use id 1 (9)"); 
    expect(shared_memory[9], 0, "Should return 0: yield should be successful (10)"); 
    expect(shared_memory[10], 0, "Should return 0: Successfully cancel thread 1 (11)"); 
    expect(shared_memory[11], 128, "Should return 128: Thread 1 is exitted with 128 due to cancel (12)");
    expect(shared_memory[12], 0, "Should return 0: Thread 0 is already exitted with 0 (13)"); 
}

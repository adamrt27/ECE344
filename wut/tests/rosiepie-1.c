#include "test.h"

#include "wut.h"

int x = 0; 

void null_run(void) {
    /* (12) Thread 0 starts executing and immediately returns, causing it to
            implicitly exit. The only other thread to run is thread 1, which
            now returns from its join. */
    return;
}

void t1_run(void) {
    /* (4) Thread 1 cancels thread 0, thread 0 is now terminated and no thread
           is attempting to join thread anymore. */
    shared_memory[4] = wut_cancel(0);
    /* (5) Thread 1 joins thread 0, which is terminated, so join returns
           immediately and thread 1 continues to execute. The status of
           thread 0 should be 128 because it got cancelled. */
    shared_memory[5] = wut_join(0);
    /* (6) Thread 1 joins thread 2, which is in the ready queue. Thread 1 blocks
           and thread 2 starts executing `t2_run`. */
    /* (13) Thread 1 continues execution and writes the return value of
            `wut_join` to index 6 of shared memory, it should get a status of
            0 because thread 2 terminated normally. */
    shared_memory[6] = wut_join(2);
    /* (14) Thread 1 increments the global variable `x` from 1 to 2. */
    ++x; 
    /* (15) Write the value of `x` to index 8 of shared memory, it should be 2.
    */
    shared_memory[8] = x;
    /* (16) Thread 1 implicitly exits, we have no other threads and the process
            terminates. The join at time (3) never returns, since the original
            main thread got cancelled, and when we check `shared_memory` at
            index 3, we use the special value to indicate that the write to
            shared memory never happened. */
}

void t2_run(void) {
    /* (7) Thread 2 starts executing, we have no other threads in the ready
           queue. It increments the global variable `x` from 0 to 1. */
    ++x;
    /* (8) Write the value of `x` to index 7 of shared memory, it should be 1.
    */
    shared_memory[7] = x;
    /* (9) Create a new thread, the lowest available thread id should be 0 since
           it got joined at (5). The current running thread is still 2, and the
           ready queue should be = [0]. */
    int id4 = wut_create(null_run); // queue = {2, 0}
    /* (10) Write the return value of `wut_create` to index 9 of shared memory,
            it should be 0. */
    shared_memory[9] = id4;
    /* (11) Thread 2 should implicitly exit, this should cause thread 1 to
            unblock and get added to the ready queue. The queue should now be:
            [0, 1]. Since thread 2 exited, we should run thread 0 next and our
            ready queue is [1]. */
}

void test(void) {
    wut_init();
    /* (1) We only have the main (initial) thread running */
    shared_memory[0] = wut_id();
    shared_memory[1] = wut_create(t1_run);
    shared_memory[2] = wut_create(t2_run);
    /* (2) Thread 0 is running, and the ready queue should be = [1, 2] */
    int id2 = shared_memory[2];
    /* (3) Thread 0 joins thread 2. Thread 0 blocks. Thread 1 starts running
           `t1_run`, and our ready queue is = [2] */
    shared_memory[3] = wut_join(id2);
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
        shared_memory[3], TEST_MAGIC, "wut_join should never return because"
                                      " id 0 is cancelled"
    );
    expect(
        shared_memory[4], 0, "second wut_join should return 0"
    );
    expect(
        shared_memory[5], 128, "third wut_join should return the status of"
                               " cancelled thread"
    );
    expect(
        shared_memory[6], 0, "fourth wut_join should should return 0"
    );
    expect(
        shared_memory[7], 1, "x should increment by 1"
    );
    expect(
        shared_memory[8], 2, "x should increment again"
    );
    expect(
        shared_memory[9], 0, "new thread should reuse id 0"
    );
}

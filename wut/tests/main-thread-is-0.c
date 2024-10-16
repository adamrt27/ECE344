/* Header file for the testing infrastructure, it creates an array of 10240
   integers in shared memory. The purpose of this is to check that your
   program sets the correct values, even if it crashes. The tests run the
   `test` function in a new child process, and after that process terminates
   the test runs the `check` function. */
#include "test.h"

/* This is the header file with all the functions you're creating for your
   threading library. */
#include "wut.h"

/* This `test` function should run the code for the test itself. The test should
   only write to an element in `shared_memory` once (only the last write gets
   checked). You can either write the return values of library calls, or an
   `int` variable you have your threads change. This example just calls
   `wut_init` to initialize your library, all tests should begin with this
   and only call it once. After initialize, it calls `wut_id` and writes the
   result to array index 0 in shared memory. */
void test(void) {
    wut_init();
    shared_memory[0] = wut_id();
}

/* The `check` function runs after the child process that executes `test` exits.
   Here we just call `expect` with the `shared_memory` element we want to check,
   then the value we expect, then a message to print if they do not match. For
   this test we just expect the return value of `wut_id` is 0 if we do not
   create any other threads. After calling `wut_init` the id of the main thread
   should be 0. */
void check(void) {
    expect(
        shared_memory[0], 0, "wut_id of the main thread is wrong"
    );
}

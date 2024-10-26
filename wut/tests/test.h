#ifndef TEST_H
#define TEST_H

#include <assert.h> // assert
#include <stdio.h> // dprintf
#include <stdlib.h> // exit

#define TEST_MAGIC 0x0FF51DE5

extern int* shared_memory;

void test(void);
void check(void);
void expect(int got, int expected, const char* message);

#endif

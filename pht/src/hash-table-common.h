#pragma once

#include <stdint.h>

/* All of our hash tables will have the same capcity so we can create a fair
   comparsion. */
#define HASH_TABLE_CAPACITY 4096

/* We'll also use the same hash function for all our hash tables, called the
   bernstein hash. You may also find it referred to as the djb2 hash. */
uint32_t bernstein_hash(const char *string);

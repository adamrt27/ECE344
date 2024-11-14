#pragma once

#include "hash-table-common.h"

#include <stdbool.h>

/* Forward declaration of our hash table, so we can hide the implementation
   and define the struct in `hash-table-base.c`. */
struct hash_table_base;

/* Create a new hash table, this should allocate any memory needed for an
   empty hash table. */
struct hash_table_base *hash_table_base_create();

/* Add a new entry to the hash table, this will insert a key (string) with
   a value to the hash table. */
void hash_table_base_add_entry(struct hash_table_base *hash_table,
                               const char *key,
                               uint32_t value);
/* Checks if there's an exact match for the specified key in the hash table. */
bool hash_table_base_contains(struct hash_table_base *hash_table,
                              const char *key);
/* Returns the value in the hash table for the specified key, if the key is
   not in the table this function will terminate the process. */
uint32_t hash_table_base_get_value(struct hash_table_base *hash_table,
                                   const char* key);
/* Destroy a hash table, returned from `hash_table_base_create`. This function
   should free all associated memory that the hash table used. It should pass
   `valgrind` with no leaks. */
void hash_table_base_destroy(struct hash_table_base *hash_table);

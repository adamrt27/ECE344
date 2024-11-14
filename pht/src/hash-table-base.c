#include "hash-table-base.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>

/* To resolve collisions, we're going to store a linked list at every entry in
   our hash table and store all the (key, value) pairs associated with that
   slot. We use a singly-linked list (SLIST) because we just iterate through
   the list in one direction. */
struct list_entry {
	const char *key;
	uint32_t value;
    /* In this case this is just a pointer to the next node. */
	SLIST_ENTRY(list_entry) pointers;
};

/* This defines a struct called `list_head` that represents our list. */
SLIST_HEAD(list_head, list_entry);

/* Every entry in the hash table just has a linked list of all (key, value)
   pairs currently in the hash table (at this specific entry). */
struct hash_table_entry {
	struct list_head list_head;
};

/* Defines the actual fields for our `hash_table_base` struct. It's an array of
   `hash_table_entry`s, determined by our `HASH_TABLE_CAPACITY` (which should
   be 4096). The hash function will correspond to one of these entries. For
   your implementations you can add fields to your hash table struct and/or
   your `hash_table_entry`. */
struct hash_table_base {
	struct hash_table_entry entries[HASH_TABLE_CAPACITY];
};

/* This function uses `calloc` to allocate dynamic memory, because it will be
   zero intialized. For every entry in our hash table, we also initialize the
   linked list. If you add any fields to the structs you should initialize them
   in your hash table's create function as well. */
struct hash_table_base *hash_table_base_create()
{
	struct hash_table_base *hash_table = calloc(1, sizeof(struct hash_table_base));
	assert(hash_table != NULL);
	for (size_t i = 0; i < HASH_TABLE_CAPACITY; ++i) {
		struct hash_table_entry *entry = &hash_table->entries[i];
		SLIST_INIT(&entry->list_head);
	}
	return hash_table;
}

/* This helper function returns the linked list we need to use for the specified
   key. It uses the hash function to find the `hash_table_entry` in the hash
   table, then just returns the address of the linked list. */
static struct list_head *get_list_head(struct hash_table_base *hash_table,
                                       const char *key)
{
	assert(key != NULL);
	uint32_t index = bernstein_hash(key) % HASH_TABLE_CAPACITY;
	struct hash_table_entry *entry = &hash_table->entries[index];
	struct list_head *list_head = &entry->list_head;
	return list_head;
}

/* This helper function returns the `list_entry` (key, value) that matches
   the specified key. There should only be one entry for a key in the hash
   table. This function just iterates though the list to find an exact match
   for the key, and if found it immediately returns. Otherwise we return
   `NULL` if the key is not in the hash table. */
static struct list_entry *get_list_entry(struct list_head *list_head,
                                         const char *key) {
	assert(key != NULL);

	struct list_entry *entry = NULL;
	
	SLIST_FOREACH(entry, list_head, pointers) {
	  if (strcmp(entry->key, key) == 0) {
	    return entry;
	  }
	}
	return NULL;
}

/* Return whether or not this key is in the hash table. Using our helper
   functions we just check if there's a valid list_entry for this key in the
   hash table. */
bool hash_table_base_contains(struct hash_table_base *hash_table,
                              const char *key)
{
	struct list_head *list_head = get_list_head(hash_table, key);
	struct list_entry *list_entry = get_list_entry(list_head, key);
	return list_entry != NULL;
}

/* Adds the (key, value) to the hash table. First we use our helper functions
   to see if this key already exists in the hash table. If it exists we should
   update the value to the new value. We do not create a new entry in this
   case because the key should only be in the hash table exactly one time.
   Otherwise, we have a collision and we add it to the linked list. We allocate
   a new list entry, which is a (key, value), and insert it to the end of the
   linked list for this hash table entry. */
void hash_table_base_add_entry(struct hash_table_base *hash_table,
                               const char *key,
                               uint32_t value)
{
	struct list_head *list_head = get_list_head(hash_table, key);
	struct list_entry *list_entry = get_list_entry(list_head, key);

	/* Update the value if it already exists */
	if (list_entry != NULL) {
		list_entry->value = value;
		return;
	}

	list_entry = calloc(1, sizeof(struct list_entry));
	list_entry->key = key;
	list_entry->value = value;
	SLIST_INSERT_HEAD(list_head, list_entry, pointers);
}

/* This code is pretty much exactly the same as `hash_table_base_contains`. The
   only difference is it checks that this key does exist in the hash table and
   terminates the process, otherwise it returns the value associated with this
   key. */
uint32_t hash_table_base_get_value(struct hash_table_base *hash_table,
                                   const char *key)
{
	struct list_head *list_head = get_list_head(hash_table, key);
	struct list_entry *list_entry = get_list_entry(list_head, key);
	assert(list_entry != NULL);
	return list_entry->value;
}

/* This function uses frees all memory our hash table uses. First it goes
   through the linked lists for every element. To properly free all the memory
   we free each node in the linked list, by remove removing the first node
   from the list, freeing it, and doing that until the linked list is empty.
   After we free all of the linked list nodes, we can free the hash table
   itself. You should free any extra memory you use in your implementations
   in your destory function as well. */
void hash_table_base_destroy(struct hash_table_base *hash_table)
{
	for (size_t i = 0; i < HASH_TABLE_CAPACITY; ++i) {
		struct hash_table_entry *entry = &hash_table->entries[i];
		struct list_head *list_head = &entry->list_head;
		struct list_entry *list_entry = NULL;
		while (!SLIST_EMPTY(list_head)) {
			list_entry = SLIST_FIRST(list_head);
			SLIST_REMOVE_HEAD(list_head, pointers);
			free(list_entry);
		}
	}
	free(hash_table);
}

#ifndef VMS_H
#define VMS_H

#include <stdint.h>

#define PAGE_SIZE 4096

/*
MMU Functions

These functions simulate the MMU and CPU instructions that get/set the root
page table. In your code you should only ever use `vms_get_root_page_table`
to get the root page table of the process you're forking.
*/
void vms_write(void* pointer, int value);
int vms_read(void* pointer);
void* vms_get_root_page_table(void);
void vms_set_root_page_table(void* pointer);

/* Page Functions

The functions simulate the kernel managing pages. You should only ever use
2 of these functions: `vms_new_page`, and `vms_get_page_index`. `vms_new_page`
returns a pointer to a page in memory you can use (this is a simulation of a
physical page). You can also use `vms_get_page_index` to turn a pointer into
an index from 0 to MAX_PAGES - 1, in case you need to keep track of something
with an array of pages. For debugging, `vms_get_page_pointer` does the inverse
of `vms_get_page_index`.
*/
void vms_init(void);
void* vms_new_page(void);
void vms_free_page(void*);
int vms_get_used_pages(void);
void* vms_get_page_pointer(int index);
int vms_get_page_index(void* pointer);

/* Page Table Functions

These functions help you access PTEs in the page tables. They work as follows:

`vms_page_table_index`
  Given a virtual address and a level, it returns the index to use at that
  level, e.g. `vms_page_table_index(0xABC123, 0)` returns 188.

`vms_page_table_pte_entry_from_index`
  Given a page table, and an index, it will return a PTE entry (a pointer to
  a PTE).

`vms_page_table_pte_entry
  A helper function that combines both of the above functions into one.
  This saves you from having to find the index, then using it to get the PTE
  entry. This function does it in one step.

`vms_ppn_to_page`
  Given a PPN it returns a pointer to the corresponding page.

`vms_page_to_ppn`
  Given a pointer to a page it returns the corresponding PPN.
*/
uint16_t vms_page_table_index(void* virtual_address, int level);
uint64_t* vms_page_table_pte_entry_from_index(void* page_table, int index);
uint64_t* vms_page_table_pte_entry(void* page_table,
                                   void* virtual_address,
                                   int level);
void* vms_ppn_to_page(uint64_t ppn);
uint64_t vms_page_to_ppn(void* pointer);

/* PTE Functions

These functions allow you to change the bits of a PTE entry without having to
do any bitwise operations.
*/
void vms_pte_valid_clear(uint64_t* entry);
void vms_pte_valid_set(uint64_t* entry);
int vms_pte_valid(uint64_t* entry);
void vms_pte_read_clear(uint64_t* entry);
void vms_pte_read_set(uint64_t* entry);
int vms_pte_read(uint64_t* entry);
void vms_pte_write_clear(uint64_t* entry);
void vms_pte_write_set(uint64_t* entry);
int vms_pte_write(uint64_t* entry);
void vms_pte_custom_clear(uint64_t* entry);
void vms_pte_custom_set(uint64_t* entry);
int vms_pte_custom(uint64_t* entry);
uint64_t vms_pte_get_ppn(uint64_t* entry);
void vms_pte_set_ppn(uint64_t* entry, uint64_t ppn);

/* VMS

These are the functions you write, you're supposed to simulate what happens
during a fork using two approaches. Both functions return a pointer to the
new L2 page table for the new process that should be an independent clone of
the current process. The two approaches are: `vms_fork_copy` just copies
all the memory used by the original processes, and `vms_fork_copy_on_write`
only copies pages when needed, and will otherwise share memory when safe.
*/
void* vms_fork_copy(void);
void* vms_fork_copy_on_write(void);

#endif

#include "vms.h"

#include "mmu.h"
#include "pages.h"

#include <stdio.h>
#include <string.h>

/* A debugging helper that will print information about the pointed to PTE
   entry. */
static void print_pte_entry(uint64_t* entry);

void page_fault_handler(void* virtual_address, int level, void* page_table) {
}

void* vms_fork_copy(void) {
    return vms_get_root_page_table();
}

void* vms_fork_copy_on_write(void) {
    return vms_get_root_page_table();
}

static void print_pte_entry(uint64_t* entry) {
    const char* dash = "-";
    const char* custom = dash;
    const char* write = dash;
    const char* read = dash;
    const char* valid = dash;
    if (vms_pte_custom(entry)) {
        custom = "C";
    }
    if (vms_pte_write(entry)) {
        write = "W";
    }
    if (vms_pte_read(entry)) {
        read = "R";
    }
    if (vms_pte_valid(entry)) {
        valid = "V";
    }

    printf("PPN: 0x%lX Flags: %s%s%s%s\n",
           vms_pte_get_ppn(entry),
           custom, write, read, valid);
}

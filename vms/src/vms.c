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
    // get root of current page table
    // create different levels of page tables
    // copy all memory
    // set the root to your new l2 table
    // return

    // Step 1: Get the root page table (L2 level) of the current process
    void *l2_parent = vms_get_root_page_table();
    
    // Step 2: Create a new empty L2 page table for the child process
    void *l2_child = vms_new_page();

    // Step 3: Loop over all entries in the L2 page table
    for (int i = 0; i < NUM_PTE_ENTRIES; i ++){
        // get parent entry at index i
        uint64_t *l2_parent_entry = vms_page_table_pte_entry_from_index(l2_parent, i);

        // check if the entry is valid
        if (!vms_pte_valid(l2_parent_entry)){
            continue;
        }

        // Step 4: Create a new L1 page table for the child
        void *l1_child = vms_new_page();

        // Get the L1 parent table using the PPN from the L2 parent entry
        void *l1_parent = vms_ppn_to_page(vms_pte_get_ppn(l2_parent_entry));

        // Copy the L2 parent entry to the L2 child entry using memcpy
        uint64_t *l2_child_entry = vms_page_table_pte_entry_from_index(l2_child, i);
        memcpy(l2_child_entry, l2_parent_entry, sizeof(uint64_t));
        vms_pte_set_ppn(l2_child_entry, vms_page_to_ppn(l1_child));  // Set the PPN to the new L1 child table

        // Step 5: Loop over all entries in the L1 page table
        for (int j = 0; j < NUM_PTE_ENTRIES; j ++) {
            // get parent entry at index i
            uint64_t *l1_parent_entry = vms_page_table_pte_entry_from_index(l1_parent, j);

            // check if the entry is valid
            if (!vms_pte_valid(l1_parent_entry)){
                continue;
            }

            // Step 6: Create a new L0 page table for the child
            void *l0_child = vms_new_page();

            // Get the L0 parent table using the PPN from the L1 parent entry
            void *l0_parent = vms_ppn_to_page(vms_pte_get_ppn(l1_parent_entry));

            // Copy the L1 parent entry to the L1 child entry using memcpy
            uint64_t *l1_child_entry = vms_page_table_pte_entry_from_index(l1_child, j);
            memcpy(l1_child_entry, l1_parent_entry, sizeof(uint64_t));
            vms_pte_set_ppn(l1_child_entry, vms_page_to_ppn(l0_child));  // Set the PPN to the new L1 child table

            // Step 7: Loop over all entries in the L0 page table
            for (int k = 0; k < NUM_PTE_ENTRIES; k++) {
                uint64_t *l0_parent_entry = vms_page_table_pte_entry_from_index(l0_parent, k);

                // If the L0 entry is not valid, skip to the next one
                if (!vms_pte_valid(l0_parent_entry)) {
                    continue;
                }

                // Step 8: Create a new P0 page (physical page) for the child
                void *p0_child = vms_new_page();
                void *p0_parent = vms_ppn_to_page(vms_pte_get_ppn(l0_parent_entry));

                // Copy the content of the parent P0 page to the child P0 page
                memcpy(p0_child, p0_parent, PAGE_SIZE);  // Copy the actual content of the physical page (P0)

                // Copy the L0 parent entry to the L0 child entry using memcpy
                uint64_t *l0_child_entry = vms_page_table_pte_entry_from_index(l0_child, k);
                memcpy(l0_child_entry, l0_parent_entry, sizeof(uint64_t));

                // Step 9: Set the PPN in the L0 child entry to the new P0 page
                vms_pte_set_ppn(l0_child_entry, vms_page_to_ppn(p0_child));  // Set the PPN to the new physical page (P0)
            }
        }
    }

    // Step 9: Set the new L2 child as the root page table for the forked process
    vms_set_root_page_table(l2_child);

    // Return the new root page table
    return l2_child;
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

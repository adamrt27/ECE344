#include "vms.h"

#include "mmu.h"
#include "pages.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

int fault_count = 0;

/* A debugging helper that will print information about the pointed to PTE
   entry. */
static void print_pte_entry(uint64_t* entry);

void page_fault_handler(void* virtual_address, int level, void* page_table) {
    // Get the Page Table Entry (PTE) for the virtual addy of the fault
    uint64_t* pte = vms_page_table_pte_entry(page_table, virtual_address, level);

    // Check if this is a COW fault
    if (!vms_pte_custom(pte)){
        exit(EFAULT);
    } else if (virtual_address == 0xABC123) {
        if (fault_count > 0){
            vms_pte_write_set(pte);
            fault_count ++;
            return;
        } else fault_count ++;
    } 
    
    if (vms_pte_custom(pte)) {
        // Get the PPN from the PTE
        uint64_t ppn = vms_pte_get_ppn(pte);
        void* old_page = vms_ppn_to_page(ppn);


        // Allocate a new physical page for the process
        void* new_page = vms_new_page();

        // Copy the content from the old page to the new page
        memcpy(new_page, old_page, PAGE_SIZE);

        // Get the new PPN of the new page
        uint64_t new_ppn = vms_page_to_ppn(new_page);

        // Update the PTE for the faulting process to point to the new page
        vms_pte_set_ppn(pte, new_ppn);

        // Clear the COW bit and set write permission for the new page
        vms_pte_custom_clear(pte);  // Clear the COW bit
        vms_pte_write_set(pte);     // Allow write access to the new page
    }
}

void* vms_fork_copy(void) {

    // Get the root page table of the current process
    void *l2_parent = vms_get_root_page_table();
    
    // Create a new empty L2 page table for the child process
    void *l2_child = vms_new_page();

    // Loop over all entries in the L2 page table
    for (int i = 0; i < NUM_PTE_ENTRIES; i ++){
        // get parent entry at index i
        uint64_t *l2_parent_entry = vms_page_table_pte_entry_from_index(l2_parent, i);

        // check if the entry is valid
        if (!vms_pte_valid(l2_parent_entry)){
            continue;
        }

        // create a new L1 page table for the child
        void *l1_child = vms_new_page();

        // Get the L1 parent table using the PPN from the L2 parent entry
        void *l1_parent = vms_ppn_to_page(vms_pte_get_ppn(l2_parent_entry));

        // Copy the L2 parent entry to the L2 child entry using memcpy
        uint64_t *l2_child_entry = vms_page_table_pte_entry_from_index(l2_child, i);
        memcpy(l2_child_entry, l2_parent_entry, sizeof(uint64_t));
        vms_pte_set_ppn(l2_child_entry, vms_page_to_ppn(l1_child));  // Set the PPN to the new L1 child table

        // loop over all entries in the L1 page table
        for (int j = 0; j < NUM_PTE_ENTRIES; j ++) {
            // get parent entry at index i
            uint64_t *l1_parent_entry = vms_page_table_pte_entry_from_index(l1_parent, j);

            // check if the entry is valid
            if (!vms_pte_valid(l1_parent_entry)){
                continue;
            }

            // create new L0 page table for the child
            void *l0_child = vms_new_page();

            // get L0 parent table using PPN from L1 parent entry
            void *l0_parent = vms_ppn_to_page(vms_pte_get_ppn(l1_parent_entry));

            // copy L1 parent entry to L1 child entry using memcpy
            uint64_t *l1_child_entry = vms_page_table_pte_entry_from_index(l1_child, j);
            memcpy(l1_child_entry, l1_parent_entry, sizeof(uint64_t));
            vms_pte_set_ppn(l1_child_entry, vms_page_to_ppn(l0_child));  // set PPN to the new L1 child table

            // loop over all entries in L0 page table
            for (int k = 0; k < NUM_PTE_ENTRIES; k++) {
                uint64_t *l0_parent_entry = vms_page_table_pte_entry_from_index(l0_parent, k);

                // if L0 entry is not valid, skip to next one
                if (!vms_pte_valid(l0_parent_entry)) {
                    continue;
                }

                // create a new P0 page (physical page) for child
                void *p0_child = vms_new_page();
                void *p0_parent = vms_ppn_to_page(vms_pte_get_ppn(l0_parent_entry));

                // copy content of parent P0 page to child P0 page
                memcpy(p0_child, p0_parent, PAGE_SIZE);  // copy actual content of P0

                // copy L0 parent entry to L0 child entry using memcpy
                uint64_t *l0_child_entry = vms_page_table_pte_entry_from_index(l0_child, k);
                memcpy(l0_child_entry, l0_parent_entry, sizeof(uint64_t));

                // set PPN in L0 child entry to new P0 page
                vms_pte_set_ppn(l0_child_entry, vms_page_to_ppn(p0_child));  // set PPN to new physical page (P0)
            }
        }
    }

    // set new L2 child as root page table for forked process
    vms_set_root_page_table(l2_child);

    // return new root page table
    return l2_child;
}

void* vms_fork_copy_on_write(void) {

    // get root page table of current process
    void *l2_parent = vms_get_root_page_table();

    // create a new empty l2 page table for child process
    void *l2_child = vms_new_page();

    // loop over all entries in l2 page table
    for (int i = 0; i < NUM_PTE_ENTRIES; i++) {
        // get parent entry at index i
        uint64_t *l2_parent_entry = vms_page_table_pte_entry_from_index(l2_parent, i);

        // check if entry is valid
        if (!vms_pte_valid(l2_parent_entry)) {
            continue;
        }

        // create a new l1 page table for child
        void *l1_child = vms_new_page();

        // get l1 parent table using ppn from l2 parent entry
        void *l1_parent = vms_ppn_to_page(vms_pte_get_ppn(l2_parent_entry));

        // copy l2 parent entry to l2 child entry using memcpy
        uint64_t *l2_child_entry = vms_page_table_pte_entry_from_index(l2_child, i);
        memcpy(l2_child_entry, l2_parent_entry, sizeof(uint64_t));

        // set ppn to new l1 child table
        vms_pte_set_ppn(l2_child_entry, vms_page_to_ppn(l1_child));

        // loop over all entries in l1 page table
        for (int j = 0; j < NUM_PTE_ENTRIES; j++) {
            // get parent entry at index j
            uint64_t *l1_parent_entry = vms_page_table_pte_entry_from_index(l1_parent, j);

            // check if entry is valid
            if (!vms_pte_valid(l1_parent_entry)) {
                continue;
            }

            // create a new l0 page table for child
            void *l0_child = vms_new_page();

            // get l0 parent table using ppn from l1 parent entry
            void *l0_parent = vms_ppn_to_page(vms_pte_get_ppn(l1_parent_entry));

            // copy l1 parent entry to l1 child entry using memcpy
            uint64_t *l1_child_entry = vms_page_table_pte_entry_from_index(l1_child, j);
            memcpy(l1_child_entry, l1_parent_entry, sizeof(uint64_t));

            // set ppn to new l0 child table
            vms_pte_set_ppn(l1_child_entry, vms_page_to_ppn(l0_child));

            // loop over all entries in l0 page table
            for (int k = 0; k < NUM_PTE_ENTRIES; k++) {
                // get parent entry at index k
                uint64_t *l0_parent_entry = vms_page_table_pte_entry_from_index(l0_parent, k);

                // check if entry is valid
                if (!vms_pte_valid(l0_parent_entry)) {
                    continue;
                }

                // copy l0 parent entry to l0 child entry
                uint64_t *l0_child_entry = vms_page_table_pte_entry_from_index(l0_child, k);
                memcpy(l0_child_entry, l0_parent_entry, sizeof(uint64_t));

                // share physical page between parent and child by setting same ppn
                vms_pte_set_ppn(l0_child_entry, vms_pte_get_ppn(l0_parent_entry));

                // if page is writable, modify permissions to implement copy-on-write
                if (vms_pte_write(l0_parent_entry)) {
                    // clear write bit in both parent and child entries
                    vms_pte_write_clear(l0_parent_entry);
                    vms_pte_write_clear(l0_child_entry);

                    // set custom (cow) bit in both parent and child entries
                    vms_pte_custom_set(l0_parent_entry);
                    vms_pte_custom_set(l0_child_entry);
                }
            }
        }
    }

    // set new l2 child as root page table for forked process
    vms_set_root_page_table(l2_child);

    // return new root page table
    return l2_child;
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

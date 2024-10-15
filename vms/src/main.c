#include "vms.h"

#include <inttypes.h>
#include <stdio.h>

int main(void) {
    // initialize VMS simulation
    vms_init();

    // create 3 new pages
    void* l2 = vms_new_page();
    void* l1 = vms_new_page();
    void* l0 = vms_new_page();
    
    // creates physical page
    void* p0 = vms_new_page();

    // set up virtual address
    void* virtual_address = (void*) 0xABC123;
    // get the entry on page l2, at the address defined, at level 2
    uint64_t* l2_entry = vms_page_table_pte_entry(l2, virtual_address, 2);
    // set the physical page number of the entry to the ppn of page l1
    vms_pte_set_ppn(l2_entry, vms_page_to_ppn(l1));
    // sets entry to valid, to ensure we know the PTE is a valid mapping
    vms_pte_valid_set(l2_entry);

    // get entry on page l1, at virtual address defined above
    uint64_t* l1_entry = vms_page_table_pte_entry(l1, virtual_address, 1);
    // set ppn of entry to ppn of page l0
    vms_pte_set_ppn(l1_entry, vms_page_to_ppn(l0));
    // set entry to valid in page table
    vms_pte_valid_set(l1_entry);

    // get entry on page l0
    uint64_t* l0_entry = vms_page_table_pte_entry(l0, virtual_address, 0);
    // set ppn of entry to ppn of page p0
    vms_pte_set_ppn(l0_entry, vms_page_to_ppn(p0));
    // set as valid mapping
    vms_pte_valid_set(l0_entry);
    // tells vms it can read and write to p0
    vms_pte_read_set(l0_entry);
    vms_pte_write_set(l0_entry);

    // sets l2 to the root of the page table for the process
    vms_set_root_page_table(l2);

    // writes 1 to virtual address, then reads it
    vms_write(virtual_address, 1);
    printf("0x%" PRIX64 " read: %d\n",
           (uint64_t) virtual_address,
           vms_read(virtual_address));
    // writes 2 to virtual address, then reads it
    vms_write(virtual_address, 2);
    printf("0x%" PRIX64 " read: %d\n",
           (uint64_t) virtual_address,
           vms_read(virtual_address));

    vms_fork_copy();

    return 0;
}

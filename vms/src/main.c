#include "vms.h"

#include <inttypes.h>
#include <stdio.h>

int main(void) {
    vms_init();

    void* l2 = vms_new_page();
    void* l1 = vms_new_page();
    void* l0 = vms_new_page();
    
    void* p0 = vms_new_page();

    void* virtual_address = (void*) 0xABC123;
    uint64_t* l2_entry = vms_page_table_pte_entry(l2, virtual_address, 2);
    vms_pte_set_ppn(l2_entry, vms_page_to_ppn(l1));
    vms_pte_valid_set(l2_entry);

    uint64_t* l1_entry = vms_page_table_pte_entry(l1, virtual_address, 1);
    vms_pte_set_ppn(l1_entry, vms_page_to_ppn(l0));
    vms_pte_valid_set(l1_entry);

    uint64_t* l0_entry = vms_page_table_pte_entry(l0, virtual_address, 0);
    vms_pte_set_ppn(l0_entry, vms_page_to_ppn(p0));
    vms_pte_valid_set(l0_entry);
    vms_pte_read_set(l0_entry);
    vms_pte_write_set(l0_entry);

    vms_set_root_page_table(l2);

    vms_write(virtual_address, 1);
    printf("0x%" PRIX64 " read: %d\n",
           (uint64_t) virtual_address,
           vms_read(virtual_address));
    vms_write(virtual_address, 2);
    printf("0x%" PRIX64 " read: %d\n",
           (uint64_t) virtual_address,
           vms_read(virtual_address));

    return 0;
}

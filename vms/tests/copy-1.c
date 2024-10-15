#include "vms.h"
#include <stdio.h>

#include <assert.h>

int expected_exit_status(void) { return 0; }

void test(void) {
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

    void* forked_l2 = vms_fork_copy();
    vms_set_root_page_table(forked_l2);

    assert(l2 != forked_l2);
    assert(vms_get_used_pages() == 8);
    assert(vms_read(virtual_address) == 1);
    vms_write(virtual_address, 2);
    assert(vms_read(virtual_address) == 2);

    vms_set_root_page_table(l2);
    assert(vms_read(virtual_address) == 1);
}

#include "vms.h"

#include <assert.h>

int expected_exit_status(void) { return 0; }

void test(void) {
    vms_init();

    void* l2 = vms_new_page();
    void* l1 = vms_new_page();
    void* l0 = vms_new_page();
    void* p0 = vms_new_page();
    void* p1 = vms_new_page();
    void* p2 = vms_new_page();
    void* p3 = vms_new_page();

    void* virtual_address_1 = (void*) 0xABC123;
    void* virtual_address_2 = (void*) 0xABD123;
    void* virtual_address_3 = (void*) 0xABE123;
    void* virtual_address_4 = (void*) 0xABF123;
    uint64_t* l2_entry = vms_page_table_pte_entry(l2, virtual_address_1, 2);
    vms_pte_set_ppn(l2_entry, vms_page_to_ppn(l1));
    vms_pte_valid_set(l2_entry);

    assert(vms_page_table_index(virtual_address_1, 2) == vms_page_table_index(virtual_address_2, 2));
    assert(vms_page_table_index(virtual_address_1, 2) == vms_page_table_index(virtual_address_3, 2));
    assert(vms_page_table_index(virtual_address_1, 2) == vms_page_table_index(virtual_address_4, 2));

    uint64_t* l1_entry = vms_page_table_pte_entry(l1, virtual_address_1, 1);
    vms_pte_set_ppn(l1_entry, vms_page_to_ppn(l0));
    vms_pte_valid_set(l1_entry);

    assert(vms_page_table_index(virtual_address_1, 1) == vms_page_table_index(virtual_address_2, 1));
    assert(vms_page_table_index(virtual_address_1, 1) == vms_page_table_index(virtual_address_3, 1));
    assert(vms_page_table_index(virtual_address_1, 1) == vms_page_table_index(virtual_address_4, 1));

    uint64_t* l0_entry_1 = vms_page_table_pte_entry(l0, virtual_address_1, 0);
    vms_pte_set_ppn(l0_entry_1, vms_page_to_ppn(p0));
    vms_pte_valid_set(l0_entry_1);
    vms_pte_read_set(l0_entry_1);
    vms_pte_write_set(l0_entry_1);

    uint64_t* l0_entry_2 = vms_page_table_pte_entry(l0, virtual_address_2, 0);
    vms_pte_set_ppn(l0_entry_2, vms_page_to_ppn(p1));
    vms_pte_valid_set(l0_entry_2);
    vms_pte_read_set(l0_entry_2);
    vms_pte_write_set(l0_entry_2);

    uint64_t* l0_entry_3 = vms_page_table_pte_entry(l0, virtual_address_3, 0);
    vms_pte_set_ppn(l0_entry_3, vms_page_to_ppn(p2));
    vms_pte_valid_set(l0_entry_3);
    vms_pte_read_set(l0_entry_3);
    vms_pte_write_set(l0_entry_3);

    uint64_t* l0_entry_4 = vms_page_table_pte_entry(l0, virtual_address_4, 0);
    vms_pte_set_ppn(l0_entry_4, vms_page_to_ppn(p3));
    vms_pte_valid_set(l0_entry_4);
    vms_pte_read_set(l0_entry_4);

    vms_set_root_page_table(l2);

    vms_write(virtual_address_1, 1);
    vms_write(virtual_address_2, 2);
    vms_write(virtual_address_3, 3);
    assert(vms_read(virtual_address_4) == 0);

    void* forked_l2 = vms_fork_copy();
    vms_set_root_page_table(forked_l2);

    assert(l2 != forked_l2);
    assert(vms_get_used_pages() == 14);
    assert(vms_read(virtual_address_1) == 1);
    vms_write(virtual_address_1, 2);
    assert(vms_read(virtual_address_1) == 2);
    assert(vms_read(virtual_address_2) == 2);
    vms_write(virtual_address_2, 4);
    assert(vms_read(virtual_address_2) == 4);
    assert(vms_read(virtual_address_3) == 3);
    vms_write(virtual_address_3, 9);
    assert(vms_read(virtual_address_3) == 9);
    assert(vms_read(virtual_address_4) == 0);

    vms_set_root_page_table(l2);
    assert(vms_read(virtual_address_1) == 1);
    assert(vms_read(virtual_address_2) == 2);
    assert(vms_read(virtual_address_3) == 3);
    assert(vms_read(virtual_address_4) == 0);
}


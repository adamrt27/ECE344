#include "vms.h"

#include "pages.h"

#include <assert.h>

#define INDEX_BITS 9
#define OFFSET_BITS 12

uint16_t vms_page_table_index(void* virtual_address, int level) {
    uint8_t start_bit = INDEX_BITS * level + OFFSET_BITS;
    uint64_t mask = (uint64_t) 0x1FF << start_bit;
    return (mask & ((uint64_t) virtual_address)) >> start_bit;
}

uint64_t* vms_page_table_pte_entry_from_index(void* page_table, int index) {
    return &((uint64_t*) page_table)[index];
}

uint64_t* vms_page_table_pte_entry(void* page_table,
                                   void* virtual_address,
                                   int level) {
    uint16_t index = vms_page_table_index(virtual_address, level);
    return vms_page_table_pte_entry_from_index(page_table, index);
}

void* vms_ppn_to_page(uint64_t ppn) {
    return (void*) (ppn << OFFSET_BITS);
}

uint64_t vms_page_to_ppn(void* pointer) {
    check_page_aligned(pointer);
    uint64_t ppn = (uint64_t) pointer;
    assert((ppn & 0xF000000000000000) == 0);
    return ppn >> OFFSET_BITS;
}

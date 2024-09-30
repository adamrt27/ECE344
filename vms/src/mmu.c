#include "vms.h"

#include "mmu.h"
#include "pages.h"

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#define MMU_LEVELS 3

static void* root_page_table = NULL;

static int should_generate_fault(int level, uint64_t* entry) {
    if (!vms_pte_valid(entry)) {
        return 1;
    }
    else if (level != 0 && (vms_pte_read(entry) || vms_pte_write(entry))) {
        return 1;
    }
    else  if (level == 0 && !vms_pte_read(entry) && !vms_pte_write(entry)) {
        return 1;
    }
    return 0;
}

void* vms_get_root_page_table(void) {
    return root_page_table;
}

void vms_set_root_page_table(void* page_table) {
    check_page_aligned(page_table);
    root_page_table = page_table;
}

static void print_fatal_page_fault(void* virtual_address,
                                   int level,
                                   void* page_table) {
    uint64_t* entry = vms_page_table_pte_entry(page_table,
                                               virtual_address,
                                               level);
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

    printf("Fatal page fault!\n"
           "  Virtual address: 0x%" PRIX64 "\n"
           "  Page table: 0x%" PRIX64 "\n"
           "  Level: %d\n"
           "  PTE:\n"
           "    PPN: 0x%" PRIX64 "\n"
           "    Flags: %s%s%s%s\n", 
           (uint64_t) virtual_address,
           (uint64_t) page_table,
           level,
           vms_pte_get_ppn(entry),
           custom, write, read, valid);
}

static uint64_t* mmu(void* virtual_address) {
    void* page_table = root_page_table;
    int faulted = 0;
    for (int level = MMU_LEVELS - 1; level >= 0; --level) {
        uint64_t* entry = vms_page_table_pte_entry(page_table,
                                                   virtual_address,
                                                   level);

        if (should_generate_fault(level, entry)) {
            if (!faulted) {
                faulted = 1;
                page_fault_handler(virtual_address, level, page_table);
                ++level;
                continue;
            }
            else {
                print_fatal_page_fault(virtual_address, level, page_table);
                exit(EFAULT);
            }
        }
        faulted = 0;

        if (level != 0) {
            page_table = vms_ppn_to_page(vms_pte_get_ppn(entry));
            continue;
        }

        return entry;
    }
    __builtin_unreachable();
}

static void* translate_address(void* virtual_address, uint64_t* entry) {
    void* physical_address = vms_ppn_to_page(vms_pte_get_ppn(entry));
    physical_address = (void*) (((uint64_t)physical_address)
                       | (((uint64_t) virtual_address) & 0xFFF));
    return physical_address;
}

static void* get_base_page(uint64_t* entry) {
    return (void*) (((uint64_t)entry) & ~0xFFF);
}

void vms_write(void* virtual_address, int value) {
    uint64_t* entry = mmu(virtual_address);
    if (!vms_pte_write(entry)) {
        void* l0 = get_base_page(entry);
        page_fault_handler(virtual_address, 0, l0);
        if (!vms_pte_write(entry)) {
            print_fatal_page_fault(virtual_address, 0, l0);
            exit(EFAULT);
        }
    }
    int* pointer = translate_address(virtual_address, entry);
    *pointer = value;
}

int vms_read(void* virtual_address) {
    uint64_t* entry = mmu(virtual_address);
    if (!vms_pte_read(entry)) {
        void* l0 = get_base_page(entry);
        page_fault_handler(virtual_address, 0, l0);
        if (!vms_pte_read(entry)) {
            print_fatal_page_fault(virtual_address, 0, l0);
            exit(EFAULT);
        }
    }
    int* pointer = translate_address(virtual_address, entry);
    return *pointer;
}

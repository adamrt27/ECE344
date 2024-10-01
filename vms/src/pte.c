#include "vms.h"

#define PTE_CUSTOM (1 << 8)
#define PTE_WRITE (1 << 2)
#define PTE_READ  (1 << 1)
#define PTE_VALID (1 << 0)
#define PTE_PPN_START_BIT 10

void vms_pte_valid_clear(uint64_t* entry) {
    *entry &= ~PTE_VALID;
}

void vms_pte_valid_set(uint64_t* entry) {
    *entry |= PTE_VALID;
}

int vms_pte_valid(uint64_t* entry) {
    return (*entry & PTE_VALID) != 0;
}

void vms_pte_read_clear(uint64_t* entry) {
    *entry &= ~PTE_READ;
}

void vms_pte_read_set(uint64_t* entry) {
    *entry |= PTE_READ;
}

int vms_pte_read(uint64_t* entry) {
    return (*entry & PTE_READ) != 0;
}

void vms_pte_write_clear(uint64_t* entry) {
    *entry &= ~PTE_WRITE;
}

void vms_pte_write_set(uint64_t* entry) {
    *entry |= PTE_WRITE;
}

int vms_pte_write(uint64_t* entry) {
    return (*entry & PTE_WRITE) != 0;
}

void vms_pte_custom_clear(uint64_t* entry) {
    *entry &= ~PTE_CUSTOM;
}

void vms_pte_custom_set(uint64_t* entry) {
    *entry |= PTE_CUSTOM;
}

int vms_pte_custom(uint64_t* entry) {
    return (*entry & PTE_CUSTOM) != 0;
}

uint64_t vms_pte_get_ppn(uint64_t* entry) {
    uint64_t mask = ((((uint64_t)~0) << 20) >> PTE_PPN_START_BIT);
    return (*entry & mask) >> PTE_PPN_START_BIT;
}

void vms_pte_set_ppn(uint64_t* entry, uint64_t ppn) {
    uint64_t mask = ~((((uint64_t)~0) << 20) >> PTE_PPN_START_BIT);
    *entry &= mask;
    ppn = (ppn << 20) >> PTE_PPN_START_BIT;
    *entry |= ppn;
}

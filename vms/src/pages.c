#include "vms.h"

#include "pages.h"

#include <assert.h> // assert
#include <errno.h> // errno
#include <stdint.h> // uintptr_t
#include <stdio.h> // perror
#include <stdlib.h> // exit
#include <string.h> // memset
#include <sys/mman.h> // mmap
#include <unistd.h> // sysconf

static void* base_pointer = NULL;
static int allocated[MAX_PAGES] = {0}; /* This can be more space efficient */
static int used_pages = 0;

void* vms_get_page_pointer(int index) {
    return ((uint8_t*) base_pointer) + (index * PAGE_SIZE);
}

int vms_get_page_index(void* pointer) {
    return (((uint64_t) pointer) - ((uint64_t) base_pointer)) / PAGE_SIZE;
}

void check_page_aligned(void* pointer) {
    assert((uintptr_t) pointer % PAGE_SIZE == 0);
}

void vms_init(void) {
    assert(sysconf(_SC_PAGE_SIZE) == PAGE_SIZE);

    base_pointer = mmap(
        NULL,
        MAX_PAGES * PAGE_SIZE,
        PROT_READ | PROT_WRITE,
        MAP_ANONYMOUS | MAP_SHARED,
        -1,
        0
    );
    if (base_pointer == MAP_FAILED) {
        int err = errno;
        perror("mmap");
        exit(err);
    }
    check_page_aligned(base_pointer);
}

void* vms_new_page(void) {
    for (int i = 0; i < MAX_PAGES; ++i) {
        if (!allocated[i]) {
            allocated[i] = 1;
            ++used_pages;
            return vms_get_page_pointer(i);
        }
    }
    exit(ENOMEM);
}

void vms_free_page(void* pointer) {
    check_page_aligned(pointer);

    int i = vms_get_page_index(pointer);
    assert(allocated[i]);
    allocated[i] = 0;
    memset(pointer, 0, PAGE_SIZE);
    --used_pages;
}

int vms_get_used_pages(void) {
    return used_pages;
}

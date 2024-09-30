#ifndef MMU_H
#define MMU_H

void page_fault_handler(void* virtual_address,
                        int level,
                        void* page_table);

#endif

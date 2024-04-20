#ifndef PAGE_H
#define PAGE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PAGE_SIZE 0x1000 // 4k (>> 12)
#define PAGE_ALIGNED(PTR) (((PTR) & 0xfffff000) + PAGE_SIZE)
#define PAGE_FLAG_USED 0x100000
#define PAGE_FLAG_ASSIGNED 0x1000000

/* Page Bits
+---------------+-----------------+--------------+
| 12 bits flags | 20 bits page no | 32 bits size |
+---------------+-----------------+--------------+
*/

typedef struct page_ {
    uint32_t data;
    size_t size;
} __attribute__((packed)) page_t;

void page_init(page_t * page, uint32_t index);

uint32_t page_address(const page_t * page);
uint32_t page_index(const page_t * page);

void page_set_index(page_t * page, uint32_t index);
void page_set_address(page_t * page, uint32_t address);

bool page_used(const page_t * page);
void page_set_used(page_t * page);
void page_set_free(page_t * page);

bool page_assigned(const page_t * page);

#endif // PAGE_H

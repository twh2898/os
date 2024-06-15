#include "libc/mem.h"

#include "defs.h"
#include "drivers/ram.h"
#include "kernel.h"
#include "libc/string.h"

#define MAGIC 0x31247596
#define STACK_START 0x90000

#define R2U(REGION) ((uint32_t)(REGION))
#define U2R(PTR) ((region_header_t *)(PTR))

#define MIN_REGION_SIZE 8

#define HEADER_TO_PTR(HEADER) ((void *)(HEADER) + sizeof(region_header_t))
#define PTR_TO_HEADER(PTR) \
    ((region_header_t *)((PTR) - sizeof(region_header_t)))

typedef struct {
    uint32_t magic : 32;
    uint32_t next : 32;
    uint32_t prev : 32;
    uint32_t size : 31;
    bool free : 1;
} __attribute__((packed)) region_header_t;

static uint32_t malloc_start;
static uint32_t malloc_end;
static region_header_t * first_region;

static region_header_t * region_split(region_header_t * region, size_t size);
static region_header_t * region_remove(region_header_t * region);
static void integrity_check();

void init_malloc() {
    size_t largest_i = 0;
    uint64_t largest_size = 0;
    bool found = false;

    for (size_t i = 0; i < ram_upper_count(); i++) {
        if (ram_upper_usable(i)) {
            uint64_t curr_size = ram_upper_size(i);
            if (ram_upper_start(i) > STACK_START && curr_size > largest_size) {
                largest_i = i;
                largest_size = curr_size;
                found = true;
            }
        }
    }

    if (!found) {
        KERNEL_PANIC("Could not find area for malloc");
    }

    if (ram_upper_end(largest_i) > 0xffffffff) {
        KERNEL_PANIC("malloc ends above 4 GB limit");
    }

    malloc_start = PAGE_ALIGNED(ram_upper_start(largest_i));
    malloc_end = PAGE_ALIGNED_DOWN(ram_upper_end(largest_i));

    first_region = (region_header_t *)malloc_start;
    first_region->magic = MAGIC;
    first_region->next = 0;
    first_region->prev = 0;
    first_region->size = (malloc_end - malloc_start) - sizeof(region_header_t);
    first_region->free = true;
}

void * malloc(size_t size) {
    if (size < MIN_REGION_SIZE)
        size = MIN_REGION_SIZE;

    integrity_check();

    region_header_t * curr = first_region;

    while (curr) {
        if (curr->free && curr->size >= size) {
            size_t size_diff = curr->size - size;

            if (size_diff > sizeof(region_header_t) + MIN_REGION_SIZE)
                region_split(curr, size);

            curr->free = false;
            return HEADER_TO_PTR(curr);
        }
        curr = U2R(curr->next);
    }

#if SAFETY > 0
    KERNEL_PANIC("Malloc out of memory");
#endif

    return 0;
}

void free(void * ptr) {
    if (!ptr)
        return;

    region_header_t * region = PTR_TO_HEADER(ptr);
    if (region->magic != MAGIC) {
        KERNEL_PANIC("Free on bad pointer");
    }

    integrity_check();
    region_remove(region);
}

static region_header_t * region_split(region_header_t * region, size_t size) {
    if (!region)
        return 0;

    region_header_t * next_region = U2R(region->next);
    region_header_t * new_region =
        U2R(R2U(region) + size + sizeof(region_header_t));

    new_region->magic = MAGIC;
    new_region->size = region->size - size - sizeof(region_header_t);
    new_region->next = R2U(next_region);
    new_region->prev = R2U(region);
    new_region->free = true;

    region->size = size;

    next_region->prev = R2U(new_region);

    region->next = R2U(new_region);

    return new_region;
}

static region_header_t * region_remove(region_header_t * region) {
    if (!region)
        return 0;

    if (!region->prev)
        return region;

    region->magic = 0;

    region_header_t * prev_region = U2R(region->prev);
    region_header_t * next_region = U2R(region->next);

    prev_region->size += region->size + sizeof(region_header_t);
    prev_region->next = region->next;

    if (region->next)
        next_region->prev = region->prev;

    return prev_region;
}

static void integrity_check() {
    region_header_t * curr = first_region;
    while (curr) {
        if (curr->magic != MAGIC) {
            KERNEL_PANIC("Corrupted memory");
        }
        curr = U2R(curr->next);
    }
}

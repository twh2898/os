#include "memory_alloc.h"

#define PAGE_SIZE 4096

#define MAGIC_FREE 0x46524545
#define MAGIC_USED 0x55534544

#define NOT_ALIGNED(SIZE)         ((uint32_t)(SIZE) & 0x3)
#define IS_ALIGNED(SIZE)          (!NOT_ALIGNED(SIZE))
#define SHOULD_SPLIT(ENTRY, SIZE) ((ENTRY)->size >= (SIZE) + sizeof(memory_entry_t) + 4)

#define ALIGN_SIZE(SIZE)                   \
    if ((SIZE) & 0x3) {                    \
        (SIZE) = (((SIZE) >> 2) + 1) << 2; \
    }

int memory_init(memory_t * mem, memory_alloc_pages_t alloc_pages_fn) {
    if (!mem || !alloc_pages_fn) {
        return -1;
    }

    mem->first          = alloc_pages_fn(1);
    mem->last           = mem->first;
    mem->alloc_pages_fn = alloc_pages_fn;

    if (!mem->first) {
        return -1;
    }

    memory_entry_t * entry = mem->first;

    entry->magic = MAGIC_FREE;
    entry->size  = PAGE_SIZE - sizeof(memory_entry_t);
    entry->next  = 0;
    entry->prev  = 0;

    return 0;
}

void * memory_alloc(memory_t * mem, size_t size) {
    if (!mem || !size) {
        return 0;
    }

    ALIGN_SIZE(size);

    memory_entry_t * entry = memory_find_entry_size(mem, size);

    if (!entry) {
        size_t request_size = size;

        if (mem->last->magic == MAGIC_FREE) {
            request_size -= mem->last->size - sizeof(memory_entry_t);
        }

        entry = memory_add_entry(mem, request_size);

        if (!entry) {
            return 0;
        }

        if (request_size != size) {
            if (entry->prev->magic != MAGIC_FREE) {
                return 0;
            }

            entry = entry->prev;

            if (memory_merge_with_next(mem, entry)) {
                return 0;
            }
        }
    }

    if (SHOULD_SPLIT(entry, size)) {
        if (memory_split_entry(mem, entry, size)) {
            return 0;
        }
    }

    entry->magic = MAGIC_USED;

    return ENTRY_PTR(entry);
}

void * memory_realloc(memory_t * mem, void * ptr, size_t size) {
    if (!mem || !ptr || !size) {
        return 0;
    }

    return 0; // not implemented

    // // Will never be found
    // if (NOT_ALIGNED(ptr)) {
    //     return 0;
    // }

    // ALIGN_SIZE(size);

    // memory_entry_t * entry = memory_find_entry_ptr(mem, ptr);

    // // Does not exist
    // if (!entry || entry->magic != MAGIC_USED) {
    //     return 0;
    // }

    // // Same size
    // if (entry->size == size) {
    //     return ENTRY_PTR(entry);
    // }

    // // Shrink
    // if (entry->size < size) {
    //     if (SHOULD_SPLIT(entry, size) && memory_split_entry(mem, entry, size)) {
    //         return 0;
    //     }

    //     return ENTRY_PTR(entry);
    // }

    // // Try to expand
    // memory_entry_t * next_entry = entry->next;

    // if (next_entry->magic == MAGIC_FREE) {
    //     size_t need_size = size - entry->size;

    //     while (next_entry && next_entry->size) {
    //         if (!next_entry->next || next_entry->next->magic != MAGIC_FREE) {
    //             break;
    //         }

    //         if (memory_merge_with_next(mem, next_entry)) {
    //             return 0;
    //         }
    //     }

    //     if (next_entry->size >= need_size) {
    //         if (!memory_merge_with_next(mem, entry)) {
    //             return 0;
    //         }

    //         if (SHOULD_SPLIT(entry, size) && memory_split_entry(mem, entry, size)) {
    //             return 0;
    //         }

    //         return ENTRY_PTR(entry);
    //     }
    // }

    // // Create new and copy
    // memory_entry_t * new_entry = memory_find_entry_size(mem, size);

    // if (!new_entry) {
    //     entry = memory_alloc(mem, size);
    // }

    // entry->magic = MAGIC_USED;

    // char * src  = ENTRY_PTR(entry);
    // char * dest = ENTRY_PTR(new_entry);

    // for (size_t i = 0; i < entry->size; i++) {
    //     *dest++ = *src++;
    // }

    // return ENTRY_PTR(new_entry);
}

int memory_free(memory_t * mem, void * ptr) {
    if (!mem || !ptr) {
        return -1;
    }

    // Will never be found
    if (NOT_ALIGNED(ptr)) {
        return -1;
    }

    memory_entry_t * entry = memory_find_entry_ptr(mem, ptr);

    if (!entry) {
        return -1;
    }

    entry->magic = MAGIC_FREE;

    return 0;
}

int memory_split_entry(memory_t * mem, memory_entry_t * entry, size_t size) {
    if (!mem || !entry || !size) {
        return -1;
    }

    ALIGN_SIZE(size);

    // There must be enough space for size + another entry of min size 4 bytes
    if (entry->size < size + sizeof(memory_entry_t) + 4) {
        return -1;
    }

    memory_entry_t * new_entry = ENTRY_PTR(entry) + size;

    new_entry->magic = MAGIC_FREE;
    new_entry->size  = entry->size - size - sizeof(memory_entry_t);
    new_entry->prev  = entry;
    new_entry->next  = entry->next;

    if (entry == mem->last) {
        mem->last = new_entry;
    }

    if (entry->next) {
        entry->next->prev = new_entry;
    }

    entry->next = new_entry;
    entry->size = size;

    return 0;
}

int memory_merge_with_next(memory_t * mem, memory_entry_t * entry) {
    if (!mem || !entry || !entry->next || entry->next->magic != MAGIC_FREE) {
        return -1;
    }

    memory_entry_t * next_entry = entry->next;

    if (next_entry == mem->last) {
        mem->last = entry;
    }

    entry->size += next_entry->size + sizeof(memory_entry_t);
    entry->next = next_entry->next;

    if (next_entry->next) {
        next_entry->next->prev = entry;
    }

    return 0;
}

memory_entry_t * memory_find_entry_size(memory_t * mem, size_t size) {
    if (!mem || !size) {
        return 0;
    }

    memory_entry_t * entry = mem->first;

    while (entry) {
        if (entry->magic == MAGIC_FREE) {
            memory_entry_t * next_entry = entry->next;

            while (entry->size < size && next_entry) {
                if (next_entry->magic != MAGIC_FREE) {
                    break;
                }

                if (memory_merge_with_next(mem, entry)) {
                    return 0;
                }

                if (entry->size >= size) {
                    break;
                }

                next_entry = entry->next;
            }

            if (entry->size >= size) {
                return entry;
            }
        }

        entry = entry->next;
    }

    return 0;
}

memory_entry_t * memory_find_entry_ptr(memory_t * mem, void * ptr) {
    if (!mem || !ptr) {
        return 0;
    }

    memory_entry_t * entry = mem->first;

    while (entry) {
        if (ENTRY_PTR(entry) == ptr) {
            return entry;
        }

        entry = entry->next;
    }

    return 0;
}

memory_entry_t * memory_add_entry(memory_t * mem, size_t size) {
    if (!mem || !size) {
        return 0;
    }

    size += sizeof(memory_entry_t);

    size_t pages = size >> 12;
    if (size & 0xfff) {
        pages++;
    }

    void * new_pages = mem->alloc_pages_fn(pages);
    if (!new_pages) {
        return 0;
    }

    memory_entry_t * entry = new_pages;

    entry->magic    = MAGIC_FREE;
    entry->size     = pages * PAGE_SIZE - sizeof(memory_entry_t);
    entry->prev     = mem->last;
    mem->last->next = entry;
    mem->last       = entry;

    return entry;
}

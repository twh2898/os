#ifndef MEMORY_ALLOC_H
#define MEMORY_ALLOC_H

#include <stddef.h>
#include <stdint.h>

#define ENTRY_PTR(ENTRY) ((void *)((uint32_t)(ENTRY) + sizeof(memory_entry_t)))

typedef void * (*memory_alloc_pages_t)(size_t pages);

typedef struct _entry {
    uint32_t        magic;
    struct _entry * next;
    struct _entry * prev;
    size_t          size;
} __attribute__((packed)) memory_entry_t;

typedef struct _memory {
    memory_entry_t *     first;
    memory_entry_t *     last;
    memory_alloc_pages_t alloc_pages_fn;
} memory_t;

int memory_init(memory_t * mem, memory_alloc_pages_t alloc_pages_fn);

void * memory_alloc(memory_t * mem, size_t size);
void * memory_realloc(memory_t * mem, void * ptr, size_t size);
int    memory_free(memory_t * mem, void * ptr);

int memory_split_entry(memory_t * mem, memory_entry_t * entry, size_t size);
int memory_merge_with_next(memory_t * mem, memory_entry_t * entry);

memory_entry_t * memory_find_entry_size(memory_t * mem, size_t size);
memory_entry_t * memory_find_entry_ptr(memory_t * mem, void * ptr);
memory_entry_t * memory_add_entry(memory_t * mem, size_t size);

#endif // MEMORY_ALLOC_H

#include "libc/datastruct/circular_buffer.h"

#include "libc/memory.h"
#include "libc/string.h"

static void * elem_ptr(const cb_t * cb, size_t i);
static size_t wrap_index(const cb_t * cb, size_t i);

int cb_create(cb_t * cb, size_t size, size_t elem_size) {
    if (!cb || !size || !elem_size) {
        return -1;
    }

    cb->buff = pmalloc(size * elem_size);
    if (!cb->buff) {
        return -1;
    }

    cb->start     = 0;
    cb->len       = 0;
    cb->size      = size;
    cb->elem_size = elem_size;

    return 0;
}

void cb_free(cb_t * cb) {
    if (cb && cb->buff) {
        pfree(cb->buff);
        cb->buff = 0;
    }
}

size_t cb_buff_size(const cb_t * cb) {
    return cb->size;
}

size_t cb_len(const cb_t * cb) {
    return cb->len;
}

void * cb_peek(const cb_t * cb, size_t i) {
    if (!cb || !cb->len || i >= cb->len) {
        return 0;
    }

    return elem_ptr(cb, i);
}

int cb_push(cb_t * cb, const void * item) {
    if (!cb || !item || cb->len >= cb->size) {
        return -1;
    }

    void * elem = elem_ptr(cb, cb->len);

    kmemcpy(elem, item, cb->elem_size);

    cb->len++;

    return 0;
}

int cb_pop(cb_t * cb, void * item) {
    if (!cb || cb->len == 0) {
        return -1;
    }

    if (item) {
        void * elem = elem_ptr(cb, 0);
        kmemcpy(item, elem, cb->elem_size);
    }

    cb->len--;
    cb->start = (cb->start + 1) % cb->size;

    return 0;
}

int cb_rpop(cb_t * cb, void * item) {
    if (!cb || !cb->len) {
        return -1;
    }

    if (item) {
        void * elem = elem_ptr(cb, cb->len - 1);
        kmemcpy(item, elem, cb->elem_size);
    }

    cb->len--;

    return 0;
}

static void * elem_ptr(const cb_t * cb, size_t i) {
    return cb->buff + (wrap_index(cb, i) * cb->elem_size);
}

static size_t wrap_index(const cb_t * cb, size_t i) {
    return (cb->start + i) % cb->size;
}

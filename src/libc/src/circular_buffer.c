#include "libc/datastruct/circular_buffer.h"

#include "libc/memory.h"
#include "libc/string.h"

struct _ds_cb {
    void * buff;
    size_t start;
    size_t len;
    size_t size;
    size_t elem_size;
};

static void * elem_ptr(const cb_t * cb, size_t i);
static size_t wrap_index(const cb_t * cb, size_t i);

cb_t * cb_new(size_t size, size_t elem_size) {
    if (!size || !elem_size) {
        return 0;
    }

    cb_t * cb = kmalloc(sizeof(cb_t));
    if (!cb) {
        return 0;
    }

    cb->buff = kmalloc(size * elem_size);
    if (!cb->buff) {
        kfree(cb);
        return 0;
    }

    cb->start     = 0;
    cb->len       = 0;
    cb->size      = size;
    cb->elem_size = elem_size;

    return cb;
}

void cb_free(cb_t * cb) {
    if (!cb) {
        return;
    }

    kfree(cb->buff);
    kfree(cb);
}

size_t cb_buff_size(cb_t * cb) {
    return cb->size;
}

size_t cb_len(cb_t * cb) {
    return cb->len;
}

void * cb_peek(cb_t * cb, size_t i) {
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

    if (!kmemcpy(elem, item, cb->elem_size)) {
        return -1;
    }

    cb->len++;

    return 0;
}

int cb_pop(cb_t * cb, void * item) {
    if (!cb || cb->len == 0) {
        return -1;
    }

    void * elem = elem_ptr(cb, 0);

    if (item) {
        if (!kmemcpy(item, elem, cb->elem_size)) {
            return -1;
        }
    }

    cb->len--;
    cb->start = (cb->start + 1) % cb->size;

    return 0;
}

static void * elem_ptr(const cb_t * cb, size_t i) {
    if (!cb) {
        return 0;
    }

    return cb->buff + (wrap_index(cb, i) * cb->elem_size);
}

static size_t wrap_index(const cb_t * cb, size_t i) {
    if (!cb) {
        return 0;
    }

    return (cb->start + i) % cb->size;
}

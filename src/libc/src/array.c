#include "libc/datastruct/array.h"

#include "libc/memory.h"
#include "libc/string.h"

struct _ds_arr {
    void * data;
    size_t len;
    size_t size;
    size_t elem_size;
};

static void * arr_at_no_limit(arr_t * arr, size_t i);
static int    grow_array(arr_t * arr);

arr_t * arr_new(size_t size, size_t elem_size) {
    if (!size || !elem_size) {
        return 0;
    }

    arr_t * arr = kmalloc(sizeof(arr_t));
    if (!arr) {
        return 0;
    }

    arr->data = kmalloc(size * elem_size);
    if (!arr->data) {
        kfree(arr);
        return 0;
    }

    arr->len       = 0;
    arr->size      = size;
    arr->elem_size = elem_size;

    return arr;
}

void arr_free(arr_t * arr) {
    if (!arr) {
        return;
    }

    kfree(arr->data);
    kfree(arr);
}

size_t arr_size(arr_t * arr) {
    return arr->len;
}

void * arr_data(arr_t * arr) {
    return arr->data;
}

void * arr_at(arr_t * arr, size_t i) {
    if (!arr || i >= arr->len) {
        return 0;
    }

    return arr_at_no_limit(arr, i);
}

int arr_get(arr_t * arr, size_t i, void * item) {
    const void * elem = arr_at(arr, i);
    if (!elem) {
        return -1;
    }

    if (!kmemcpy(item, elem, arr->elem_size)) {
        return -1;
    }

    return 0;
}

int arr_set(arr_t * arr, size_t i, const void * item) {
    void * elem = arr_at(arr, i);
    if (!elem) {
        return -1;
    }

    if (!kmemcpy(elem, item, arr->elem_size)) {
        return -1;
    }

    return 0;
}

int arr_insert(arr_t * arr, size_t i, const void * item) {
    if (!arr || !item || i > arr->len) {
        return -1;
    }

    if (arr->len >= arr->size) {
        grow_array(arr);
    }

    void * elem = arr_at_no_limit(arr, i);

    if (i < arr->len) {
        void * shift = elem + arr->elem_size;
        if (!kmemmove(shift, elem, (arr->len - i) * arr->elem_size)) {
            return -1;
        }
    }

    if (!kmemcpy(elem, item, arr->elem_size)) {
        return -1;
    }

    arr->len++;

    return 0;
}

int arr_remove(arr_t * arr, size_t i, void * item) {
    void * elem = arr_at(arr, i);
    if (!elem) {
        return -1;
    }

    if (item) {
        if (!kmemcpy(item, elem, arr->elem_size)) {
            return -1;
        }
    }

    if (i < arr->len - 1) {
        void * shift = elem + arr->elem_size;
        if (!kmemmove(elem, shift, (arr->len - i) * arr->elem_size)) {
            return -1;
        }
    }

    arr->len--;

    return 0;
}

static void * arr_at_no_limit(arr_t * arr, size_t i) {
    return arr->data + (i * arr->elem_size);
}

static int grow_array(arr_t * arr) {
    if (!arr) {
        return -1;
    }

    size_t new_size = arr->size + (arr->size / 2);
    void * new_data = krealloc(arr->data, new_size);
    if (!new_data) {
        return -1;
    }

    arr->data = new_data;
    arr->size = new_size;

    return 0;
}

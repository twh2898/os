#include "libc/circbuff.h"

#include "libc/memory.h"

#if SAFETY > 1
#include "libc/stdio.h"
#define TEST_PTR(REF)                     \
    if (!(REF)) {                         \
        printf(                           \
            "[ERROR] "__FILE__            \
            ":%u Null circular buffer\n", \
            __LINE__);                    \
        return 0;                         \
    }
#define TEST_EMPTY(REF)                       \
    if ((REF)->len == 0) {                    \
        printf(                               \
            "[ERROR] "__FILE__                \
            ":%u Circular buffer is empty\n", \
            __LINE__);                        \
        return 0;                             \
    }
#define TEST_FULL(REF)                       \
    if ((REF)->len >= (REF)->buff_size) {    \
        printf(                              \
            "[ERROR] "__FILE__               \
            ":%u Circular buffer is full\n", \
            __LINE__);                       \
        return 0;                            \
    }
#else
#define TEST_PTR(REF)
#define TEST_EMPTY(REF)
#define TEST_FULL(REF)
#endif

struct _circbuff {
    uint8_t * buff;
    size_t    buff_size;
    size_t    start;
    size_t    len;
};

static size_t _wrap_index(const circbuff_t * cbuff, size_t i) {
    TEST_PTR(cbuff)
    while (i >= cbuff->buff_size) return i -= cbuff->buff_size;
    return i;
}

circbuff_t * circbuff_new(size_t size) {
    if (size == 0)
        return 0;

    circbuff_t * cbuff = kmalloc(sizeof(circbuff_t));
    if (cbuff) {
        cbuff->buff = kmalloc(size);
        if (!cbuff->buff) {
            kfree(cbuff);
            return 0;
        }
        cbuff->buff_size = size;
        cbuff->start     = 0;
        cbuff->len       = 0;
    }
    return cbuff;
}

void circbuff_free(circbuff_t * cbuff) {
    if (cbuff) {
        kfree(cbuff->buff);
        kfree(cbuff);
    }
}

size_t circbuff_buff_size(const circbuff_t * cbuff) {
    if (!cbuff)
        return 0;
    return cbuff->buff_size;
}

size_t circbuff_len(const circbuff_t * cbuff) {
    if (!cbuff)
        return 0;
    return cbuff->len;
}

uint8_t circbuff_at(const circbuff_t * cbuff, size_t index) {
    if (!cbuff || cbuff->len == 0)
        return 0;
    size_t i = _wrap_index(cbuff, cbuff->start + index);
    return cbuff->buff[i];
}

size_t circbuff_push(circbuff_t * cbuff, uint8_t data) {
    if (!cbuff || cbuff->len >= cbuff->buff_size)
        return 0;
    size_t next       = _wrap_index(cbuff, cbuff->start + cbuff->len++);
    cbuff->buff[next] = data;
    return 1;
}

uint8_t circbuff_pop(circbuff_t * cbuff) {
    if (!cbuff || cbuff->len == 0)
        return 0;
    uint8_t val  = circbuff_at(cbuff, 0);
    cbuff->start = _wrap_index(cbuff, cbuff->start + 1);
    cbuff->len--;
    return val;
}

uint8_t circbuff_rpop(circbuff_t * cbuff) {
    if (!cbuff || cbuff->len == 0)
        return 0;
    cbuff->len--;
    uint8_t val = circbuff_at(cbuff, cbuff->len);
    return val;
}

size_t circbuff_insert(circbuff_t * cbuff, uint8_t * data, size_t count) {
    if (!cbuff || !data)
        return 0;

    size_t remainder = cbuff->buff_size - cbuff->len;
    if (count > remainder)
        count = remainder;
    for (size_t i = 0; i < count; i++) {
        circbuff_push(cbuff, data[i]);
    }
    return count;
}

size_t circbuff_read(const circbuff_t * cbuff, uint8_t * data, size_t count) {
    if (!cbuff || !data)
        return 0;

    if (count > cbuff->len)
        count = cbuff->len;
    for (size_t i = 0; i < count; i++) {
        data[i] = circbuff_at(cbuff, i);
    }
    return count;
}

size_t circbuff_remove(circbuff_t * cbuff, size_t count) {
    if (!cbuff)
        return 0;
    if (count > cbuff->len)
        count = cbuff->len;

    cbuff->start = _wrap_index(cbuff, cbuff->start + count);
    cbuff->len -= count;

    return count;
}

size_t circbuff_clear(circbuff_t * cbuff) {
    if (!cbuff)
        return 0;
    size_t o_len = cbuff->len;
    cbuff->start = 0;
    cbuff->len   = 0;
    return o_len;
}

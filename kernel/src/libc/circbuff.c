#include "libc/circbuff.h"

#include "libc/mem.h"

#ifndef SAFETY
#define SAFETY 1
#endif

#if SAFETY
#include "libc/stdio.h"
#define TEST_PTR(REF)                           \
    if (!(REF)) {                               \
        puts("[ERROR] Null circular buffer\n"); \
        return 0;                               \
    }
#define TEST_MIN(REF)                               \
    if ((REF)->len < (REF)->buff_size) {            \
        puts("[ERROR] Circular buffer is empty\n"); \
        return 0;                                   \
    }
#define TEST_MAX(REF)                              \
    if ((REF)->len >= (REF)->buff_size) {          \
        puts("[ERROR] Circular buffer is full\n"); \
        return 0;                                  \
    }
#else
#define TEST_PTR(REF)
#define TEST_MIN(REF)
#define TEST_MAX(REF)
#endif

struct _circbuff {
    uint8_t * buff;
    size_t buff_size;
    size_t start;
    size_t len;
};

static size_t _next_index(const circbuff_t * cbuff, size_t i) {
    TEST_PTR(cbuff)
    while (i >= cbuff->buff_size) return i -= cbuff->buff_size;
    return i + 1;
}

circbuff_t * circbuff_new(size_t size) {
    if (size == 0)
        return 0;

    circbuff_t * cbuff = malloc(sizeof(circbuff_t));
    if (cbuff) {
        cbuff->buff = malloc(size);
        if (!cbuff->buff) {
            free(cbuff);
            return 0;
        }
        cbuff->buff_size = size;
        cbuff->start = 0;
        cbuff->len = 0;
    }
    return cbuff;
}

void circbuff_free(circbuff_t * cbuff) {
    if (cbuff) {
        free(cbuff->buff);
        free(cbuff);
    }
}

size_t circbuff_buff_size(circbuff_t * cbuff) {
    TEST_PTR(cbuff)
    return cbuff->buff_size;
}

size_t circbuff_len(circbuff_t * cbuff) {
    TEST_PTR(cbuff)
    return cbuff->len;
}

uint8_t circbuff_at(circbuff_t * cbuff, size_t index) {
    TEST_PTR(cbuff)
    TEST_MIN(cbuff)
    return cbuff->buff[index];
}

size_t circbuff_push(circbuff_t * cbuff, uint8_t data) {
    TEST_PTR(cbuff)
    if (cbuff->len < cbuff->buff_size) {
        size_t next = _next_index(cbuff, cbuff->start + cbuff->len);
        cbuff->buff[next] = data;
        cbuff->len++;
        return 1;
    }
    return 0;
}

uint8_t circbuff_pop(circbuff_t * cbuff) {
    TEST_PTR(cbuff)
    if (cbuff->len > 0) {
        uint8_t at = cbuff->buff[cbuff->start];
        cbuff->start = _next_index(cbuff, cbuff->start);
        return at;
    }
    return 0;
}

size_t circbuff_insert(circbuff_t * cbuff, uint8_t * data, size_t count) {
    TEST_PTR(cbuff)
    size_t remainder = cbuff->buff_size - cbuff->len;
    if (count > remainder)
        count = remainder;
    for (size_t i = 0; i < count; i++) {
        circbuff_push(cbuff, data[i]);
    }
    return count;
}

size_t circbuff_read(circbuff_t * cbuff, uint8_t * data, size_t count) {
    TEST_PTR(cbuff)
    if (count > cbuff->len)
        count = cbuff->len;
    size_t curr = cbuff->start;
    for (size_t i = 0; i < count; i++) {
        data[i] = circbuff_at(cbuff, curr);
        curr = _next_index(cbuff, curr);
    }
    return count;
}

size_t circbuff_remove(circbuff_t * cbuff, uint8_t * data, size_t count) {
    TEST_PTR(cbuff)
    if (count > cbuff->len)
        count = cbuff->len;
    for (size_t i = 0; i < count; i++) {
        data[i] = circbuff_pop(cbuff);
    }
    return count;
}

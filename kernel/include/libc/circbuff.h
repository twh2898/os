#ifndef CIRCBUFF_H
#define CIRCBUFF_H

#include <stddef.h>
#include <stdint.h>

typedef struct _circbuff circbuff_t;

circbuff_t * circbuff_new(size_t size);
void circbuff_free(circbuff_t * cbuff);

size_t circbuff_buff_size(const circbuff_t * cbuff);
size_t circbuff_len(const circbuff_t * cbuff);

uint8_t circbuff_at(const circbuff_t * cbuff, size_t index);
size_t circbuff_push(circbuff_t * cbuff, uint8_t data);
uint8_t circbuff_pop(circbuff_t * cbuff);

size_t circbuff_insert(circbuff_t * cbuff, uint8_t * data, size_t count);
size_t circbuff_read(const circbuff_t * cbuff, uint8_t * data, size_t count);
size_t circbuff_remove(circbuff_t * cbuff, uint8_t * data, size_t count);

size_t circbuff_clear(circbuff_t * cbuff);

#endif // CIRCBUFF_H

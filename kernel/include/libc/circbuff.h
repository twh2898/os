#ifndef CIRCBUFF_H
#define CIRCBUFF_H

#include <stdint.h>
#include <stddef.h>

typedef struct _circbuff circbuff_t;

circbuff_t * circbuff_new(size_t size);
void circbuff_free(circbuff_t * cbuff);

size_t circbuff_buff_size(circbuff_t * cbuff);
size_t circbuff_lens(circbuff_t * cbuff);

uint8_t circbuff_at(circbuff_t * cbuff, size_t index);
size_t circbuff_push(circbuff_t * cbuff, uint8_t data);
uint8_t circbuff_pop(circbuff_t * cbuff);

size_t circbuff_insert(circbuff_t * cbuff, uint8_t * data, size_t count);
size_t circbuff_read(circbuff_t * cbuff, uint8_t * data, size_t count);
size_t circbuff_remove(circbuff_t * cbuff, uint8_t * data, size_t count);

#endif // CIRCBUFF_H

#ifndef CIRCBUFF_H
#define CIRCBUFF_H

#include <stddef.h>
#include <stdint.h>

typedef struct _circbuff circbuff_t;

/**
 * @brief Create an empty circular buffer with given maximum size.
 * 
 * Will return null if size is 0.
 * 
 * @param size maximum buffer size
 * @return circbuff_t* new buffer
 */
circbuff_t * circbuff_new(size_t size);

/**
 * @brief Free a circular buffer.
 * 
 * @pre cbuff is not null and is a valid circular buffer
 * @param cbuff buffer
 */
void circbuff_free(circbuff_t * cbuff);

/**
 * @brief Get the maximum length of the buffer.
 * 
 * This is the size given to circbuff_new
 * 
 * @param cbuff buffer
 * @return size_t max length of buffer
 */
size_t circbuff_buff_size(const circbuff_t * cbuff);

/**
 * @brief Get the current buffer length.
 * 
 * @param cbuff buffer
 * @return size_t number of elements in the buffer
 */
size_t circbuff_len(const circbuff_t * cbuff);

/**
 * @brief Get element at index
 * 
 * If the buffer is empty, return 0.
 * 
 * @param cbuff buffer
 * @param index index from start
 * @return uint8_t value at index
 */
uint8_t circbuff_at(const circbuff_t * cbuff, size_t index);

/**
 * @brief Add an element to the end of the buffer.
 * 
 * If the buffer is full, return 0.
 * 
 * @param cbuff buffer
 * @param data element to append
 * @return size_t 1 if data is appended, 0 if buffer is full
 */
size_t circbuff_push(circbuff_t * cbuff, uint8_t data);

/**
 * @brief Remove and return an element from the start of the buffer.
 * 
 * If the buffer is empty, return 0.
 * 
 * @param cbuff buffer
 * @return uint8_t element or 0 if buffer is empty
 */
uint8_t circbuff_pop(circbuff_t * cbuff);

size_t circbuff_insert(circbuff_t * cbuff, uint8_t * data, size_t count);
size_t circbuff_read(const circbuff_t * cbuff, uint8_t * data, size_t count);
size_t circbuff_remove(circbuff_t * cbuff, uint8_t * data, size_t count);

size_t circbuff_clear(circbuff_t * cbuff);

#endif // CIRCBUFF_H

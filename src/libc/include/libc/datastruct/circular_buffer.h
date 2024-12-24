#ifndef LIBC_DATASTRUCT_CIRCULAR_BUFFER_H
#define LIBC_DATASTRUCT_CIRCULAR_BUFFER_H

#include <stddef.h>

typedef struct _ds_cb cb_t;

/**
 * @brief Create a new circular buffer with `size` elements of `elem_size` bytes
 * each.
 *
 * @param size number of elements in buffer
 * @param elem_size size of each element in bytes
 * @return cb_t* pointer to the new buffer or 0 for fail
 */
cb_t * cb_new(size_t size, size_t elem_size);

/**
 * @brief Free a circular buffer.
 *
 * @param cb pointer to the buffer
 */
void cb_free(cb_t * cb);

/**
 * @brief Get the number of elements the buffer can hold.
 *
 * This is the max value cb_len can return.
 *
 * @param cb pointer to the buffer
 * @return size_t number of elements
 */
size_t cb_buff_size(cb_t * cb);

/**
 * @brief Get the number of elements present in the buffer.
 *
 * This can be no larger than cb_buff_size
 *
 * @param cb pointer to the buffer
 * @return size_t number of elements
 */
size_t cb_len(cb_t * cb);

/**
 * @brief Get a pointer to the value at index i.
 *
 * @param cb pointer to the buffer
 * @param i index of the element
 * @return void* pointer to the element or 0 for fail
 */
void * cb_peek(cb_t * cb, size_t i);

/**
 * @brief Add an element to the end of the buffer.
 *
 * @param cb pointer to the buffer
 * @param item item to insert
 * @return int 0 for success
 */
int cb_push(cb_t * cb, const void * item);

/**
 * @brief Remove an element from the back of the buffer.
 *
 * If `item` is not 0, the value of the first element will be copied to item
 * before poping.
 *
 * @param cb pointer to the buffer
 * @param item optional output of item value
 * @return int 0 for success
 */
int cb_pop(cb_t * cb, void * item);

#endif // LIBC_DATASTRUCT_CIRCULAR_BUFFER_H

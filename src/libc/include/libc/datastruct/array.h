#ifndef LIBC_DATASTRUCT_ARRAY_H
#define LIBC_DATASTRUCT_ARRAY_H

#include <stddef.h>

typedef struct _ds_arr {
    void * data;
    size_t len;
    size_t size;
    size_t elem_size;
} arr_t;

/**
 * @brief Create a new array with pre-allocated `size` elements of `elem_size`
 * bytes each.
 *
 * `arr` is expected to be existing memory that will be used by the function.
 *
 * @param arr pointer to the array struct
 * @param size number of elements to pre-allocate
 * @param elem_size size of each element in bytes
 * @return int 0 for success
 */
int arr_create(arr_t * arr, size_t size, size_t elem_size);

/**
 * @brief Free array's buffer.
 *
 * This does not free the memory pointed to by `arr`.
 *
 * @param arr pointer to the array
 */
void arr_free(arr_t * arr);

/**
 * @brief Get the number of elements present in the array.
 *
 * @param arr pointer to the array
 * @return size_t number of elements
 */
size_t arr_size(const arr_t * arr);

/**
 * @brief Get a pointer to the array's data.
 *
 * This pointer is invalid after the next call to insert or remove.
 *
 * @param arr pointer to the array
 * @return void* pointer to the data
 */
void * arr_data(const arr_t * arr);

/**
 * @brief Get a pointer to the value at index i.
 *
 * @param arr pointer to the array
 * @param i index of the element
 * @return void* pointer to the element or 0 for fail
 */
void * arr_at(const arr_t * arr, size_t i);

/**
 * @brief Set the value of the element at index i.
 *
 * @param arr pointer to the array
 * @param i index of the element
 * @param item value to set
 * @return int 0 for success
 */
int arr_set(arr_t * arr, size_t i, const void * item);

/**
 * @brief Get the value of the element at index i.
 *
 * @param arr pointer to the array
 * @param i index of the element
 * @param item pointer to output
 * @return int 0 for success
 */
int arr_get(const arr_t * arr, size_t i, void * item);

/**
 * @brief Add an element to the array.
 *
 * If there is no space in the array, a new internal array will be allocated
 * with a copy of the current contents.
 *
 * @param arr pointer to the array
 * @param i array index
 * @param item item to insert
 * @return int 0 for success
 */
int arr_insert(arr_t * arr, size_t i, const void * item);

/**
 * @brief Remove an element from the array.
 *
 * If `item` is not 0, the value of the element will be copied to item before
 * removing.
 *
 * @param arr pointer to the array
 * @param i array index
 * @param item optional output of item value
 * @return int 0 for success
 */
int arr_remove(arr_t * arr, size_t i, void * item);

#endif // LIBC_DATASTRUCT_ARRAY_H

#ifndef LIBC_DATASTRUCT_ARRAY_H
#define LIBC_DATASTRUCT_ARRAY_H

#include <stddef.h>

typedef struct _ds_arr ds_arr_t;

#include <stddef.h>

typedef struct _ds_arr arr_t;

/**
 * @brief Create a new array with pre-allocated `size` elements of `elem_size`
 * bytes each.
 *
 * The pre-allocated size will grow as elements are added.
 *
 * @param size number of elements to pre-allocate
 * @param elem_size size of each element in bytes
 * @return arr_t* pointer to the new array or 0 for fail
 */
arr_t * arr_new(size_t size, size_t elem_size);

/**
 * @brief Free a array.
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
size_t arr_size(arr_t * arr);

/**
 * @brief Get a pointer to the array's data.
 *
 * This pointer is invalid after the next call to insert or remove.
 *
 * @param arr pointer to the array
 * @return void* pointer to the data
 */
void * arr_data(arr_t * arr);

/**
 * @brief Get a pointer to the value at index i.
 *
 * @param arr pointer to the array
 * @param i index of the element
 * @return void* pointer to the element or 0 for fail
 */
void * arr_at(arr_t * arr, size_t i);

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
int arr_get(arr_t * arr, size_t i, void * item);

/**
 * @brief Add an element to the end of the array.
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
 * @brief Remove an element from the back of the array.
 *
 * If `item` is not 0, the value of the first element will be copied to item
 * before poping.
 *
 * @param arr pointer to the array
 * @param i array index
 * @param item optional output of item value
 * @return int 0 for success
 */
int arr_remove(arr_t * arr, size_t i, void * item);

#endif // LIBC_DATASTRUCT_ARRAY_H

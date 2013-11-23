#ifndef ARRAY_H
#define ARRAY_H

#include <stdlib.h> /* size_t */

struct array
{
    size_t capacity;
    size_t length;

    size_t element_size;
    unsigned char *data;
};


struct array *array_new(size_t elemsize);
struct array *array_new_prealloc(size_t elemsize, size_t elemcount);

#define ARRAY_NEW(t)             array_new(sizeof(t))
#define ARRAY_NEW_PREALLOC(t, n) array_new_prealloc(sizeof(t), (n));

void array_free(struct array *arr);

/* Returns the managed array data casted to a pointer to t */
#define ARRAY_DATA(a, t) ((t*)((a)->data))

/*
 * Analogous to arr[i] (can be assigned to, can overflow, does NOT update
 * array length on assign!) - use only for fast access within known boundaries
 * or when only using functions that do not depend on the current length.
 */
#define ARRAY_INDEX(a, i, t) ARRAY_DATA(a, t)[i]

size_t array_length(struct array *arr);
size_t array_size(struct array *arr);
size_t array_capacity(struct array *arr);

void array_resize(struct array *arr, size_t elemcount);

void array_insert(struct array *arr, size_t pos, const void *elems, size_t n);
void array_append(struct array *arr, const void *elems, size_t n);
void array_prepend(struct array *arr, const void *elem, size_t n);

void array_remove(struct array *arr, size_t pos, size_t n);

#define ARRAY_CLEAR(a)        array_remove((a), 0, array_length((a)))
#define ARRAY_INSERT(a, p, e) array_insert((a), (p), (e), 1)
#define ARRAY_REMOVE(a, p)    array_remove((a), (p), 1)
#define ARRAY_APPEND(a, e)    array_append((a), (e), 1)
#define ARRAY_PREPEND(a, e)   array_prepend((a), (e), 1)

void array_sort(struct array *arr, int (*cmpfn)(const void *, const void *));

#endif /* defined ARRAY_H */

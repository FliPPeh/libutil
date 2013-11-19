#include "array.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


struct array *array_new(size_t elemsize)
{
    return array_new_prealloc(elemsize, 0);
}

struct array *array_new_prealloc(size_t elemsize, size_t elemcount)
{
    struct array *arr = malloc(sizeof(*arr));

    if (arr != NULL) {
        memset(arr, 0, sizeof(*arr));

        arr->capacity = elemcount;
        arr->element_size = elemsize;

        // Because malloc(0) is implementation defined.
        if (elemcount > 0) {
            arr->data = malloc(elemcount * elemsize);
            memset(arr->data, 0, elemcount * elemsize);
        }
    }

    return arr;
}

void array_free(struct array *arr)
{
    free(arr->data);
    free(arr);
}

size_t array_length(struct array *arr)
{
    return arr->length;
}

size_t array_size(struct array *arr)
{
    return arr->capacity * arr->element_size;
}

size_t array_capacity(struct array *arr)
{
    return arr->capacity;
}

void array_resize(struct array *arr, size_t elemcount)
{
    size_t oldcap = arr->capacity * arr->element_size;
    size_t newcap = elemcount     * arr->element_size;

    arr->data = realloc(arr->data, newcap);
    arr->capacity = elemcount;

    if (oldcap < newcap)
        memset(arr->data + oldcap, 0, newcap - oldcap);
}

void array_insert(struct array *arr, size_t pos, const void *elems, size_t n)
{
    if (pos > arr->length)
        pos = arr->length;

    if (arr->capacity < (arr->length + n))
        array_resize(arr, arr->length + n);

    memmove(arr->data + ((pos + n) * arr->element_size),
            arr->data + (pos * arr->element_size),
            (arr->length - pos) * arr->element_size);

    memmove(arr->data + (pos * arr->element_size),
            elems,
            n * arr->element_size);

    arr->length += n;
}

void array_remove(struct array *arr, size_t pos, size_t n)
{
    if (pos >= arr->length)
        return;

    if ((pos + n) > arr->length)
        n = arr->length - pos;

    memmove(arr->data + (pos * arr->element_size),
            arr->data + ((pos + n) * arr->element_size),
            (arr->length - (pos + n)) * arr->element_size);

    arr->length -= n;
    array_resize(arr, arr->length);
}

void array_append(struct array *arr, const void *elems, size_t n)
{
    array_insert(arr, array_length(arr), elems, n);
}

void array_prepend(struct array *arr, const void *elems, size_t n)
{
    array_insert(arr, 0, elems, n);
}

void array_sort(struct array *arr, int (*cmpfn)(const void *, const void *))
{
    qsort(arr->data, arr->length, arr->element_size, cmpfn);
}


int cmp(const void *a, const void *b)
{
    return *(int*)a - *(int*)b;
}


#ifndef HEAP_H
#define HEAP_H

#include <libutil/libutil.h>

#include <stdlib.h>

enum heap_order
{
    HEAP_MAX,
    HEAP_MIN
};

struct heap_element
{
    void *data;
    int priority;
};

struct heap
{
    struct heap_element *heap;
    enum heap_order order;

    size_t capacity;
    size_t usage;
};

#define HEAP_ELEMENT_PRIORITY(elem) (elem)->priority
#define HEAP_ELEMENT_DATA(elem) (elem)->data

void heap_init(struct heap *heap);
void heap_init_heapmax(struct heap *heap);
void heap_free(struct heap *heap);

size_t heap_size(struct heap *heap);

void heap_insert(struct heap *heap, void *data, int priority);
struct heap_element *heap_get_max(struct heap *heap, int *prio);
struct heap_element *heap_pop_max(struct heap *heap, int *prio);

#endif /* defined HEAP_H */

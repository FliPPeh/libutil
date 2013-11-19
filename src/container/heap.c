#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "heap.h"

void heap_init(struct heap *heap)
{
    memset(heap, 0, sizeof(*heap));

    heap->heap = NULL;
    heap->capacity = 0;
    heap->usage = 0;
    heap->order = HEAP_MIN;
}

void heap_init_heapmax(struct heap *heap)
{
    heap_init(heap);

    heap->order = HEAP_MAX;
}

void heap_free(struct heap *heap)
{
    free(heap->heap);
}

void heap_insert(struct heap *heap, void *data, int priority)
{
    struct heap_element elem = { data, priority };

    if (heap->capacity > 0) {
        if (heap->usage >= heap->capacity)
            _heap_increase(heap);

        memcpy(&(heap->heap[heap->usage]), &elem, sizeof(elem));

        _heap_heapup(heap, heap->usage++);
    } else {
        _heap_increase(heap);

        memcpy(&(heap->heap[heap->usage++]), &elem, sizeof(elem));
    }
}

struct heap_element *heap_get_max(struct heap *heap)
{
    return (heap->usage > 0) ? &(heap->heap[0]) : NULL;
}

void *heap_pop_max(struct heap *heap)
{
    if (heap->usage > 0) {
        void *data = heap_get_max(heap)->data;

        heap->heap[0] = heap->heap[--(heap->usage)];
        _heap_heapdown(heap, 0);

        return data;
    }

    return NULL;
}


void _heap_increase(struct heap *heap)
{
    heap->heap = realloc(heap->heap,
            sizeof(struct heap_element) * (heap->capacity * 2 + 1));

    heap->capacity = heap->capacity * 2 + 1;
}

void _heap_heapup(struct heap *heap, int ind)
{
    int child = ind;

    while (child) {
        int parent = heap_get_parent(child);

        if (_heap_needs_swap(heap, parent, child)) {
            _heap_swap(heap, parent, child);

            child = parent;
        } else {
            break;
        }
    }
}

void _heap_heapdown(struct heap *heap, int ind)
{
    int root = ind;
    size_t child;

    while ((child = heap_get_left_child(root)) <= heap->usage) {
        int swap = root;

        if (_heap_needs_swap(heap, swap, child)) {
            swap = child;
        }

        if (child + 1 <= heap->usage) {
            if (_heap_needs_swap(heap, swap, child + 1)) {
                swap = child;
            }
        }

        if (swap != root) {
            _heap_swap(heap, root, swap);
            root = swap;
        } else {
            break;
        }
    }
}

int _heap_needs_swap(struct heap *heap, int a, int b)
{
    struct heap_element *ha = &heap->heap[a];
    struct heap_element *hb = &heap->heap[b];

    if (heap->order == HEAP_MAX)
        return ha->priority < hb->priority;
    else
        return ha->priority > hb->priority;
}

void _heap_swap(struct heap *heap, int a, int b)
{
    struct heap_element tmp;

    memcpy(&tmp, &heap->heap[a], sizeof(struct heap_element));

    memcpy(&heap->heap[a], &heap->heap[b], sizeof(struct heap_element));
    memcpy(&heap->heap[b], &tmp, sizeof(struct heap_element));
}

#include <libutil/libutil.h>
#include <libutil/container/heap.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HEAP_GET_PARENT(i) (int)((i - 1) / 2)
#define HEAP_GET_LEFT_CHILD(i) (2 * i + 1)
#define HEAP_GET_RIGHT_CHILD(i) (2 * i + 2)

static void _heap_increase(struct heap *heap);
static void _heap_heapup(struct heap *heap, int ind);
static void _heap_heapdown(struct heap *heap, int ind);

static int _heap_needs_swap(struct heap *heap, int a, int b);
static void _heap_swap(struct heap *heap, int a, int b);

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

size_t heap_size(struct heap *heap)
{
    return heap->usage;
}

void heap_insert(struct heap *heap, void *data, int priority)
{
    struct heap_element elem;

    elem.data = data;
    elem.priority = priority;

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

struct heap_element *heap_get_max(struct heap *heap, int *prio)
{
    if (heap->usage > 0) {
        if (prio != NULL) {
            *prio = heap->heap[0].priority;
        }

        return heap->heap[0].data;
    }

    return NULL;
}

struct heap_element *heap_pop_max(struct heap *heap, int *prio)
{
    if (heap->usage > 0) {
        void *data = heap_get_max(heap, prio);

        heap->heap[0] = heap->heap[--(heap->usage)];
        _heap_heapdown(heap, 0);

        return data;
    }

    return NULL;
}


static void _heap_increase(struct heap *heap)
{
    heap->heap = realloc(heap->heap,
            sizeof(struct heap_element) * (heap->capacity * 2 + 1));

    heap->capacity = heap->capacity * 2 + 1;
}

static void _heap_heapup(struct heap *heap, int ind)
{
    int child = ind;

    while (child) {
        int parent = HEAP_GET_PARENT(child);

        if (_heap_needs_swap(heap, parent, child)) {
            _heap_swap(heap, parent, child);

            child = parent;
        } else {
            break;
        }
    }
}

static void _heap_heapdown(struct heap *heap, int ind)
{
    int root = ind;
    size_t child;

    while ((child = HEAP_GET_LEFT_CHILD(root)) <= heap->usage) {
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

static int _heap_needs_swap(struct heap *heap, int a, int b)
{
    struct heap_element *ha = &heap->heap[a];
    struct heap_element *hb = &heap->heap[b];

    if (heap->order == HEAP_MAX)
        return ha->priority < hb->priority;
    else
        return ha->priority > hb->priority;
}

static void _heap_swap(struct heap *heap, int a, int b)
{
    struct heap_element tmp;

    memcpy(&tmp, &heap->heap[a], sizeof(struct heap_element));

    memcpy(&heap->heap[a], &heap->heap[b], sizeof(struct heap_element));
    memcpy(&heap->heap[b], &tmp, sizeof(struct heap_element));
}

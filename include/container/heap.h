#ifndef HEAP_H
#define HEAP_H

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

#define heap_element_priority(elem) (elem)->priority
#define heap_element_data(elem) (elem)->data

#define heap_get_parent(i) (int)((i - 1) / 2)
#define heap_get_left_child(i) (2 * i + 1)
#define heap_get_right_child(i) (2 * i + 2)

void heap_init(struct heap *heap);
void heap_init_heapmax(struct heap *heap);
void heap_free(struct heap *heap);

void heap_insert(struct heap *heap, void *data, int priority);
struct heap_element *heap_get_max(struct heap *heap);
void *heap_pop_max(struct heap *heap);

void _heap_increase(struct heap *heap);
void _heap_heapup(struct heap *heap, int ind);
void _heap_heapdown(struct heap *heap, int ind);

int _heap_needs_swap(struct heap *heap, int a, int b);
void _heap_swap(struct heap *heap, int a, int b);

#endif /* defined HEAP_H */

#ifndef SLIST_H
#define SLIST_H

#include <libutil/libutil.h>

#include <stdlib.h>

struct slist
{
    void *data;

    struct slist *next;
};

#define SLIST_NEXT(list)              ((list)->next)
#define SLIST_DATA(list, type) ((type)((list)->data))

typedef int (*slist_compare_func)(const void *a, const void *b);
typedef void (*slist_data_func)(void *data, void *userdata);
typedef void (*slist_delete_func)(void *data);
typedef void *(*slist_copy_func)(const void *data, void *userdata);


struct slist *slist_new();
struct slist *slist_new_with_data(void *data);

void slist_free(struct slist *link, slist_delete_func fn);

struct slist *slist_append(struct slist *list, void *data);
struct slist *slist_prepend(struct slist *list, void *data);
struct slist *slist_insert(struct slist *list, void *data, int position);

struct slist *slist_insert_before(
        struct slist *list, struct slist *pos, void *data);

struct slist *slist_insert_sorted(
        struct slist *list, void *data, slist_compare_func fn);

struct slist *slist_remove(
        struct slist *list, const void *data, slist_delete_func fn);

struct slist *slist_remove_all(
        struct slist *list, const void *data, slist_delete_func fn);

struct slist *slist_remove_link(
        struct slist *list, struct slist *link, slist_delete_func fn);

void slist_free_all(struct slist *list, slist_delete_func fn);

size_t slist_length(struct slist *list);
void slist_foreach(struct slist *list, slist_data_func fn, void *ud);

struct slist *slist_copy(struct slist *list);
struct slist *slist_copy_deep(struct slist *list, slist_copy_func fn, void *ud);

struct slist *slist_reverse(struct slist *list);
struct slist *slist_sort(struct slist *list, slist_compare_func fn);

struct slist *slist_concat(struct slist *list1, struct slist *list2);

struct slist *slist_last(struct slist *list);
struct slist *slist_nth(struct slist *list, unsigned int n);

struct slist *slist_find(struct slist *list, const void *data);
struct slist *slist_find_custom(
        struct slist *list, const void *data, slist_compare_func fn);

int slist_position(struct slist *list, struct slist *link);
int slist_index(struct slist *list, const void *data);

#endif /* defined SLIST_H */

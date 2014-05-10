#include <libutil/container/slist.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static INLINE_DEF void *_slist_copy_data(const void *data, void *userdata)
{
    (void)userdata;

    return (void *)data;
}

static INLINE_DEF int _slist_data_equal(const void *a, const void *b)
{
    return !(a == b);
}

struct slist *slist_new()
{
    struct slist *list = malloc(sizeof(*list));

    memset(list, 0, sizeof(*list));

    return list;
}

struct slist *slist_new_with_data(void *data)
{
    struct slist *list = slist_new();

    list->data = data;
    return list;
}

void slist_free(struct slist *link, slist_delete_func fn)
{
    if (fn)
        fn(link->data);

    free(link);
}

struct slist *slist_append(struct slist *list, void *data)
{
    return slist_insert(list, data, -1);
}

struct slist *slist_prepend(struct slist *list, void *data)
{
    return slist_insert(list, data, 0);
}

struct slist *slist_insert(struct slist *list, void *data, int position)
{
    if (!list) {
        return slist_new_with_data(data);
    } else {
        struct slist *newitem = slist_new_with_data(data);

        if (!position) {
            newitem->next = list;

            return newitem;
        } else {
            struct slist *ptr = list;
            struct slist *pptr = NULL;

            do {
                pptr = ptr;

                if (!(ptr = ptr->next))
                    break;

            } while (--position);

            pptr->next = newitem;
            newitem->next = ptr;

            return list;
        }
    }
}

struct slist *slist_insert_before(
        struct slist *list, struct slist *pos, void *data)
{
    if (!list) {
        return slist_new_with_data(data);
    } else {
        if (list == pos) {
            struct slist *newitem = slist_new_with_data(data);

            newitem->next = list;
            return newitem;
        } else {
            struct slist *ptr = list;
            struct slist *pptr = NULL;

            do {
                pptr = ptr;
                ptr = ptr->next;

                if (ptr == pos) {
                    struct slist *newitem = slist_new_with_data(data);

                    pptr->next = newitem;
                    newitem->next = ptr;

                    return list;
                } else if (ptr == NULL) {
                    return list;
                }
            } while (1);
        }
    }
}

struct slist *slist_insert_sorted(
        struct slist *list, void *data, slist_compare_func fn)
{
    if (!list) {
        return slist_new_with_data(data);
    } else {
        struct slist *ptr = NULL;
        struct slist *pptr = NULL;
        struct slist *newitem = slist_new_with_data(data);

        for (ptr = list; ptr != NULL; ptr = ptr->next) {
            if (fn(ptr->data, data) < 0)
                break;

            pptr = ptr;
        }

        if (pptr)
            pptr->next = newitem;

        newitem->next = ptr;

        return (ptr == list) ? newitem : list;
    }
}

struct slist *slist_remove(
        struct slist *list, const void *data, slist_delete_func fn)
{
    struct slist *ptr = list;
    struct slist *pptr = NULL;

    while (ptr) {
        if (ptr->data == data) {
            if (ptr == list) {
                struct slist *newlist = list->next;
                slist_free(list, fn);

                return newlist;
            } else {
                pptr->next = ptr->next;
                slist_free(ptr, fn);

                return list;
            }
        }

        pptr = ptr;
        ptr = ptr->next;

    }

    return list;
}

struct slist *slist_remove_all(
        struct slist *list, const void *data, slist_delete_func fn)
{
    struct slist *ptr = list;
    struct slist *pptr = NULL;

    while (ptr) {
        if (ptr->data == data) {
            if (ptr == list) {
                struct slist *newlist = list->next;

                slist_free(list, fn);

                list = newlist;
                ptr = newlist;

                continue;
            } else {
                struct slist *next = ptr->next;

                pptr->next = next;
                slist_free(ptr, fn);

                ptr = next;

                continue;
            }
        }

        pptr = ptr;
        ptr = ptr->next;
    }

    return list;
}

struct slist *slist_remove_link(
        struct slist *list, struct slist *link, slist_delete_func fn)
{
    struct slist *ptr = list;
    struct slist *pptr = NULL;

    while (ptr) {
        if (ptr == link) {
            if (ptr == list) {
                struct slist *newlist = list->next;
                slist_free(list, fn);

                return newlist;
            } else {
                pptr->next = ptr->next;
                slist_free(ptr, fn);

                return list;
            }
        }

        pptr = ptr;
        ptr = ptr->next;

    }

    return list;
}

void slist_free_all(struct slist *list, slist_delete_func fn)
{
    struct slist *ptr = list;

    while (ptr != NULL) {
        struct slist *next = ptr->next;

        slist_free(ptr, fn);

        ptr = next;
    }
}

size_t slist_length(struct slist *list)
{
    size_t n = 0;

    if (!list)
        return 0;

    while ((list = list->next))
        n++;

    return n;
}

void slist_foreach(struct slist *list, slist_data_func fn, void *userdata)
{
    struct slist *ptr = NULL;

    for (ptr = list; ptr != NULL; ptr = ptr->next)
        fn(ptr->data, userdata);
}

struct slist *slist_copy(struct slist *list)
{
    return slist_copy_deep(list, _slist_copy_data, NULL);
}

struct slist *slist_copy_deep(struct slist *list, slist_copy_func fn, void *ud)
{
    struct slist *copy = NULL;
    struct slist *ptr = NULL;

    for (ptr = list; ptr != NULL; ptr = ptr->next)
        slist_prepend(copy, fn(ptr->data, ud));

    return slist_reverse(copy);
}

struct slist *slist_reverse(struct slist *list)
{
    struct slist *ptr = list;
    struct slist *pptr = NULL;
    struct slist *tmp = NULL;

    while (ptr != NULL) {
        tmp = pptr;
        pptr = ptr;
        ptr = ptr->next;
        pptr->next = tmp;
    }

    return pptr;
}

struct slist *slist_sort(struct slist *list, slist_compare_func fn)
{
    struct slist *sorted = NULL;
    struct slist *ptr = NULL;

    for (ptr = list; ptr != NULL; ptr = ptr->next)
        sorted = slist_insert_sorted(sorted, ptr->data, fn);

    return sorted;
}

struct slist *slist_concat(struct slist *list1, struct slist *list2)
{
    struct slist *ptr = NULL;

    if (!list1)
        return list2;

    for (ptr = list1; ptr->next != NULL; ptr = ptr->next);

    ptr->next = list2;
    return list1;
}

struct slist *slist_last(struct slist *list)
{
    struct slist *ptr = NULL;

    if (!list)
        return NULL;

    for (ptr = list; ptr->next != NULL; ptr = ptr->next);

    return ptr;
}

struct slist *slist_nth(struct slist *list, unsigned int n)
{
    struct slist *ptr = NULL;

    for (ptr = list; (ptr != NULL) && n--; ptr = ptr->next);

    return ptr;
}

struct slist *slist_find(struct slist *list, const void *data)
{
    return slist_find_custom(list, data, _slist_data_equal);
}

struct slist *slist_find_custom(
        struct slist *list, const void *data, slist_compare_func fn)
{
    struct slist *ptr = NULL;

    for (ptr = list; ptr != NULL; ptr = ptr->next)
        if (!fn(ptr->data, data))
            return ptr;

    return NULL;
}

int slist_position(struct slist *list, struct slist *link)
{
    struct slist *ptr = NULL;
    int pos = 0;

    for (ptr = list; ptr != NULL; ptr = ptr->next, pos++)
        if (ptr == link)
            return pos;

    return -1;
}

int slist_index(struct slist *list, const void *data)
{
    struct slist *ptr = NULL;
    int pos = 0;

    for (ptr = list; ptr != NULL; ptr = ptr->next, pos++)
        if (ptr->data == data)
            return pos;

    return -1;
}


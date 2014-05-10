#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <libutil/libutil.h>
#include <libutil/container/list.h>

#include <stdlib.h>

#define HASHTABLE_INIT_SIZE 4
#define AUTOREHASH             /* undefine to never automatically rehash   */
#define HASHTABLE_MINLOAD 0.12 /* minimum load factor for rehash on delete */
#define HASHTABLE_MAXLOAD 0.75 /* maximum load factor for rehash on insert */

typedef size_t (*hashtable_hash_func)(const void *key);
typedef int (*hashtable_equality_func)(const void *ka, const void *kb);
typedef void (*hashtable_delete_func)(void *data);

struct hashtable
{
    size_t bucket_count;
    size_t entries;
    struct list **buckets;

    hashtable_hash_func key_hash;
    hashtable_equality_func key_equal;

    hashtable_delete_func free_key;
    hashtable_delete_func free_value;
};

struct hashtable_entry
{
    void *key;
    void *value;
};

struct hashtable_iterator
{
    const struct hashtable *table;

    size_t bucket;
    struct list *lst;
};

struct hashtable *hashtable_new(hashtable_hash_func hsh,
                                hashtable_equality_func eq);

struct hashtable *hashtable_new_with_free(hashtable_hash_func hsh,
                                          hashtable_equality_func eq,
                                          hashtable_delete_func fkey,
                                          hashtable_delete_func fvalue);

struct hashtable *hashtable_new_real(size_t buckets,
                                     hashtable_hash_func hsh,
                                     hashtable_equality_func eq,
                                     hashtable_delete_func fkey,
                                     hashtable_delete_func fvalue);

struct hashtable *hashtable_new_from(struct hashtable *orig);

void hashtable_free(struct hashtable *table);

/*
 * Inserts an item into the hashmap. Should key already exist, it replaces its
 * value with the new value, freeing the old one if a deleter function for the
 * value was given.
 */
void hashtable_insert(struct hashtable *table, void *key, void *value);

/*
 * Removing and clearing. Remove removes the given key from the hashtable (if
 * it exists), clear removes all keys from the hashtable. Their _shallow
 * variants also remove the key/value pairs from the hashtable, but do not call
 * the release functions for either (in other words, they only remove the
 * entries from the hashtable, but don't free their memory).
 */
void hashtable_remove(struct hashtable *table, const void *key);
void hashtable_remove_shallow(struct hashtable *table, const void *key);

void hashtable_clear(struct hashtable *table);
void hashtable_clear_shallow(struct hashtable *table);

void *hashtable_lookup(const struct hashtable *table, const void *key);
bool hashtable_contains(const struct hashtable *table, const void *key);
size_t hashtable_size(const struct hashtable *table);

double hashtable_load_factor(const struct hashtable *table);

/*
 * Rehash the hashtable by (depending on the load factor, see HASHTABLE_MAXLOAD,
 * HASHTABLE_MINLOAD) increasing or decreasing the number of buckets to reduce
 * seek time (for many elements) and reduce memory usage (for few elements).
 * The order of the contained entries may change, but the amount will not.
 */
void hashtable_rehash(struct hashtable *table);

struct list *hashtable_keys(const struct hashtable *table);
struct list *hashtable_values(const struct hashtable *table);

/*
 * Set operations on hashtables.
 *
 * hashtable_union() merges all key/value pairs from B into A.
 * hashtable_complement() removes all keys not in B from A.
 *
 * These functions are intended for merging changes that could not be done in
 * place while iterating A. Hashtable A takes ownership of added key/value paris
 * so B must not free them. If B was created using hashtable_new_from(), perform
 * a shallow clear on B before destroying it.
 */
void hashtable_union(struct hashtable *a, struct hashtable *b);
void hashtable_complement(struct hashtable *a, struct hashtable *b);

void hashtable_iterator_init(struct hashtable_iterator *iter,
                             const struct hashtable *t);

bool hashtable_iterator_next(struct hashtable_iterator *iter,
                             void **tkey,
                             void **tval);

#if __STDC_VERSION__ >= 201112L
    #define HASHTABLE_NEW(T) hashtable_new(HASHFUNC_FOR(T), EQUALFUNC_FOR(T))

    #define HASHTABLE_NEW_WRITH_FREE(T, fkey, fval) \
        hashtable_new_with_free(HASHFUNC_FOR(T),    \
                                EQUALFUNC_FOR(T), (fkey), (fval))

    /* :) */
    #define __CONST_NONCONST(T, fun)  T: fun, const T: fun
    #define __SIGNED_UNSIGNED(T, fun) T: fun, unsigned T: unsigned_ ## fun
    #define __SIGNED_UNSIGNED_NEITHER(T, fun) \
                 T:              fun, \
          signed T:              fun, \
        unsigned T: unsigned_ ## fun

    #define FUNC_FOR(T, type) _Generic((T)0,                  \
                         _Bool:               bool ## type,   \
        __CONST_NONCONST( char *,              str ## type),  \
        __SIGNED_UNSIGNED_NEITHER(char,       char ## type),  \
        __SIGNED_UNSIGNED(int,                 int ## type),  \
        __SIGNED_UNSIGNED(short,             short ## type),  \
        __SIGNED_UNSIGNED(long,               long ## type),  \
        __SIGNED_UNSIGNED(long long,     long_long ## type))

    #define HASHFUNC_FOR(T) FUNC_FOR(T, _hash)
    #define EQUALFUNC_FOR(T) FUNC_FOR(T, _equal)
#endif

size_t ascii_hash(const void *k);
int ascii_equal(const void *a, const void *b);

size_t str_hash(const void *k);
int str_equal(const void *a, const void *b);

/* Autogenerate hashing and equality checking for primitive types */
#define PTYPES              \
    X(char,      char)      \
    X(int,       int)       \
    X(short,     short)     \
    X(long,      long)      \
    X(long long, long_long)

#define X(T, fn)                                       \
    size_t fn ## _hash(const void *k);                 \
    int    fn ## _equal(const void *a, const void *b); \
                                                       \
    size_t unsigned_ ## fn ## _hash(const void *k);                 \
    int    unsigned_ ## fn ## _equal(const void *a, const void *b);
PTYPES
#undef X

#if __STDC_VERSION__ >= 199901L
    size_t bool_hash(const void *k);
    int bool_equal(const void *a, const void *b);
#endif

#endif /* defined HASHTABLE_H */

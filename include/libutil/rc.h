#ifndef RC_H
#define RC_H

#include <stddef.h>
#include <assert.h>
#include <stdlib.h>

/*
 * A simple reference counting system for arbitrary data. RC_{X,}{INC,DEC}REF
 * work similarly the macros used by the Python API.
 *
 * Example usage:
 *
 *     // Allocate a reference counted string (with initial refcount of 1), no
 *     // special destructor
 *
 *     char *string = rc_malloc(sizeof("Hello World!"), NULL, NULL);
 *     [...]
 *
 *     // Do stuff with string, calling RC_[X]INCREF for any new reference
 *     // and RC_[X]DECREF for any reference going out of scope.
 *
 * Reallocation is not supported (due to the reference count being saved in the
 * same block of memory as the data), but can be emulated by double indirection,
 * using a reference counted structure that owns a normal pointer to the
 * resizable data and providing a deconstructor that cleans up the memory (not
 * unlike C++ classes like std::vector and std::list do)
 *
 * Reference cycles may also be problematic. Try to avoid them.
 */

/*
 * If possible, use flexible array members as supported by C99, but fall back
 * to the de-facto hack for ANSI if not.
 */
#if __STDC_VERSION__ >= 199901L
    #define RC_FLEXIBLE
    #define RC_ALLOCSIZE(size) (sizeof(struct rc_object) + size)
#else
    #define RC_FLEXIBLE 1
    #define RC_ALLOCSIZE(size) (offsetof(struct rc_object, data) + size)

#endif

#define RC_GETOBJ(ptr) ((struct rc_object *) \
    ((unsigned char*)(ptr) - offsetof(struct rc_object, data) ))

#define RC_DECREF(ref) do {                         \
        assert((ref) != NULL);                      \
        struct rc_object *_hdr = RC_GETOBJ(ref);    \
                                                    \
        if (!--(_hdr->refcount)) {                  \
            void *_tmp = (ref);                     \
                                                    \
            (ref) = NULL;                           \
                                                    \
            if (_hdr->destructor != NULL) {         \
                _hdr->destructor(_hdr->data,        \
                                 _hdr->udata);      \
            }                                       \
                                                    \
            free(_hdr);                             \
        }                                           \
    } while (0)

#define RC_INCREF(ref) do {           \
        assert((ref) != NULL);        \
        ++RC_GETOBJ(ref)->refcount;   \
    } while(0)

#define RC_XINCREF(ref) do {   \
    if ((ref) != NULL) {       \
        RC_INCREF(ref)         \
    } while (0)

#define RC_XDECREF(ref) do {   \
        if ((ref) != NULL) {   \
            RC_DECREF(ref);    \
        }                      \
    } while (0)

#define RC_NUMREFS(ref) (RC_GETOBJ((ref))->refcount)

typedef void (*rc_destructor_fun)(void *data, void *udata);

struct rc_object
{
    rc_destructor_fun destructor;
    void *udata;

    unsigned refcount;
    unsigned char data[RC_FLEXIBLE];
};

void *rc_malloc(size_t size, rc_destructor_fun dtor, void *udata);

#endif /* define RC_H */

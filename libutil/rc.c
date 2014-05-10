#include "rc.h"

void *rc_malloc(size_t size, rc_destructor_fun dtor, void *udata) {
    struct rc_object *obj = malloc(RC_ALLOCSIZE(size));
    obj->refcount = 1;
    obj->destructor = dtor;
    obj->udata = udata;

    return obj->data;
}

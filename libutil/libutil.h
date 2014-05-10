#ifndef LIBUTIL_H
#define LIBUTIL_H

/*
 * If inline is supported, function prototypes annotated with INLINE_DEC and
 * function definitions annotated with INLINE_DEF can be declared as inline
 */
#if __STDC_VERSION__ >= 199901L
    #define INLINE_DEC extern
    #define INLINE_DEF inline
#else
    #define INLINE_DEC
    #define INLINE_DEF
#endif

#if __STDC_VERSION__ >= 199901L
    #include <stdbool.h>
#else
    typedef int bool;
    #define true 1
    #define false 0
#endif

#endif /* defined LIBUTIL_H */

#include "dstring.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#ifdef NONSTRICT
#   define UTF8_DECODE utf8_decode_s
#else
#   define UTF8_DECODE utf8_decode
#endif


char *dstrinit(char32_t c, size_t n)
{
    char utfbuf[8] = {0};
    int clen = utf8_encode(utfbuf, sizeof(utfbuf), c);

    char *str = malloc(sizeof(char) * ((n * clen) + 1));

    for (size_t i = 0; i < n; ++i)
        memcpy(str + (i * clen), utfbuf, clen);

    str[sizeof(char) * ((n * clen))] = '\0';

    return str;
}

char *dstrset(char *oldstr, const char *newstr)
{
    free(oldstr);
    return (char *)newstr;
}

char *dstrdup(const char *src)
{
    return dstrndup(src, strlen(src));
}

char *dstrndup(const char *src, size_t n)
{
    char *end = dstrpos(src, n);
    if (end == NULL)
        end = (char *)(src + strlen(src));

    n = end - src;
    char *str = dstrinit('\0', n);

    return strncpy(str, src, n);
}

char *dstrcat(const char *a, const char *b)
{
    return strcat(strcat(dstrinit('\0', strlen(a) + strlen(b)), a), b);
}

char *dstrcati(char *a, const char *b)
{
    size_t newlen = strlen(a) + strlen(b);

    char *newstr = realloc(a, newlen + 1);
    return strcat(newstr, b);
}

char *dsprintf(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    char *str = dvsprintf(fmt, args);
    va_end(args);

    return str;
}

char *dvsprintf(const char *fmt, va_list args)
{
    va_list args2;

    va_copy(args2, args);
    int n = vsnprintf(NULL, 0, fmt, args2);
    va_end(args2);

    char *str = dstrinit('\0', n + 1);
    vsnprintf(str, n + 1, fmt, args);

    return str;
}

char *dsprintfi(char *dest, size_t pos, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    char *str = dvsprintfi(dest, pos, fmt, args);
    va_end(args);

    return str;
}

char *dvsprintfi(char *dest, size_t pos, const char *fmt, va_list args)
{
    char *ins = dvsprintf(fmt, args);
    char *ret = dstrinsn(dest, pos, ins);
    free(ins);

    return ret;
}

char *dstrpos(const char *str, size_t n)
{
    size_t pos = 0;
    size_t rawlen = strlen(str);

    while (pos < rawlen) {
        if (!n--)
            return (char *)(str + pos);

        int un = UTF8_DECODE(str + pos, rawlen - pos, NULL);

        if (un <= 0)
            return NULL;

        pos += un;
    }

    return NULL;
}

size_t dstrdiff(const char *str, const char *ptr)
{
    size_t diff = 0;
    size_t pos = 0;
    size_t len = strlen(str);

    while ((str + pos) < ptr) {
        int un = UTF8_DECODE(str + pos, len - pos, NULL);

        if (un <= 0)
            return -1;

        pos += un;
        diff++;
    }

    return diff;
}

char *dstrins(char *str, size_t pos, char32_t codepoint)
{
    char utfbuf[8] = {0};

    if (utf8_encode(utfbuf, sizeof(utfbuf), codepoint) < 0)
        return str;

    return dstrinsn(str, pos, utfbuf);
}

char *dstrinsn(char *str, size_t pos, const char *src)
{
    if (str == NULL)
        return dstrdup(src);

    size_t oldlen = strlen(str);
    size_t srclen = strlen(src);

    char *inspos = dstrpos(str, pos);
    if (inspos == NULL)
        inspos = str + oldlen;

    size_t realpos = inspos - str;

    size_t newlen = oldlen + srclen;
    char *newstr = realloc(str, newlen + 1);

    /* shift contents behind insert position back */
    memmove(newstr + realpos + srclen, newstr + realpos, oldlen - realpos + 1);
    memmove(newstr + realpos, src, srclen);

    return newstr;
}

char *dstrdel(char *str, size_t pos)
{
    return dstrdeln(str, pos, 1);
}

char *dstrdeln(char *str, size_t pos, size_t n)
{
    char *sptr = dstrpos(str, pos);
    if (sptr == NULL)
        return str;

    char *eptr = dstrpos(sptr, n);
    if (eptr == NULL)
        eptr = sptr + strlen(sptr);

    memmove(sptr, eptr, strlen(eptr) + 1);
    return realloc(str, strlen(str) + 1);
}

char *dstrsub(const char *str, size_t pos, size_t n)
{
    char *start = dstrpos(str, pos);
    if (start == NULL)
        return NULL;

    return dstrndup(start, n);
}

char *dstrchr(const char *str, char32_t code)
{
    char utfstr[8] = {0};

    utf8_encode(utfstr, sizeof(utfstr), code);
    return strstr(str, utfstr);
}

char32_t dstridx(const char *str, size_t pos)
{
    char *ptr = dstrpos(str, pos);
    if (ptr == NULL)
        return -1;

    char32_t val = 0;
    if (UTF8_DECODE(ptr, strlen(ptr), &val) <= 0)
        return -1;

    return val;
}

long dstrlen(const char *str)
{
#ifdef NONSTRICT
    return utf8_strlen_s(str);
#else
    return utf8_strlen(str);
#endif
}

bool dstrvalid(const char *str)
{
    return utf8_strlen(str) >= 0;
}

char **dstrsplitc(const char *str, char32_t sep)
{
    char utfbuf[8] = {0};

    utf8_encode(utfbuf, sizeof(utfbuf), sep);

    return dstrsplit(str, utfbuf);
}

char **dstrsplit(const char *str, const char *sep)
{
    char **result = NULL;
    size_t nres = 0;

    size_t seplen = dstrlen(sep);

    size_t spos = 0;
    size_t epos = 0;

    char *ptr;
    while ((ptr = strstr(dstrpos(str, spos), sep)) != NULL) {
        epos = dstrdiff(str, ptr);

        /* Skip empty results */
        if (epos != spos) {
            result = realloc(result, sizeof(char*) * (nres + 1));
            result[nres++] = dstrsub(str, spos, epos - spos);
        }

        spos = epos + seplen;
    }

    result = realloc(result, sizeof(char*) * (nres + 2));
    result[nres] = dstrsub(str, spos, -1);
    result[nres + 1] = NULL;

    return result;
}

void dstrlstfree(char **strlst)
{
    for (char **ptr = strlst; *ptr != NULL; ptr++)
        free(*ptr);

    free(strlst);
}

#ifdef NONSTRICT
#   undef utf8_decode
#endif

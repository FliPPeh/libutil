#include "dstring.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#ifdef NONSTRICT
    #define UTF8_DECODE utf8_decode_s
#else
    #define UTF8_DECODE utf8_decode
#endif


char *dstrinit(char32_t c, size_t n)
{
    size_t i;
    char utfbuf[8] = {0};
    int clen = utf8_encode(utfbuf, sizeof(utfbuf), c);
    char *str = malloc(sizeof(char) * ((n * clen) + 1));

    for (i = 0; i < n; ++i)
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
    char *str;

    char *end = dstrpos(src, n);
    if (end == NULL)
        end = (char *)(src + strlen(src));

    n = end - src;
    str = dstrinit('\0', n);

    return strncpy(str, src, n);
}

char *dstrcat(const char *a, const char *b)
{
    if (a == NULL)
        return dstrdup(b);

    return strcat(strcat(dstrinit('\0', strlen(a) + strlen(b)), a), b);
}

char *dstrcati(char *a, const char *b)
{
    size_t newlen;
    char *newstr;

    if (a == NULL)
        return dstrdup(b);

    newlen = strlen(a) + strlen(b);
    newstr = realloc(a, newlen + 1);

    return strcat(newstr, b);
}

#if __STDC_VERSION__ >= 199901L
/* snprintf is not available in ANSI C90, neither is va_copy */

char *dsprintf(const char *fmt, ...)
{
    va_list args;
    char *str;

    va_start(args, fmt);
    str = dvsprintf(fmt, args);
    va_end(args);

    return str;
}

char *dvsprintf(const char *fmt, va_list args)
{
    va_list args2;
    int n;
    char *str;

    va_copy(args2, args);
    n = vsnprintf(NULL, 0, fmt, args2);
    va_end(args2);

    str = dstrinit('\0', n + 1);
    vsnprintf(str, n + 1, fmt, args);

    return str;
}

char *dsprintfi(char *dest, size_t pos, const char *fmt, ...)
{
    va_list args;
    char *str;

    va_start(args, fmt);
    str = dvsprintfi(dest, pos, fmt, args);
    va_end(args);

    return str;
}

char *dvsprintfi(char *dest, size_t pos, const char *fmt, va_list args)
{
    char *ins = dvsprintf(fmt, args);
    char *ret;

    if (dest == NULL)
        return ins;

    ret = dstrinsn(dest, pos, ins);
    free(ins);

    return ret;
}
#endif

char *dstrpos(const char *str, size_t n)
{
    size_t pos = 0;
    size_t rawlen = strlen(str);

    while (pos < rawlen) {
        int un;

        if (!n--)
            return (char *)(str + pos);

        un = UTF8_DECODE(str + pos, rawlen - pos, NULL);

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
    size_t oldlen;
    size_t srclen;
    char *inspos;

    size_t realpos;
    size_t newlen;
    char *newstr;

    if (str == NULL)
        return dstrdup(src);

    oldlen = strlen(str);
    srclen = strlen(src);

    inspos = dstrpos(str, pos);

    if (inspos == NULL)
        inspos = str + oldlen;

    realpos = inspos - str;

    newlen = oldlen + srclen;
    newstr = realloc(str, newlen + 1);

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
    char *sptr;
    char *eptr;

    sptr = dstrpos(str, pos);
    if (sptr == NULL)
        return str;

    eptr = dstrpos(sptr, n);
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
    char32_t val;
    char *ptr = dstrpos(str, pos);

    if (ptr == NULL)
        return -1;

    val = 0;
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

int dstrvalid(const char *str)
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

char **dstrshlex(const char *str, int *argc)
{
    enum lex_state { NONE, WORD, STRING } state = NONE;

    int i;

    struct lex_token {
        enum lex_state type;
        const char *start;
        const char *end;
    } tokens[256] = {{0, 0, 0}};

    struct lex_token *tok = &tokens[0];

    const char *ptr;
    char **ret = NULL;

    *argc = 0;

    for (ptr = str; *ptr != '\0'; ++ptr) {
        if (state == NONE) {
            if (isspace(*ptr))
                continue;

            if (*ptr == '"') {
                tok->type = state = STRING;
                tok->start = ptr + 1;
            } else {
                tok->type = state = WORD;
                tok->start = ptr;
            }

            continue;
        } else {
            if (*ptr == '\\') {
                if (*(ptr + 1) != '\0')
                    ++ptr;

                continue;
            }

            if (state == STRING) {
                if (*ptr != '"')
                    continue;
            } else if (state == WORD) {
                if (!isspace(*ptr))
                    continue;
            }

            tok->end = ptr;
            state = NONE;
            tok = &tokens[++*argc];
        }
    }

    /* Finish the last token */
    if (state != NONE) {
        tok->end = ptr;
        ++*argc;
    }

    ret = malloc(sizeof(char *) * (*argc + 1));

    for (i = 0; i < *argc; ++i) {
        size_t size = (tokens[i].end - tokens[i].start) + 1;

        char *str = malloc(size);

        const char *ptr;  /* read pointer */
        char *optr = str; /* write pointer */

        memset(str, 0, size);

        for (ptr = tokens[i].start; ptr < tokens[i].end; ++ptr) {
            if (*ptr == '\\') {
                ++ptr;

                switch (*ptr) {
                    case ' ': *optr++ = ' ';  break;
                    case '"': *optr++ = '"';  break;
                    case 't': *optr++ = '\t'; break;
                }
            } else {
                if ((tokens[i].type == WORD) && (*ptr == '"'))
                    continue;

                *optr++ = *ptr;
            }
        }

        ret[i] = str;
    }

    ret[*argc] = NULL;
    return ret;
}

void dstrlstfree(char **strlst)
{
    char **ptr;

    for (ptr = strlst; *ptr != NULL; ptr++)
        free(*ptr);

    free(strlst);
}

#ifdef NONSTRICT
#   undef utf8_decode
#endif

#ifndef DSTRING_H
#define DSTRING_H

#include "utf8.h"

#include <stdlib.h>
#include <stdio.h>

/*
 * Oldschool strings and helper functions for working with them dynamically and
 * with UTF-8 support.
 *
 * Functions of which their C standard variants modify their first argument
 * return a new string instead, leaving the old intact, unless an 'i' (inplace)
 * version for those exists, in which case they will reallocate the string and
 * return the new pointer. In general, if the first string argument is *not*
 * const, it will reallocate that string and return it.
 *
 * All the functions that deal with UTF-8 will throw error values *unless*
 * NONSTRICT is defined prior to including this header. If NONSTRICT is defined,
 * all occurences of utf8_decode will be replaced with utf8_decode_s
 * which will handle invalid UTF-8 more gracefully (by replacing
 * bad UTF-8 sequences with U+FFFD (replacement character) and skipping bad
 * bytes). dstrvalid() however will still return the correct value.
 */

/*
 * Creates a new string out of the unicode codepoint c repeated n times.
 */
char *dstrinit(char32_t c, size_t n);

/* Simple helper that frees the old string and returns src */
char *dstrset(char *oldstr, const char *newstr);

char *dstrdup(const char *src);
char *dstrndup(const char *src, size_t n);

char *dstrcat(const char *dest, const char *src);
char *dstrcati(char *dest, const char *src);

char *dsprintf(const char *fmt, ...);
char *dvsprintf(const char *fmt, va_list args);

char *dsprintfi(char *dest, size_t pos, const char *fmt, ...);
char *dvsprintfi(char *dest, size_t pos, const char *fmt, va_list args);

/*
 * Returns a pointer to the nth codepoint (not byte). Return NULL if str
 * is not UTF-8 valid or n is outside the boundaries. Used mostly internally
 * but can also be used as a UTF-8 aware offset function.
 */
char *dstrpos(const char *str, size_t n);

/*
 * The other way around. Given a substring ptr within string str, figure out
 * how many codepoints lie between those. The UTF-8 counterpart to (ptr - str)
 * for ASCII strings. Bad things will happen if ptr does not point inside str
 * or str. Returns the difference in codepoints between str and ptr on success
 * or (size_t)-1 on failure (not UTF-8 valid).
 */
size_t dstrdiff(const char *str, const char *ptr);

/*
 * Inserts a Unicode codepoint into the string str at position pos (in
 * codepoints, not bytes), encoding it to UTF-8 in the process.
 * If for some reason encoding fails, returns NULL and leaves
 * str unmodified. If pos is out of bounds, assumes end of string. If str is
 * NULL, creates a new string.
 */
char *dstrins(char *str, size_t pos, char32_t codepoint);
char *dstrinsn(char *str, size_t pos, const char *src);

/*
 * Removes the Unicode codepoint at position pos. If str is not UTF-8 valid,
 * returns NULL and leaves str unmodified. If pos is outside of bounds, returns
 * str unmodified.
 */
char *dstrdel(char *str, size_t pos);
char *dstrdeln(char *str, size_t pos, size_t n);

/*
 * Substring from [pos .. pos + n). If pos + n is would run off the end of the
 * string, n will be limited automatically. If pos is not within bounds, return
 * NULL.
 */
char *dstrsub(const char *str, size_t pos, size_t n);

/*
 * UTF-8 aware strchr(). If code is not found in str or str is not UTF-8 valid,
 * returns NULL.
 */
char *dstrchr(const char *str, char32_t code);

/*
 * Returns the unicode codepoint at position pos. If pos is out of bounds, or
 * str is not UTF-8 valid, returns a negative value. Pretty much str[pos] for
 * UTF-8 strings instead of ASCII.
 */
char32_t dstridx(const char *str, size_t pos);

/*
 * UTF-8 aware string length, returns -1 if str is not UTF-8 valid.
 */
long dstrlen(const char *str);

/* returns whether or not str is valid UTF-8 */
bool dstrvalid(const char *str);

/* Splits the given string on a given separator (not including it in the
 * results). Returns a NULL terminated array of strings which have to be freed
 * individually, including the array itself. */
char **dstrsplitc(const char *str, char32_t sep);
char **dstrsplit(const char *str, const char *sep);

void dstrlstfree(char **strlst);

#endif /* define DSTRING_H */

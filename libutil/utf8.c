#include "utf8.h"

#include <stdlib.h>
#include <string.h>

static unsigned _utf8_shiftpos(unsigned w, unsigned n);
static int _utf8_is_continuation(unsigned char c);

static unsigned _utf8_encoding_width(char32_t codepoint);

static char _utf8_header_byte(char32_t codepoint, unsigned w);
static char _utf8_continuation_byte(char32_t codepoint, unsigned w, unsigned n);

static unsigned _utf8_decoding_width(unsigned char header);

static char32_t _utf8_header_value(unsigned char header, unsigned w);
static char32_t _utf8_continuation_value(unsigned char cont,
                                         unsigned w,
                                         unsigned n);


int utf8_encode(char *buf, size_t bufsiz, char32_t codepoint)
{
    unsigned w = _utf8_encoding_width(codepoint);

    if ((buf == NULL) && (bufsiz == 0))
        return w;
    else if (bufsiz < w)
        return -1;

    if (w > 1) {
        size_t i;

        buf[0] = _utf8_header_byte(codepoint, w);

        for (i = 1; i < w; ++i)
            buf[i] = _utf8_continuation_byte(codepoint, w, i);
    } else {
        buf[0] = codepoint & 0x7f;
    }

    return w;
}

int utf8_decode(const char *buf, size_t bufsiz, char32_t *codepoint)
{
    unsigned w;

    /* Can't do anything without at least reading one byte. */
    if (bufsiz < 1)
        return -1;

    w = _utf8_decoding_width(buf[0]);

    if (!w || (w > 6))
        return 0;
    else if ((bufsiz < w) && (codepoint != NULL))
        return -1;
    else if (codepoint == NULL)
        return w;

    if (w > 1) {
        size_t i;

        *codepoint = _utf8_header_value(buf[0], w);

        for (i = 1; i < w; ++i) {
            /* We need a continuation byte, it's an error if there isn't. */
            if (!_utf8_is_continuation(buf[i]))
                return 0;

            *codepoint |= _utf8_continuation_value(buf[i], w, i);
        }
    } else {
        *codepoint = buf[0] & 0x7f;
    }

    return w;
}

int utf8_decode_s(const char *buf, size_t bufsiz, char32_t *codepoint)
{
    size_t res = utf8_decode(buf, bufsiz, codepoint);
    if (!res) {
        /*
         * utf8_decode is not happy, but instead of returning the error code
         * to the client, we swap in U+FFFD as replacement and manually skip
         * bad characters without validation.
         */
        size_t n = 1;

        if (codepoint)
            *codepoint = 0xFFFD;

        for (; (n < bufsiz) && (((unsigned char *)buf)[n] > 0x7f); ++n)
            ;;

        return n;
    }

    return res;
}

long utf8_strlen(const char *str)
{
    size_t len;
    size_t pos;
    size_t rawlen = strlen(str);

    for (len = 0, pos = 0; pos < rawlen; ++len) {
        int n = utf8_decode(str + pos, rawlen - pos, NULL);

        if (n <= 0)
            return -1;

        pos += n;
    }

    return len;
}

long utf8_strlen_s(const char *str)
{
    size_t len;
    size_t pos;
    size_t rawlen = strlen(str);

    for (len = 0, pos = 0; pos < rawlen; ++len) {
        int n = utf8_decode_s(str + pos, rawlen - pos, NULL);

        if (n < 0)
            return -1;

        pos += n;
    }

    return len;
}

static unsigned _utf8_shiftpos(unsigned w, unsigned n)
{
    return 6 * (w - 1 - n);
}

static int _utf8_is_continuation(unsigned char c)
{
    return (c & 0xc0) == 0x80;
}

static unsigned _utf8_encoding_width(char32_t codepoint)
{
    size_t steps[] = { 0x7f, 0x07ff, 0xffff, 0x1fffff, 0x3ffffff, 0x7fffffff };
    unsigned n = 0;

    if (codepoint > 0x7fffffff)
        return 0;

    for (n = 1; codepoint > steps[n - 1]; ++n)
        ;; /* my brain needs this to identify it as a loop, don't judge. */

    return n;
}

static char _utf8_header_byte(char32_t codepoint, unsigned w)
{
    unsigned char pattern = ~0 << (8 - w);
    unsigned char valmask = ~0 >> (7 - w);

    return pattern | (char)(codepoint >> _utf8_shiftpos(w, 0) & valmask);
}

static char _utf8_continuation_byte(char32_t codepoint, unsigned w, unsigned n)
{
    return 0x80 | (char)(codepoint >> _utf8_shiftpos(w, n) & 0x3f);
}

static unsigned _utf8_decoding_width(unsigned char header)
{
    unsigned n = 0;

    /* Test for invalid continuation byte */
    if ((header & 0xc0) == 0x80)
        return 0;
    /* And for ASCII values */
    else if (!(header & 0x80))
        return 1;

    /* Count the most significant bits until they're set */
    while (((header << n) & 0x80) > 0)
        ++n;

    return n;
}

static char32_t _utf8_header_value(unsigned char header, unsigned w)
{
    return (header & ((unsigned char)~0 >> (7 - w))) << _utf8_shiftpos(w, 0);
}

static char32_t _utf8_continuation_value(unsigned char cont,
                                         unsigned w,
                                         unsigned n)
{
    return (cont & 0x3f) << _utf8_shiftpos(w, n);
}

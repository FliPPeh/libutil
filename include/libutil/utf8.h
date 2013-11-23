#ifndef UTF8_H
#define UTF8_H

#include <stdlib.h>

#if __STDC_VERSION >= 201112L
#   include <uchar.h> /* char32_t */
#else
#   define char32_t unsigned long
#endif

/*
 * Encodes the given codepoint into the buffer buf.
 *
 * If the encoding requires more bytes than available, returns a negative value.
 *
 * If buf is NULL and bufsiz is zero, simply returns the number of bytes needed
 * to encode the codepoint but doesn't attempt to write any of them.
 *
 * A buffer of 6 bytes will be enough to encode any codepoint.
 */
int utf8_encode(char *buf, size_t bufsiz, char32_t codepoint);

/*
 * Attempts to decode a unicode codepoint from the buffer.
 *
 * If codepoint is NULL, only the first byte will be read and the total number
 * of bytes the would need to be read will be returned.
 *
 * If no valid UTF-8 header is found, zero is returned.
 *
 * If the buffer is not large enough to contain the full encoding, a negative
 * value is returned (unless codepoint is NULL).
 *
 * In any case, the first byte will always have to be read, so a negative value
 * is returned if the buffer doesn't have space for even a single character.
 *
 * A buffer of 6 bytes is able to contain any codepoint.
 */
int utf8_decode(const char *buf, size_t bufsiz, char32_t *codepoint);

/*
 * A somewhat simpler version of utf8_decode. Instead of leaving codepoint
 * undefined on failure, places U+FFFD (replacement character) into codepoint
 * if utf8_decode returns with an invalid UTF-8 sequence, manually skips
 * possibly following continuation bytes and returns the number of bytes
 * skipped.
 */
int utf8_decode_s(const char *buf, size_t bufsiz, char32_t *codepoint);

/*
 * Tries to determine the number of codepoints inside the zero terminated
 * string. If str is not a valid UTF-8 string a negative value is returned.
 */
long utf8_strlen(const char *str);

/*
 * Equivalent to utf8_strlen, but it ignores invalid UTF-8 sequences by
 * assuming the replacement character.
 */
long utf8_strlen_s(const char *str);

/*
 * Helpers
 *
 * Most of these could be done with macros but let's avoid them where it's
 * not necessary.
 */
unsigned _utf8_shiftpos(unsigned w, unsigned n);
int _utf8_is_continuation(unsigned char c);

/*
 * Encoding
 */
unsigned _utf8_encoding_width(char32_t codepoint);

char _utf8_header_byte(char32_t codepoint, unsigned w);
char _utf8_continuation_byte(char32_t codepoint, unsigned w, unsigned n);

/*
 * Decoding
 */
unsigned _utf8_decoding_width(unsigned char header);

/* Returned values are already shifted to their respective places */
char32_t _utf8_header_value(unsigned char header, unsigned w);
char32_t _utf8_continuation_value(unsigned char cont, unsigned w, unsigned n);


#endif /* defined UTF8_H */

#ifndef JSON_H
#define JSON_H

#define _XOPEN_SOURCE 700

#include "container/list.h"
#include "container/hashtable.h"

#include <stdio.h>
#include <stdlib.h>
#include <uchar.h>
#include <stdbool.h>

enum json_value_type
{
    JSON_STRING,
    JSON_NUMBER,
    JSON_NULL,
    JSON_BOOLEAN,
    JSON_ARRAY,
    JSON_OBJECT
};

struct json_value
{
    enum json_value_type type;

    union
    {
        char *jstring;
        double jnumber;
        bool jbool;

        struct list *jarray;
        struct hashtable *jobject;
    };
};

enum json_token_type
{
    // Object tokens
    TOK_BRACE_OPEN,
    TOK_BRACE_CLOSE,
    TOK_COLON,

    // Array tokens
    TOK_SQUARE_BRACKET_OPEN,
    TOK_SQUARE_BRACKET_CLOSE,

    // Array and object token
    TOK_COMMA,

    // Simple types
    TOK_STRING,
    TOK_NUMBER,
    TOK_TRUE,
    TOK_FALSE,
    TOK_NULL,

    TOK_MAX
};

// Maps token types to their string representation
extern const char *json_token_str[];

struct json_token
{
    size_t i; // token start position within input
    size_t j; // token end position within input

    enum json_token_type type;
};

struct json_lexer_state
{
    size_t pos; // position within the input range
    const char *input;
};

struct json_value *json_value_new(enum json_value_type type);
struct json_value *json_string_new(const char *str);
struct json_value *json_string_new_n(const char *str, size_t n);
struct json_value *json_number_new(double dbl);
struct json_value *json_bool_new(bool b);
struct json_value *json_null_new();
struct json_value *json_array_new();
struct json_value *json_object_new();

/*
 * These either return a pointer to the expected value, or NULL on type error.
 * Functions that don't return a const pointer can be used to set the contained
 * value (by design).
 */
bool json_is_null(struct json_value *val);

const char *json_get_string(struct json_value *val);
void        json_set_string(struct json_value *val, const char *newstring);

double *json_get_number(struct json_value *val);
bool   *json_get_bool  (struct json_value *val);

/*
 * Returns true or false in a more weakly typed manner. Empty strings, false,
 * null, zero, an empty array, an empty hashtable return false. Everything else
 * returns true.
 */
bool json_get_logical_bool(struct json_value *val);

/*
 * Since json is bundled in this library of hashtables and lists, rewriting
 * functionality like json_object_insert() and json_object_remove() would be
 * redundant. Instead, we simply return the underlying data structures for use
 * with their native functions
 */
struct list      *json_get_array (struct json_value *val);
struct hashtable *json_get_object(struct json_value *val);

/*
 * Although little helpers never harmed anybody.
 */
#define JSON_OBJECT_LOOKUP(obj, key) \
    (struct json_value *)hashtable_lookup(json_get_object((obj)), (key))

/*
 * Or bigger helpers. This one will segfault on a type mismatch (unless it
 * resolves to a string, in which case it will still return NULL, or a bool is
 * requested), so watch out! Use it only when you know the types are correct
 * (by verifying the JSON document beforehand for example)
 */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#   define JSON_GET(val, type) ((type)(_Generic((type)0,    \
        unsigned:    (*json_get_number(val)),               \
        int:         (*json_get_number(val)),               \
        double:      (*json_get_number(val)),               \
        float:       (*json_get_number(val)),               \
        bool:        (json_get_logical_bool(val)),          \
        const char*: (json_get_string(val)))))
#endif

void json_free_wrapper_hash(void *val);
void json_free_wrapper(void *val, void *ud);

void json_free(struct json_value *v);
void json_free_contents(struct json_value *v);

struct json_value *json_parse(const char *input);
struct json_value *json_parse_value(struct json_lexer_state *lex);
char *json_parse_string(struct json_lexer_state *lex,
                        struct json_token *tok);

size_t json_dump(char *out, size_t nout, struct json_value *val);
size_t json_dump_string(char *out, size_t nout, const char *str);

/*
 * Attempt to fetch the next token from state into tok. Returns 0 on success and
 * 1 on error. If 1 is returned, the position where to error occured is stored
 * in tok->i.
 */
int json_lexer_next_token(struct json_lexer_state *state,
                          struct json_token *tok);


#endif /* defined JSON_H */

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

void json_free_wrapper_hash(void *val);
void json_free_wrapper(void *val, void *ud);
void json_free(struct json_value *v);

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

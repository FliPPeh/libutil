#include "json.h"
#include "utf8.h"

#include <string.h>
#include <assert.h>
#include <ctype.h>


const char *json_token_str[] = {
    "{", "}", ":", "[", "]", ",", "string", "number", "true", "false", "null"
};

struct json_value *json_value_new(enum json_value_type type)
{
    struct json_value *val = malloc(sizeof(*val));
    memset(val, 0, sizeof(*val));

    val->type = type;
    return val;
}

struct json_value *json_string_new(const char *str)
{
    return json_string_new_n(str, strlen(str));
}

struct json_value *json_string_new_n(const char *str, size_t n)
{
    struct json_value *v = json_value_new(JSON_STRING);
    v->jstring = strndup(str, n);

    return v;
}

struct json_value *json_number_new(double dbl)
{
    struct json_value *v = json_value_new(JSON_NUMBER);
    v->jnumber = dbl;

    return v;
}

struct json_value *json_bool_new(bool b)
{
    struct json_value *v = json_value_new(JSON_BOOLEAN);
    v->jbool = b;

    return v;
}

struct json_value *json_null_new()
{
    return json_value_new(JSON_NULL);
}

struct json_value *json_array_new()
{
    return json_value_new(JSON_ARRAY);
}

struct json_value *json_object_new()
{
    struct json_value *v = json_value_new(JSON_OBJECT);

    v->jobject = hashtable_new_with_free(
        str_hash, str_equal, free, (void (*)(void*))json_free_wrapper_hash);
    return v;
}

void json_free_wrapper_hash(void *val)
{
    json_free(val);
}

void json_free_wrapper(void *val, void *ud)
{
    (void)ud;

    json_free(val);
}

void json_free(struct json_value *v)
{
    switch (v->type) {
    case JSON_STRING:
        free(v->jstring);
        break;

    case JSON_ARRAY:
        list_free_all(v->jarray, json_free_wrapper, NULL);
        break;

    case JSON_OBJECT:
        hashtable_free(v->jobject);
        break;

    default:
        break;
    }

    free(v);
}

struct json_value *json_parse(const char *input)
{
    struct json_lexer_state state = { .pos = 0, .input = input };
    return json_parse_value(&state);
}

struct json_value *json_parse_value(struct json_lexer_state *lex)
{
    // Read first token to determine how to proceed
    struct json_token tok;
    struct json_token next;

    if (!json_lexer_next_token(lex, &tok)) {
        switch (tok.type) {
        case TOK_BRACE_OPEN:
            ;;
            /* Read string:value pairs until TOK_BRACE_CLOSE */
            struct json_value *obj = json_object_new();
            char *key = NULL;

            while (!json_lexer_next_token(lex, &next)) {
                if (next.type == TOK_STRING) {
                    key = json_parse_string(lex, &next);
                    if (!key)
                        goto exit_err_obj;

                    if ((json_lexer_next_token(lex, &next) != 0)
                            || (next.type != TOK_COLON)) {
                        free(key);
                        goto exit_err_obj;
                    }

                    struct json_value *kval = json_parse_value(lex);
                    if (!kval) {
                        free(key);
                        goto exit_err_obj;
                    }

                    hashtable_insert(obj->jobject, key, kval);

                    if (json_lexer_next_token(lex, &next) != 0)
                        goto exit_err_obj;

                    if (next.type == TOK_COMMA) {
                        continue;
                    } else if (next.type == TOK_BRACE_CLOSE) {
                        return obj;
                    } else {
                        //fprintf(stderr, "expected `%s' or `%s', got `%s'\n",
                        //    json_token_str[TOK_COMMA],
                        //    json_token_str[TOK_BRACE_CLOSE],
                        //    json_token_str[next.type]);

                        goto exit_err_obj;
                    }
                } else if (next.type == TOK_BRACE_CLOSE) {
                    // In case it's an empty object
                    return obj;
                } else {
                    //fprintf(stderr, "expected `%s' or `%s', got `%s'\n",
                    //    json_token_str[TOK_COLON],
                    //    json_token_str[TOK_BRACE_CLOSE],
                    //    json_token_str[next.type]);

                    goto exit_err_obj;
                }
            }

            break;

exit_err_obj:
            json_free(obj);
            return NULL;

        case TOK_SQUARE_BRACKET_OPEN:
            ;;
            /* Read values until TOK_SQUARE_BRACKET_CLOSE */
            struct json_value *arr = json_array_new();

            // Can't expect with arrays the way we can expect with objects,
            // so we have to peek
            size_t oldpos = lex->pos;

            while (!json_lexer_next_token(lex, &next)) {
                if (next.type == TOK_SQUARE_BRACKET_CLOSE) {
                    // In case it's an empty array
                    return arr;
                } else {
                    // Jump back so json_parse_value can correctly pars
                    lex->pos = oldpos;

                    struct json_value *val = json_parse_value(lex);
                    if (!val)
                        goto exit_err_arr;

                    arr->jarray = list_append(arr->jarray, val);

                    if (json_lexer_next_token(lex, &next) != 0)
                        goto exit_err_arr;

                    if (next.type == TOK_COMMA) {
                        oldpos = lex->pos;

                        continue;
                    } else if (next.type == TOK_SQUARE_BRACKET_CLOSE) {
                        return arr;
                    } else {
                        // fprintf(stderr, "expected `%s' or `%s', got `%s'\n",
                        //    json_token_str[TOK_COMMA],
                        //    json_token_str[TOK_SQUARE_BRACKET_CLOSE],
                        //    json_token_str[next.type]);

                        goto exit_err_arr;
                    }
                }
            }

            break;

exit_err_arr:
            json_free(arr);
            return NULL;

        case TOK_STRING:
            ;;
            struct json_value *str = json_value_new(JSON_STRING);
            str->jstring = json_parse_string(lex, &tok);

            return str->jstring != NULL ? str : NULL;

        case TOK_NUMBER:
            /* sscanf() number and return */
            ;;
            double n;

            if ((sscanf(lex->input + tok.i, "%lf", &n)) != 1) {
                //fprintf(stderr, "Bad number format!\n");
                return NULL;
            }

            return json_number_new(n);

        case TOK_TRUE:
            return json_bool_new(true);

        case TOK_FALSE:
            return json_bool_new(false);

        case TOK_NULL:
            return json_null_new();

        default:
            //fprintf(stderr, "Unexpected token `%s'!\n",
            //    json_token_str[tok.type]);

            return NULL;
        }
    } else {
        //fprintf(stderr, "prematurely reached EOF\n");
    }

    return NULL;
}

char *json_parse_string(struct json_lexer_state *lex,
                        struct json_token *tok)
{
    // TODO: calculate length after reducing escapes, but for now, the
    //       unescaped string can not be longer than the escaped string
    size_t len = tok->j - tok->i;

    char *str = malloc(sizeof(char) * len);
    memset(str, 0, sizeof(char) * len);

    // From i+1 to j-1 to skip the first and last quotes
    for (size_t i = tok->i + 1, j = 0; i < tok->j - 1; ++i) {
        if (lex->input[i] != '\\') {
            str[j++] = lex->input[i];

            continue;
        }

        char esc = lex->input[++i];

        switch (esc) {
        case '\"': str[j++] = '\"'; break;
        case '\\': str[j++] = '\\'; break;
        case '/':  str[j++] = '/';  break;
        case 'b':  str[j++] = '\b'; break;
        case 'f':  str[j++] = '\f'; break;
        case 'n':  str[j++] = '\n'; break;
        case 'r':  str[j++] = '\r'; break;
        case 't':  str[j++] = '\t'; break;
        case 'u': {
            i++; // Skip the 'u'

            // Make sure there's at least 4 characters left
            if ((long)(len - (i + 4)) < 0) {
                //fprintf(stderr, "incomplete unicode escape `%.*s'\n",
                //    (int)(len - (i + 4)), lex->input + i);

                goto exit_err;
            }

            // Because you can't tell scanf to scan EXACTLY 4 characters.
            char cp0, cp1, cp2, cp3;
            if (sscanf(lex->input + i, "%1hhx%1hhx%1hhx%1hhx",
                       &cp0, &cp1, &cp2, &cp3) != 4) {
                //fprintf(stderr, "invalid unicode escape `%.4s'\n",
                //            lex->input + i);

                goto exit_err;
            }

            unsigned short codepoint = cp0 << 12 | cp1 << 8 | cp2 << 4 | cp3;

            j += utf8_encode(str + j, len - j, codepoint);
            i += 2; // Skip 2 digits, let the next be skipped below and the
                    // one after that by the for-loop
            break;
        }

        default:
            // Ignore unknown escape code
            continue;
        }

        ++i;
    }

    return str;

exit_err:
    free(str);
    return NULL;
}


/*
 * Basically disgusting, aka a way to not spam inline ifs everywhere to prevent
 * buffer overflows by short circuiting both the offsetting and length
 * adjustment if the buffer has run out, so you don't explicitely have to
 * check after every. single. snprintf. and. json_dump_*. If the buffer runs
 * out, replaces the buffer offsetting by NULL and the length adjustment by 0
 * and keep going. The snprintfs will produce the correct return values but
 * write nothing, and the total result is still correct.
 */
#define BUFORNULL ((pos > nout) ? NULL : (out + pos)), \
                   (pos > nout) ? 0    : (nout - pos)

size_t json_dump(char *out, size_t nout, struct json_value *val)
{
    size_t pos = 0;

    switch (val->type) {
    case JSON_STRING:
        return json_dump_string(out, nout, val->jstring);

    case JSON_NUMBER:
        return snprintf(out, nout, "%.2f", val->jnumber);

    case JSON_NULL:
        return snprintf(out, nout, "null");

    case JSON_BOOLEAN:
        return snprintf(out, nout, val->jbool ? "true" : "false");

    case JSON_ARRAY:
        pos += snprintf(out, nout, "[");

        for (struct list *ptr = val->jarray; ptr != NULL; ptr = ptr->next) {
            pos += json_dump(BUFORNULL, list_data(ptr, struct json_value *));

            if (ptr->next != NULL)
                pos += snprintf(BUFORNULL, ", ");
        }

        pos += snprintf(BUFORNULL, "]");
        return pos;

    case JSON_OBJECT:
        pos += snprintf(out, nout, "{");

        struct hashtable_iterator iter;
        size_t i = 0;

        void *key = NULL;
        void *value = NULL;

        hashtable_iterator_init(&iter, val->jobject);
        while (hashtable_iterator_next(&iter, &key, &value)) {
            pos += json_dump_string(BUFORNULL, key);
            pos += snprintf(BUFORNULL, ": ");
            pos += json_dump(BUFORNULL, value);

            if (++i < hashtable_size(val->jobject))
                pos += snprintf(BUFORNULL, ", ");
        }

        pos += snprintf(BUFORNULL, "}");
        return pos;
    }
}

size_t json_dump_string(char *out, size_t nout, const char *str)
{
    size_t pos = 0;

    pos += snprintf(BUFORNULL, "\"");

    for (const char *ptr = str; *ptr != 0; ++ptr) {
        switch (*ptr) {
            case '\"': pos += snprintf(BUFORNULL, "\\\"");     break;
            case '\\': pos += snprintf(BUFORNULL, "\\\\");     break;
            case '\b': pos += snprintf(BUFORNULL, "\\b");      break;
            case '\f': pos += snprintf(BUFORNULL, "\\f");      break;
            case '\n': pos += snprintf(BUFORNULL, "\\n");      break;
            case '\r': pos += snprintf(BUFORNULL, "\\r");      break;
            case '\t': pos += snprintf(BUFORNULL, "\\t");      break;
            default:   pos += snprintf(BUFORNULL, "%c", *ptr); break;
        }
    }

    pos += snprintf(BUFORNULL, "\"");
    return pos;
}

#undef BUFORNULL

int json_lexer_next_token(struct json_lexer_state *state,
                          struct json_token *tok)
{
    for (;;) {
        char c = state->input[state->pos];

        if (!c) {
            tok->i = state->pos;
            return 1;
        }

        if (strchr("{}[]:,", c) != NULL) {
            // Simple symbol, look it up
            for (size_t i = 0; i < TOK_MAX; ++i) {
                if (c == *json_token_str[i]) {
                    tok->type = (enum json_token_type)i;
                    tok->i = state->pos;
                    tok->j = state->pos + 1;

                    state->pos++;
                    return 0;
                }
            }
        } else if (isspace(c) || iscntrl(c)) {
            state->pos++;
        } else if (c == '"') {
            tok->type = TOK_STRING;
            tok->i = state->pos++;

            // TODO: Escapes
            while (state->input[state->pos] != '"') {
                if (state->input[state->pos] == '\\')
                    state->pos++; // skip backspace and let next line skip
                                  // escape code, we'll let the parser take
                                  // care of validating the string.
                state->pos++;
            }

            tok->j = ++state->pos;
            return 0;
        } else if (isdigit(c) || (c == '+') || (c == '-')) {
            tok->type = TOK_NUMBER;
            tok->i = state->pos;

            // For the sake of simplicity let's just ignore what order the
            // individual parts are in and let the parser worry about that.
            for (;;) {
                char c = state->input[state->pos++];

                if (isdigit(c) || (strchr("+-.eE", c) != NULL))
                    continue;
                else
                    break;
            }

            // Back pos back up a character so we can look at that in the next
            // round.
            tok->j = --state->pos;
            return 0;
        } else if (isalpha(c)) {
            // true, false or null, scan until non-alpha
            tok->i = state->pos;

            while (isalpha(state->input[state->pos]))
                state->pos++;

            size_t len = state->pos - tok->i;

            if (!strncmp(state->input + tok->i, "null", len))
                tok->type = TOK_NULL;
            else if (!strncmp(state->input + tok->i, "false", len))
                tok->type = TOK_FALSE;
            else if (!strncmp(state->input + tok->i, "true", len))
                tok->type = TOK_TRUE;
            else
                return 1;

            return 0;
        } else {
            tok->i = state->pos;
            return 1;
        }
    }
}

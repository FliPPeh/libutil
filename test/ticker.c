#include <libutil/dstring.h>
#include <libutil/json.h>
#include <libutil/container/hashtable.h>
#include <libutil/container/list.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void print_bitcoin_ticker(struct json_value *ticker);

int main(int argc, char **argv) {
    FILE *f = NULL;

    if ((f = fopen("ticker.json", "r")) != NULL) {
        char buffer[BUFSIZ] = {0};
        char *file = NULL;

        while (fread(buffer, 1, sizeof(buffer) - 1, f) > 0) {
            file = dstrcati(file, buffer);

            memset(buffer, 0, sizeof(buffer));
        }

        struct json_value *v = NULL;

        if ((v = json_parse(file)) != NULL) {
            print_bitcoin_ticker(v);
            json_free(v);
        } else {
            printf("Bad JSON!\n");
        }

        free(file);
        fclose(f);
    } else {
        perror("fopen()");
    }
}

void print_bitcoin_ticker(struct json_value *ticker)
{
    struct hashtable *rootobj = json_get_object(ticker);
    struct hashtable_iterator iter;

    hashtable_iterator_init(&iter, rootobj);
    const char *key;
    struct json_value *val;

    while (hashtable_iterator_next(&iter, (void**)&key, (void **)&val)) {
        printf("  %s (%s) / BTC: %.2lf\n",
            key,
            JSON_GET(JSON_OBJECT_LOOKUP(val, "symbol"), const char*),
            JSON_GET(JSON_OBJECT_LOOKUP(val, "15m"), double));
    }
}

# -ansi and -std=c90 will also work with some limitations (everything snprintf)
CFLAGS=-Wall -g -fPIC -Wextra -std=c11 -pedantic -I.
LDFLAGS=-shared -Wl,-soname,libutil.so.1.0
CC=cc

SOURCES=dstring.c json.c utf8.c rc.c container/array.c \
		container/hashtable.c container/heap.c \
		container/list.c container/slist.c

OBJECTS=$(addprefix libutil/, $(addsuffix .o, $(basename $(SOURCES))))

libutil.so.1.0: $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

$(OBJECTS):

clean:
	find -name '*.o' -delete -print

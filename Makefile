CFLAGS=-Wall -g -fPIC -Wextra -std=c11
LDFLAGS=-shared -Wl,-soname,libutil.so.1.0
CC=clang

SOURCES=dstring.c json.c utf8.c container/array.c \
		container/hashtable.c container/heap.c \
		container/list.c container/slist.c

OBJECTS=$(addprefix src/, $(addsuffix .o, $(basename $(SOURCES))))

libutil.so.1.0: $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

$(OBJECTS):


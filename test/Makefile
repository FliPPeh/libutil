CFLAGS=-I../include -Wall -std=c11 -g
LDFLAGS=-Wl,-rpath,../
CC=clang

ticker: ticker.o
	clang  -o $@ ../libutil.so.1.0 ${LDFLAGS} $^ 

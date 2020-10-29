CFLAGS=-g -Wall
CC=gcc
SRCS=tokenizer.c penn-shredder.c
OBJS=tokenizer.o penn-shredder.o
LDFLAGS=
LIBS=

all:    penn-shredder

$(SRCS):
	$(CC) $(CFLAGS) -c $*.c

penn-shredder: $(OBJS)
	$(CC) $(LDFLAGS) $(LIBS) -o penn-shredder $(OBJS)

clean:
	rm -f *.o penn-shredder

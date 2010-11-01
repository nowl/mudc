CC = gcc
CCFLAGS = -Wall -g
INCLUDES = $(shell pkg-config --cflags gtk+-2.0) -I../libtelnetp
LDFLAGS = -L../libtelnetp
LIBS = $(shell pkg-config --libs gtk+-2.0) -ltelnetp

SRCS = \
	base.c \
	telnet.c \
	config.c

OBJS = $(SRCS:.c=.o)

MAIN = base

.SUFFIXES: .c .o

.c.o:
	$(CC) $(CCFLAGS) $(INCLUDES) -c $< -o $@

.PHONY: .depend clean

$(MAIN): $(OBJS)
	$(CC) $(CCFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LIBS) $(LDFLAGS)

clean:
	rm -f *.o *~ $(MAIN)

.depend: $(SRCS)
	$(CC) -M $(CCFLAGS) $(INCLUDES) $^ > $@

include .depend
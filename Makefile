CC = gcc
CCFLAGS = -Wall -g
INCLUDES = $(shell pkg-config --cflags gtk+-2.0) -I../libtelnetp
LDFLAGS = -L../libtelnetp
LIBS = $(shell pkg-config --libs gtk+-2.0) -ltelnetp -lz

SRCS = \
	main.c \
	telnet.c \
	config.c \
	p_codes.c \
	tab_complete.c \
	globals.c \
	entry_handler.c \
	gui.c \
	settings.c \
	worlds.c

OBJS = $(SRCS:.c=.o)

MAIN = mudc

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

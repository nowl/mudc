bin_PROGRAMS = mudc

mudc_SOURCES = \
	main.c \
	telnet.c \
	config.c \
	p_codes.c \
	tab_complete.c \
	globals.c \
	entry_handler.c \
	gui.c \
	settings.c \
	worlds.c \
	macros.c

mudc_CFLAGS = $(GTK_DEPS_CFLAGS) -I../../libtelnetp/include
mudc_LDFLAGS = $(GTK_DEPS_LIBS) -L../../libtelnetp/src -ltelnetp

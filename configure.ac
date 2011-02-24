AC_INIT([mudc], [1.0])
AM_INIT_AUTOMAKE
AM_PROG_LIBTOOL
AC_PROG_CC
AC_CHECK_HEADERS([zlib.h],,[AC_MSG_ERROR([can not find zlib])])
PKG_CHECK_MODULES([GTK_DEPS], [gtk+-2.0])
# TODO: check for libtelnetp and gtk
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT

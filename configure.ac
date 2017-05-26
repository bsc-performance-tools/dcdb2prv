#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([dcdb2prv], [0.4.0], [lau.mercadal@bsc.es])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/dcdb2prv.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_INSTALL

# Checks for libraries.
AC_SEARCH_LIBS([poptGetContext], [popt], [],
							 [AC_MSG_ERROR([Cannot find popt.])])

PKG_CHECK_MODULES([glib2], [glib-2.0 >= 2.40], [],
									[AC_MSG_ERROR([Cannot find glib-2.0.])])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL

# Checks for library functions.
AC_CHECK_FUNCS([strerror strtoul])

AC_CONFIG_FILES([Makefile
                 src/Makefile])

AC_OUTPUT

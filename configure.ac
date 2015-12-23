#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([prvpwr], [0.1], [lau.mercadal@bsc.es])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/prvpwr.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_INSTALL

# Checks for libraries.
AC_SEARCH_LIBS([poptGetContext], [popt], [],
							 [AC_MSG_ERROR([Cannot find popt.])])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL

# Checks for library functions.
AC_CHECK_FUNCS([strerror strtoul])

AC_CONFIG_FILES([Makefile
                 src/Makefile])

AC_OUTPUT

AC_INIT([cycling-sim], [0.1])
AM_INIT_AUTOMAKE([subdir-objects foreign -Wall -Werror])

AC_CONFIG_SRCDIR([src/cycling-sim.c])

AC_PROG_CC
AC_HEADER_STDC

AC_DEFUN([AC_CHECK_HEADER_MANDATORY], [
	AC_CHECK_HEADER([$1], [], [
		AC_MSG_ERROR([missing header: $1])
	])
])

AC_CHECK_HEADER_MANDATORY(pthread.h)

AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES([
	Makefile
	src/Makefile
])

AC_OUTPUT

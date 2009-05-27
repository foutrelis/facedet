AC_INIT([facedet], [1.0.0], [foutrelis@gmail.com])
AC_CONFIG_SRCDIR([src/facedet.c])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC

dnl Check for opencv
PKG_CHECK_MODULES(OPENCV, opencv >= 1.1.0, , AC_MSG_ERROR([OpenCV 1.1.0 or greater is required.]))
AC_SUBST(OPENCV_CFLAGS)
AC_SUBST(OPENCV_LIBS)

dnl Check whether to enable debugging
AC_MSG_CHECKING(whether to enable debugging)
debug_default="no"
AC_ARG_ENABLE(debug, AC_HELP_STRING([--enable-debug], [Turn on debugging]),
                     enable_debug="$enableval", enable_debug="$debug_default")

if test "x$enable_debug" = "xyes"; then
	CFLAGS="$CFLAGS -g -DDEBUG"
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile
                 doc/Makefile
                 src/Makefile])
AC_OUTPUT

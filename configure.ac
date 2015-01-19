dnl $Id$
AC_CONFIG_MACRO_DIR([aclocal])

dnl Bookkeeping
AC_PREREQ(2.52)

dnl Initialize autoconf and automake
AC_INIT([c2ffi], [0.0.0])
AC_SUBST([PACKAGE_NAME])dnl

AC_PROG_CC([clang])
AC_PROG_CXX([clang++])

AM_INIT_AUTOMAKE([subdir-objects])
AM_SILENT_RULES([yes])
AC_CONFIG_HEADERS([defs.h])

dnl Various required programs
AC_PROG_CPP
AX_CXX_COMPILE_STDCXX_11([], [mandatory])
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_LIBTOOL_DLOPEN
AC_DISABLE_STATIC
AC_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)

dnl Various required libraries
AX_PTHREAD([],[AC_MSG_ERROR([-lpthread not found])])
AC_CHECK_LIB(c, dlopen, [],
  AC_CHECK_LIB(dl, dlopen, LIBDL="-ldl",
    AC_MSG_ERROR([dlopen not found])))
AC_CHECK_LIB(curses, setupterm, LIBCURSES="-lcurses",
  AC_CHECK_LIB(ncurses, setupterm, LIBCURSES="-lncurses",
    AC_CHECK_LIB(tinfo, setupterm, LIBCURSES="-ltinfo",
      AC_MSG_ERROR([curses/terminfo not found]))))
AC_CHECK_LIB(z, compress, LIBZ="-lz", AC_MSG_ERROR([-lz not found]))

AC_SUBST(LIBDL)
AC_SUBST(LIBCURSES)
AC_SUBST(LIBZ)

dnl Header checking
AC_HEADER_STDC
AC_CHECK_HEADERS([libintl.h limits.h stdlib.h])
AC_CHECK_HEADERS([sys/types.h])

dnl Typedef checking
AC_CHECK_TYPES([int8_t,  int16_t,  int32_t,  int64_t,
                u_int8_t, u_int16_t, u_int32_t, u_int64_t],,
                AC_MSG_ERROR([portable type not found.]))

dnl Library function checking
AC_FUNC_MALLOC
AC_CHECK_FUNCS([getopt_long])

dnl Miscellaneousness
AC_C_BIGENDIAN

dnl Flags
AC_ARG_ENABLE(debug, [  --enable-debug          enable debugging output [[default=no]]],
[
    if test $enableval = "yes";
     then test -z "$CFLAGS" && CFLAGS="-O0 -g3 -Wall -Werror"
     else CFLAGS="$CFLAGS -DNDEBUG"
    fi
], [
    CFLAGS="$CFLAGS -DNDEBUG"
])

AC_LANG(C++)

AC_MSG_CHECKING([for llvm >= 3.5])
AS_VERSION_COMPARE(`llvm-config --version`, [3.5], [ver_check=-1], [ver_check=0], [ver_check=1])

if test $ver_check -lt 0; then
   AC_MSG_RESULT([no])
   AC_MSG_FAILURE([version 3.5 required], 1)
else
   AC_MSG_RESULT([yes])
   LLVM_CXXFLAGS="`llvm-config --cxxflags`"
   LLVM_LDFLAGS="`llvm-config --ldflags`"
   LLVM_LIBS="`llvm-config --libs`"
fi
AC_SUBST([LLVM_CXXFLAGS])
AC_SUBST([LLVM_LDFLAGS])
AC_SUBST([LLVM_LIBS])

# Check for libclangFrontend.a as a heuristic to make sure the user
# generally has all the libraries necessary

LIBCLANG_CPPFLAGS="$LLVM_CXXFLAGS"
LIBCLANG_LDFLAGS="$LLVM_LDFLAGS"
AX_CHECK_LIBRARY([LIBCLANG], [clang/Frontend/CompilerInstance.h], [clangFrontend],
                 [], [AC_MSG_ERROR([clang development libs not found])])

dnl And output
AC_CONFIG_FILES([Makefile src/include/c2ffi/Makefile src/include/Makefile src/Makefile src/test/Makefile 
               ])

AC_OUTPUT

# Process this file with autoconf to produce a configure script.
AC_INIT(tenebrae, 1.0.2)

# Detect the canonical target build environment
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE

# Checks for sources 
AC_CONFIG_SRCDIR([../sys_sdl.c])
# automake need AM_CONFIG_HEADER but autoconf doesn't want it
AC_CONFIG_HEADER([config.h])


# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# autoconf 2.53 doesn't check that
# AM_PROG_AS
CCAS="$CC"
CCASFLAGS="$CFLAGS"
AC_SUBST(CCAS)
AC_SUBST(CCASFLAGS)


# The alpha architecture needs special flags for binary portability

case "$target" in
    i686-*-linux*)
        #CFLAGS="-g -mpentiumpro -O6 -ffast-math -funroll-loops -fomit-frame-pointer -fexpensive-optimizations"
        #CFLAGS="-g -mpentiumpro -O6 -ffast-math -funroll-loops -fexpensive-optimizations"
        ;;
    alpha*-*-linux*)
        #CFLAGS="$CFLAGS -mcpu=ev4 -Wa,-mall"
        ;;
esac


# Checks for libraries.

# Figure out which math and networking libraries to use
case "$target" in
    *-*-mingw32*)
        MATHLIB=""
        INETLIB="-lwsock32"
        ;;
    *-*-beos*)
        MATHLIB=""
        INETLIB=""
        ;;
    *)
        MATHLIB="-lm"
        INETLIB=""
        ;;
esac
AC_SUBST(MATHLIB)
AC_SUBST(INETLIB)

# Check for SDL
SDL_VERSION=1.2.0
AM_PATH_SDL($SDL_VERSION,
            :,
            AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!])
)

# Game data 
QUAKE_GAMEDIR=games/quake/

AC_SUBST(QUAKE_GAMEDIR)


CFLAGS="$CFLAGS $SDL_CFLAGS -DSDL"
LIBS="$LIBS $SDL_LIBS"

# Enable/disable the i686 asm

AC_ARG_ENABLE(asm,
[  --enable-asm            enable i686 assembly routines [default=no]],
  ,[enable_asm=no])
if test x$enable_asm = xyes; then
    CFLAGS="$CFLAGS -DUSE_ASM"
fi

CFLAGS="$CFLAGS -DELF"

# Checks for header files.
AC_PATH_X
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h netdb.h netinet/in.h stddef.h stdlib.h string.h sys/file.h sys/ioctl.h sys/param.h sys/socket.h sys/time.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_ALLOCA
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MMAP
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([atexit floor gethostbyaddr gethostbyname gethostname gethrtime getpagesize gettimeofday getwd inet_ntoa memset mkdir munmap pow select socket sqrt strcasecmp strchr strerror strstr])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
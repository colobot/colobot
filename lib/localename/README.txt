Part of gnulib library (http://www.gnu.org/software/gnulib/)

Original source with added patch to also include LANGUAGE environment variable.

Orignal module description below

================================

Description:
Return current locale's name, according to glibc naming conventions.

Files:
lib/localename.h
lib/localename.c
m4/localename.m4
m4/intlmacosx.m4
m4/lcmessage.m4

Depends-on:
strdup
lock

configure.ac:
gl_LOCALENAME

Makefile.am:
lib_SOURCES += localename.c

Include:
"localename.h"

Link:
@INTL_MACOSX_LIBS@
$(LTLIBTHREAD) when linking with libtool, $(LIBTHREAD) otherwise

License:
LGPLv2+

Maintainer:
Bruno Haible


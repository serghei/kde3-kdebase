bin_PROGRAMS = kfontinst
kfontinst_SOURCES = \
Main.cpp \
FontEngine.cpp \
Fontmap.cpp \
GetPid.c \
XConfig.cpp

noinst_HEADERS= \
FontEngine.h \
Fontmap.h \
XConfig.h

kfontinst_LDADD = ../../fonts/libkxftconfig.la $(LIBFONTCONFIG_LIBS) $(LIBFREETYPE_LIBS) $(LIBZ) $(LIB_KIO) ../lib/libkfontinst.la
kfontinst_LDFLAGS = $(all_libraries) $(LIBFONTCONFIG_RPATH) $(LIBFREETYPE_RPATH) $(KDE_RPATH)
AM_CPPFLAGS= -DOS_$(UNAME) -I$(srcdir)/../lib -I$(srcdir)/../../fonts $(all_includes) $(LIBFREETYPE_CFLAGS) $(LIBFONTCONFIG_CFLAGS)

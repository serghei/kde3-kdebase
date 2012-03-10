INCLUDES = -I$(top_srcdir)/kate/utils $(all_includes)

EXTRA_DIST = README

lib_LTLIBRARIES = libkateutils.la

libkateutils_la_SOURCES = dockviewbase.cpp messageview.cpp listboxview.cpp

libkateutils_la_LDFLAGS = $(all_libraries) -no-undefined
                      
libkateutils_la_LIBADD = $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_KIO)

libkateutils_la_METASOURCES = AUTO

kateutilsinclude_HEADERS = dockviewbase.h messageview.h listboxview.h
kateutilsincludedir = $(includedir)/kate/utils

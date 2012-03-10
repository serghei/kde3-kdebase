AM_CPPFLAGS = -D_KDEPRINT_COMPILE

kde_module_LTLIBRARIES = libkdeprint_part.la

libkdeprint_part_la_SOURCES = printpart.cpp
libkdeprint_part_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
libkdeprint_part_la_LIBADD = -lkdeprint_management $(LIB_KPARTS)

noinst_HEADERS = printpart.h

METASOURCES = AUTO
INCLUDES = $(all_includes)

# service
myservice_DATA = kdeprint_part.desktop
myservicedir = $(kde_servicesdir)

# UI file
myrcfile_DATA = kdeprint_part.rc
myrcfiledir = $(kde_datadir)/kdeprint_part

messages: rc.cpp
	$(EXTRACTRC) kdeprint_part.rc >> rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kdeprint_part.pot

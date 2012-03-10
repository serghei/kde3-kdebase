AM_CPPFLAGS = -D_KDEPRINT_COMPILE

SUBDIRS = mimetypes templates

INCLUDES= $(all_includes)

kde_module_LTLIBRARIES = kio_print.la

kio_print_la_SOURCES = kio_print.cpp
kio_print_la_LIBADD = $(LIB_KIO) -lkdeprint
kio_print_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

METASOURCES = AUTO

noinst_HEADERS = kio_print.h

# Services
servicesdir = $(kde_servicesdir)
services_DATA = print.protocol printdb.protocol

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kio_print.pot

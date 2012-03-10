AM_CPPFLAGS = -D_KDEPRINT_COMPILE

INCLUDES = $(all_includes)

bin_PROGRAMS = 
lib_LTLIBRARIES = 
kdeinit_LTLIBRARIES = kprinter.la

kprinter_la_SOURCES = main.cpp printwrapper.cpp
kprinter_la_LIBADD = -lkdeprint
kprinter_la_LDFLAGS = -module -avoid-version $(all_libraries) $(KDE_RPATH)
kprinter_la_METASOURCES = AUTO

noinst_HEADERS = printwrapper.h

messages:
	$(XGETTEXT) *.cpp -o $(podir)/kprinter.pot


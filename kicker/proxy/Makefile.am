INCLUDES = -I$(srcdir)/../libkicker $(all_includes)

bin_PROGRAMS = 
noinst_PROGRAMS = extensiondebugger
lib_LTLIBRARIES =
kdeinit_LTLIBRARIES = appletproxy.la extensionproxy.la

METASOURCES = AUTO

appletproxy_la_LIBADD = $(LIB_KDEUI) ../libkicker/libkickermain.la
appletproxy_la_SOURCES = appletproxy.cpp
appletproxy_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -module -avoid-version

extensionproxy_la_LIBADD = $(LIB_KDEUI) ../libkicker/libkickermain.la
extensionproxy_la_SOURCES = extensionproxy.cpp
extensionproxy_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -module -avoid-version

extensiondebugger_SOURCES = extensiondebugger.cpp
extensiondebugger_LDADD = ../libkicker/libkickermain.la
extensiondebugger_LDFLAGS = $(all_libraries) $(KDE_RPATH)

messages:
	$(XGETTEXT) appletproxy.cpp -o $(podir)/appletproxy.pot
	$(XGETTEXT) extensionproxy.cpp -o $(podir)/extensionproxy.pot

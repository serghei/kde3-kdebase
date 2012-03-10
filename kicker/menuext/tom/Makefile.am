INCLUDES = -I$(srcdir)/../../libkicker -I$(srcdir)/../../ui -I$(srcdir)/../../core $(all_includes)

kde_module_LTLIBRARIES = kickermenu_tom.la

kickermenu_tom_la_SOURCES = tom.cc
kickermenu_tom_la_LDFLAGS = $(all_libraries) -module -avoid-version
kickermenu_tom_la_LIBADD = $(LIB_KDEUI)
#$(top_builddir)/kicker/ui/libkicker_ui.la 

kickermenu_tom_la_METASOURCES = AUTO

desktopmenu_DATA = tom.desktop 
desktopmenudir = $(kde_datadir)/kicker/menuext

tomdata_DATA = destinations
tomdatadir = $(kde_datadir)/kicker/tom

messages:
	$(XGETTEXT) *.cc -o $(podir)/libkickermenu_tom.pot

INCLUDES = $(all_includes)

kde_module_LTLIBRARIES = kickermenu_recentdocs.la

kickermenu_recentdocs_la_SOURCES = recentdocsmenu.cpp
kickermenu_recentdocs_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
kickermenu_recentdocs_la_LIBADD = $(LIB_KDEUI) $(LIB_KIO)

kickermenu_recentdocs_la_METASOURCES = AUTO

desktopmenu_DATA = recentdocs.desktop
desktopmenudir = $(kde_datadir)/kicker/menuext

messages:
	$(XGETTEXT) *.cpp -o $(podir)/libkickermenu_recentdocs.pot

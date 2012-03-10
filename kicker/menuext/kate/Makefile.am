INCLUDES = $(all_includes)

kde_module_LTLIBRARIES = kickermenu_kate.la

kickermenu_kate_la_SOURCES = katesessionmenu.cpp
kickermenu_kate_la_LDFLAGS = $(all_libraries) -module -avoid-version
kickermenu_kate_la_LIBADD = $(LIB_KDEUI)

kickermenu_kate_la_METASOURCES = AUTO

desktopmenu_DATA = katesessionmenu.desktop
desktopmenudir = $(kde_datadir)/kicker/menuext

messages:
	$(XGETTEXT) *.cpp -o $(podir)/kickermenu_kate.pot
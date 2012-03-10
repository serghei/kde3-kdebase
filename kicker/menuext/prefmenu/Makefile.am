INCLUDES = -I../../libkicker -I$(top_srcdir)/kicker/libkicker $(all_includes)

kde_module_LTLIBRARIES = kickermenu_prefmenu.la

kickermenu_prefmenu_la_SOURCES = prefmenu.cpp
kickermenu_prefmenu_la_LDFLAGS = $(all_libraries) -module -avoid-version
kickermenu_prefmenu_la_LIBADD = $(LIB_KDEUI) $(top_builddir)/kicker/libkicker/libkickermain.la

kickermenu_prefmenu_la_METASOURCES = AUTO

desktopmenu_DATA = prefmenu.desktop
desktopmenudir = $(kde_datadir)/kicker/menuext

messages:
	$(XGETTEXT) *.cpp -o $(podir)/libkickermenu_prefmenu.pot

prefmenu.lo: ../../libkicker/kickerSettings.h

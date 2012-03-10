INCLUDES = $(all_includes)

kde_module_LTLIBRARIES = dockbar_panelextension.la

dockbar_panelextension_la_SOURCES = dockcontainer.cpp dockbarextension.cpp
dockbar_panelextension_la_METASOURCES = AUTO
dockbar_panelextension_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
dockbar_panelextension_la_LIBADD = $(LIB_KDEUI)

noinst_HEADERS = dockcontainer.h dockbarextension.h

lnkdir = $(kde_datadir)/kicker/extensions
lnk_DATA = dockbarextension.desktop

EXTRA_DIST = $(lnk_DATA)

messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/dockbarextension.pot

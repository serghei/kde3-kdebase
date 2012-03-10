INCLUDES = $(all_includes) 

kde_module_LTLIBRARIES = run_panelapplet.la

run_panelapplet_la_SOURCES = runapplet.cpp

METASOURCES = runapplet.moc
noinst_HEADERS = runapplet.h

lnkdir = $(kde_datadir)/kicker/applets
lnk_DATA = runapplet.desktop

EXTRA_DIST = $(lnk_DATA)

run_panelapplet_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
run_panelapplet_la_LIBADD = $(LIB_KSYCOCA) $(LIB_KDEUI)

messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/krunapplet.pot

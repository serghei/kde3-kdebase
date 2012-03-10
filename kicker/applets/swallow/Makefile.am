INCLUDES = $(all_includes)

kde_module_LTLIBRARIES = swallow_panelapplet.la

swallow_panelapplet_la_LDFLAGS	= -module $(KDE_PLUGIN) $(all_libraries)
swallow_panelapplet_la_LIBADD = $(LIB_KDEUI) $(LIB_KIO)

swallow_panelapplet_la_SOURCES = swallow.cpp prefwidgetbase.ui prefwidget.cpp

noinst_HEADERS = swallow.h prefwidget.h prefwidgetbase.h

swallow_panelapplet_la_METASOURCES = AUTO

applnk_DATA = swallowapplet.desktop
applnkdir = $(kde_datadir)/kicker/applets

#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/swallowapplet.pot

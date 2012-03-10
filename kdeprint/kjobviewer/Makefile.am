AM_CPPFLAGS = -D_KDEPRINT_COMPILE

INCLUDES = $(all_includes)

bin_PROGRAMS=
lib_LTLIBRARIES=
kdeinit_LTLIBRARIES = kjobviewer.la

kjobviewer_la_SOURCES = kjobviewer.cpp main.cpp
kjobviewer_la_METASOURCES = AUTO
kjobviewer_la_LIBADD = -lkdeprint_management $(LIB_KDEUI)
kjobviewer_la_LDFLAGS = -module -avoid-version $(all_libraries) $(KDE_RPATH)

noinst_HEADERS = kjobviewer.h

KDE_ICON = kjobviewer

rc_DATA = kjobviewerui.rc
rcdir = $(kde_datadir)/kjobviewer

xdg_apps_DATA = kjobviewer.desktop

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kjobviewer.pot

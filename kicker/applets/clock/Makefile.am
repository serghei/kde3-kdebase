pic_DATA = lcd.png
picdir = $(kde_datadir)/clockapplet/pics

INCLUDES = -I$(top_srcdir)/kicker/libkicker $(all_includes)

kde_module_LTLIBRARIES = clock_panelapplet.la

clock_panelapplet_la_SOURCES = clock.skel clock.cpp datepicker.cpp zone.cpp analog.ui digital.ui fuzzy.ui settings.ui prefs.kcfgc

METASOURCES = AUTO
noinst_HEADERS = clock.h datepicker.h zone.h

lnkdir = $(kde_datadir)/kicker/applets
lnk_DATA = clockapplet.desktop

EXTRA_DIST = $(lnk_DATA) $(pic_DATA)

clock_panelapplet_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
clock_panelapplet_la_LIBADD =  ../../libkicker/libkickermain.la $(LIB_KDEUI)

srcdoc:
	kdoc -a -p -H -d $$HOME/web/src/clockapplet clockapplet *.h -lqt -lkdecore -lkdeui -lkfile

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/clockapplet.pot

KDE_OPTIONS=nofinal

clock_skel.lo: settings.h

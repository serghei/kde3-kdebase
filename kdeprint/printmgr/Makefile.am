AM_CPPFLAGS = -D_KDEPRINT_COMPILE

INCLUDES = $(all_includes)

kde_module_LTLIBRARIES = kcm_printmgr.la

kcm_printmgr_la_SOURCES = kcmprintmgr.cpp
kcm_printmgr_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
kcm_printmgr_la_LIBADD = -lkdeprint_management

noinst_HEADERS = kcmprintmgr.h

KDE_ICON = printmgr

xdg_apps_DATA = printers.desktop

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmprintmgr.pot

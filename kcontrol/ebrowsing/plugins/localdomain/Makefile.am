## Makefile.am of kdebase/kcontrol/ebrowsing/plugins/localdomain

AM_CPPFLAGS   =  $(all_includes) -DQT_NO_CAST_ASCII

kde_module_LTLIBRARIES = liblocaldomainurifilter.la

liblocaldomainurifilter_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
liblocaldomainurifilter_la_LIBADD = $(LIB_KIO) $(LIB_KDEUI) $(LIB_KDECORE)

liblocaldomainurifilter_la_SOURCES = localdomainurifilter.cpp localdomainurifilter.skel

service_DATA = localdomainurifilter.desktop
servicedir = $(kde_servicesdir)

METASOURCES = AUTO

bin_PROGRAMS = klocaldomainurifilterhelper

klocaldomainurifilterhelper_SOURCES = klocaldomainurifilterhelper.c
klocaldomainurifilterhelper_LDADD = $(LIBSOCKET)

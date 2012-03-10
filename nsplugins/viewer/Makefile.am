INCLUDES = -I$(top_srcdir)/nsplugins -I$(top_builddir)/nsplugins $(all_includes) `pkg-config --cflags glib-2.0`
METASOURCES = AUTO

bin_PROGRAMS = nspluginviewer 

nspluginviewer_SOURCES = NSPluginCallbackIface.stub NSPluginClassIface.skel \
	nsplugin.cpp viewer.cpp kxt.cpp qxteventloop.cpp glibevents.cpp
nspluginviewer_LDFLAGS = $(all_libraries) $(KDE_RPATH) -export-dynamic `pkg-config --libs glib-2.0`
nspluginviewer_LDADD = $(LIB_KIO) $(LIB_KPARTS) -lXt

NSPluginCallbackIface_DIR = $(srcdir)/..

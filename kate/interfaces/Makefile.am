METASOURCES = AUTO

noinst_LTLIBRARIES = libkateinterfacesprivate.la

libkateinterfacesprivate_la_SOURCES = application.cpp mainwindow.cpp documentmanager.cpp viewmanager.cpp toolviewmanager.cpp \
                                        pluginmanager.cpp plugin.cpp pluginconfiginterface.cpp pluginconfiginterfaceextension.cpp

libkateinterfacesprivate_la_LIBADD   = -lkatepartinterfaces
libkateinterfacesprivate_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

kateinclude_HEADERS = application.h documentmanager.h mainwindow.h plugin.h viewmanager.h pluginconfiginterface.h pluginconfiginterfaceextension.h toolviewmanager.h pluginmanager.h
kateincludedir = $(includedir)/kate

INCLUDES= $(all_includes)

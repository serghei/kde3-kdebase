METASOURCES = AUTO 

kde_module_LTLIBRARIES = media_propsdlgplugin.la

media_propsdlgplugin_la_LIBADD = ../libmediacommon/libmediacommon.la $(LIB_KIO)
media_propsdlgplugin_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

AM_CPPFLAGS = $(all_includes)

kde_services_DATA  = media_propsdlgplugin.desktop

media_propsdlgplugin_la_SOURCES = propertiespage.cpp propertiespagegui.ui propsdlgshareplugin.cpp

INCLUDES = $(all_includes)

lib_LTLIBRARIES = libkickermain.la

libkickermain_la_SOURCES = appletinfo.cpp global.cpp kickertip.cpp \
                           menuinfo.cpp panelbutton.cpp panner.cpp \
                           kickerSettings.kcfgc kshadowsettings.cpp \
                           kshadowengine.cpp paneldrag.cpp \
                           simplebutton.cpp

libkickermain_la_METASOURCES = AUTO

libkickermain_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -no-undefined
libkickermain_la_LIBADD = $(LIB_KIO)

kde_kcfg_DATA = kickerSettings.kcfg

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/libkicker.pot

INCLUDES= $(all_includes)

bin_PROGRAMS = kpager

kpager_SOURCES = desktop.cpp kpager.cpp config.cpp windowdrag.cpp \
	kpagerIface.skel main.cpp  

kpager_METASOURCES = AUTO
kpager_LDFLAGS = $(all_libraries) $(KDE_RPATH)

kpager_LDADD = $(LIB_KDEUI)

KDE_ICON = kpager

xdg_apps_DATA = kpager.desktop

install-data-local: uninstall.desktop
	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)/Utilities
	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(kde_appsdir)/Utilities/kpager.desktop

messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/kpager.pot

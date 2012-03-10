INCLUDES= $(all_includes)

bin_PROGRAMS = kdeprintfax
kdeprintfax_SOURCES = main.cpp kdeprintfax.cpp faxab.cpp faxctrl.cpp confgeneral.cpp configdlg.cpp \
		      conffax.cpp confsystem.cpp conffilters.cpp filterdlg.cpp defcmds.cpp
kdeprintfax_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kdeprintfax_LDADD   = $(LIB_KDEUI) $(LIB_KIO) $(LIB_KDEPRINT) -lkabc
kdeprintfax_METASOURCES = AUTO

xdg_apps_DATA = kdeprintfax.desktop

uidir = $(kde_datadir)/kdeprintfax
ui_DATA = kdeprintfaxui.rc faxfilters

private_SCRIPTS = anytops
privatedir = $(kde_datadir)/kdeprintfax

messages: rc.cpp
	$(XGETTEXT) `find . -name \*.h -o -name \*.cpp -o -name \*.cc` -o $(podir)/kdeprintfax.pot

KDE_ICON = kdeprintfax
pic_ICON = abentry
picdir = $(kde_datadir)/kdeprintfax/icons

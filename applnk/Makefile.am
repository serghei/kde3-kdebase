SUBDIRS = compat

xdg_menu_DATA = kde-settings.menu kde-information.menu kde-screensavers.menu

xdg_mergedmenu_DATA = kde-essential.menu
xdg_mergedmenudir = $(xdg_menudir)/applications-merged

xdg_directory_DATA = \
kde-main.directory \
kde-development.directory \
kde-editors.directory \
kde-edutainment.directory \
kde-games-arcade.directory \
kde-games-board.directory \
kde-games-card.directory \
kde-games-kids.directory \
kde-games-roguelikes.directory \
kde-games-strategy.directory \
kde-games.directory \
kde-graphics.directory \
kde-information.directory \
kde-internet-terminal.directory \
kde-internet.directory \
kde-more.directory \
kde-multimedia.directory \
kde-office.directory \
kde-science.directory \
kde-settings-accessibility.directory \
kde-settings-components.directory \
kde-settings-desktop.directory \
kde-settings-looknfeel.directory \
kde-settings-network.directory \
kde-settings-peripherals.directory \
kde-settings-hardware.directory \
kde-settings-power.directory \
kde-settings-security.directory \
kde-settings-sound.directory \
kde-settings-system.directory \
kde-settings-webbrowsing.directory \
kde-settings.directory \
kde-settingsmenu.directory \
kde-system-screensavers.directory \
kde-system-terminal.directory \
kde-system.directory \
kde-toys.directory \
kde-unknown.directory \
kde-utilities-xutils.directory \
kde-utilities.directory \
kde-utilities-accessibility.directory \
kde-utilities-desktop.directory \
kde-utilities-peripherals.directory \
kde-utilities-file.directory \
kde-utilities-pim.directory \
kde-development-translation.directory \
kde-development-webdevelopment.directory

install-data-local:
	for file in `cat $(srcdir)/old_desktop_files` ; do \
	    if test -f $(DESTDIR)$(xdg_appsdir)/$${file} ; then \
	        rm $(DESTDIR)$(xdg_appsdir)/$${file} ; \
		echo Deleting $(DESTDIR)$(xdg_appsdir)/$${file} ; \
	    fi; \
	done;

	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)
	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)/.hidden
	$(INSTALL_DATA) $(srcdir)/hidden.directory $(DESTDIR)$(kde_appsdir)/.hidden/.directory

uninstall-local:
	-rm -f $(DESTDIR)$(kde_appsdir)/.hidden/.directory

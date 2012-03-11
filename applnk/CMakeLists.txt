#################################################
#
#  (C) 2010-2012 Serghei Amelian
#  serghei (DOT) amelian (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

add_subdirectory( compat )

install( FILES
    kde-settings.menu kde-information.menu kde-screensavers.menu
  DESTINATION ${XDG_MENU_INSTALL_DIR} )

install( FILES
    kde-essential.menu
  DESTINATION ${XDG_MENU_INSTALL_DIR}/applications-merged )

install( FILES
    hidden.directory
  DESTINATION ${APPS_INSTALL_DIR}/.hidden
  RENAME .directory )

install( FILES
    kde-main.directory kde-development.directory kde-editors.directory
    kde-edutainment.directory kde-games-arcade.directory
    kde-games-board.directory kde-games-card.directory
    kde-games-kids.directory kde-games-roguelikes.directory
    kde-games-strategy.directory kde-games.directory
    kde-graphics.directory kde-information.directory
    kde-internet-terminal.directory kde-internet.directory
    kde-more.directory kde-multimedia.directory kde-office.directory
    kde-science.directory kde-settings-accessibility.directory
    kde-settings-components.directory kde-settings-desktop.directory
    kde-settings-looknfeel.directory kde-settings-network.directory
    kde-settings-peripherals.directory kde-settings-hardware.directory
    kde-settings-power.directory kde-settings-security.directory
    kde-settings-sound.directory kde-settings-system.directory
    kde-settings-webbrowsing.directory kde-settings.directory
    kde-settingsmenu.directory kde-system-screensavers.directory
    kde-system-terminal.directory kde-system.directory
    kde-toys.directory kde-unknown.directory
    kde-utilities-xutils.directory kde-utilities.directory
    kde-utilities-accessibility.directory kde-utilities-desktop.directory
    kde-utilities-peripherals.directory kde-utilities-file.directory
    kde-utilities-pim.directory kde-development-translation.directory
    kde-development-webdevelopment.directory
  DESTINATION ${XDG_DIRECTORY_INSTALL_DIR} )

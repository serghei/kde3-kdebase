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

include_directories(
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES
    kwin.upd kwinsticky.upd kwiniconify.upd kwin3_plugin.upd
    kwin_focus1.upd kwinupdatewindowsettings.upd
    kwin_focus2.upd kwin_fsp_workarounds_1.upd
  DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )

install( PROGRAMS
    pluginlibFix.pl kwin3_plugin.pl kwin_focus1.sh kwin_focus2.sh
  DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )

install( FILES
    pop.wav
  DESTINATION ${SOUND_INSTALL_DIR} )

install( FILES
    fsp_workarounds_1.kwinrules
  DESTINATION ${DATA_INSTALL_DIR}/kwin/default_rules )


##### kwin_update_window_settings (executable) ##

kde_add_executable( kwin_update_window_settings
  SOURCES update_window_settings.cpp
  LINK kdecore-shared
  DESTINATION ${LIB_INSTALL_DIR}/kconf_update_bin
)


##### kwin_update_default_rules (executable) ####

kde_add_executable( kwin_update_default_rules
  SOURCES update_default_rules.cpp
  LINK kdecore-shared
  DESTINATION ${LIB_INSTALL_DIR}/kconf_update_bin
)

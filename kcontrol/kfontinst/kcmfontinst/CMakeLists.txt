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
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kcontrol/kfontinst/lib
  ${CMAKE_SOURCE_DIR}/kcontrol/kfontinst/viewpart
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

kde_install_icons()
install( FILES kcmfontinst.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### kcm_fontinst (module) #####################

kde_add_kpart( kcm_fontinst AUTOMOC
  SOURCES
    KCmFontInst.cpp KFileFontIconView.cpp KFileFontView.cpp
    PrintDialog.cpp SettingsDialog.cpp
  LINK kfontinstprint-static kfontinst-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

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
  ${CMAKE_BINARY_DIR}/kicker/libkicker
  ${CMAKE_SOURCE_DIR}/kicker/libkicker
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES clockapplet.desktop DESTINATION ${DATA_INSTALL_DIR}/kicker/applets )
install( FILES lcd.png DESTINATION ${DATA_INSTALL_DIR}/clockapplet/pics )


##### clock_panelapplet (module) ################

kde_add_kpart( clock_panelapplet AUTOMOC
  SOURCES
    clock.skel clock.cpp datepicker.cpp zone.cpp analog.ui
    digital.ui fuzzy.ui settings.ui prefs.kcfgc
  DEPENDENCIES clock_panelapplet-static
  LINK kickermain-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

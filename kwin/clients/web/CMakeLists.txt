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
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES web.desktop DESTINATION ${DATA_INSTALL_DIR}/kwin )


##### kwin3_web (module) ########################

kde_add_kpart( kwin3_web AUTOMOC
  SOURCES Web.cpp WebButton.cpp
  LINK kdecorations-shared kdeui-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

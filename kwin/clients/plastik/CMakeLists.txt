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

add_subdirectory( config )

add_definitions( -DQT_PLUGIN )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kwin/lib
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES plastik.desktop DESTINATION ${DATA_INSTALL_DIR}/kwin )


##### kwin3_plastik (module) ####################

kde_add_kpart( kwin3_plastik AUTOMOC
  SOURCES plastik.cpp plastikclient.cpp plastikbutton.cpp misc.cpp
  LINK kdecorations-shared kdeui-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

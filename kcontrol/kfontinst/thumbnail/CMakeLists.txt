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
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kcontrol/kfontinst/lib
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES fontthumbnail.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


##### fontthumbnail (module) ####################

kde_add_kpart( fontthumbnail
  SOURCES FontThumbnail.cpp
  LINK kfontinst-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

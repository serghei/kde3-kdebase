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
  ${CMAKE_SOURCE_DIR}/khotkeys/shared
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### khotkeys_update (executable) ##############

kde_add_executable( khotkeys_update
  SOURCES update.cpp
  LINK khotkeys_shared-shared
  DESTINATION ${LIB_INSTALL_DIR}/kconf_update_bin
)

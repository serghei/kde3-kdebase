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


##### other data ################################

install( FILES khotkeys.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded )


##### kded_khotkeys (module) ####################

kde_add_kpart( kded_khotkeys AUTOMOC
  SOURCES kded.cpp kded.skel
  LINK khotkeys_shared-shared kdeinit_kded-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### khotkeys (kdeinit) ########################

kde_add_kdeinit_executable( khotkeys AUTOMOC
  SOURCES app.cpp app.skel
  LINK khotkeys_shared-shared
)

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

if( WITH_ARTS )
  set( HAVE_ARTS 1 CACHE INTERNAL "" FORCE )
endif( )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/khotkeys/kcontrol
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/khotkeys/shared
  ${CMAKE_SOURCE_DIR}/khotkeys/kcontrol
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### khotkeys_arts (module) #######################

kde_add_kpart( khotkeys_arts AUTOMOC
  SOURCES soundrecorder_arts.cpp voicerecorder_arts.cpp
  LINK khotkeys_shared-shared artskde-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
  DEPENDENCIES ui-static
)

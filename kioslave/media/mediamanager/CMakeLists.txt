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
  ${CMAKE_CURRENT_BINARY_DIR}/../libmediacommon
  ${CMAKE_CURRENT_SOURCE_DIR}/../libmediacommon
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
  ${HAL_INCLUDE_DIRS}
)


##### other data ################################

install( FILES mediamanager.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded )


##### kded_mediamanager (module) ################

if( WITH_UDISKS2 )
  set_source_files_properties( mediamanager.cpp PROPERTIES COMPILE_FLAGS "-DCOMPILE_UDISKS2BACKEND" )
  set( KDEDBUS_LIBRARY kdedbus-shared )
  list( APPEND backends udisks2backend.cpp )
endif( )

if( WITH_HAL )
  list( APPEND backends halbackend.cpp )
endif( )

kde_add_kpart( kded_mediamanager AUTOMOC
  SOURCES
    mediamanager.cpp mediamanager.skel medialist.cpp
    backendbase.cpp fstabbackend.cpp removablebackend.cpp
    mediadirnotify.cpp mediadirnotify.skel ${backends}
  LINK
    mediacommon-static kdeinit_kded-shared
    ${KDEDBUS_LIBRARY} ${HAL_LIBRARIES}
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

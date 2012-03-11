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

# FIXME there is no support for HAL and linuxcdpolling yet


include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/kioslave/media/libmediacommon
  ${CMAKE_SOURCE_DIR}/kioslave/media/libmediacommon
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
  ${HAL_INCLUDE_DIRS}
)


##### other data ################################

install( FILES mediamanager.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded )


##### kded_mediamanager (module) ################

set( target kded_mediamanager )

if( WITH_HAL )
  set( ${target}_SRCS
    mediamanager.cpp mediamanager.skel medialist.cpp
    backendbase.cpp fstabbackend.cpp removablebackend.cpp
    mediadirnotify.cpp mediadirnotify.skel
    halbackend.cpp linuxcdpolling.cpp
  )

  kde_add_kpart( ${target} AUTOMOC
    SOURCES ${${target}_SRCS}
    LINK mediacommon-static kdeinit_kded-shared ${HAL_LIBRARIES} -lhal-storage ${DBUS_TQT_LIBRARIES}
    DESTINATION ${PLUGIN_INSTALL_DIR}
  )
else( )
  set( ${target}_SRCS
    mediamanager.cpp mediamanager.skel medialist.cpp
    backendbase.cpp fstabbackend.cpp removablebackend.cpp
    mediadirnotify.cpp mediadirnotify.skel
  )

  kde_add_kpart( ${target} AUTOMOC
    SOURCES ${${target}_SRCS}
    LINK mediacommon-static kdeinit_kded-shared
    DESTINATION ${PLUGIN_INSTALL_DIR}
  )
endif( )

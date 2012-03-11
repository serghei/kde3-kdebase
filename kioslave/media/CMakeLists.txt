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

add_subdirectory( libmediacommon )
add_subdirectory( mediamanager )
add_subdirectory( medianotifier )
add_subdirectory( mounthelper )
add_subdirectory( kfile-plugin )
add_subdirectory( kcmodule )
add_subdirectory( mimetypes )
add_subdirectory( services )
kde_conditional_add_subdirectory( WITH_HAL propsdlgplugin )


include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/libmediacommon
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES media.protocol DESTINATION ${SERVICES_INSTALL_DIR} )


##### kiomedia (static) #########################

set( target kiomedia )

kde_add_library( ${target} STATIC_PIC AUTOMOC
  SOURCES kio_media.cpp mediaimpl.cpp mediaimpl.skel
)


##### kio_media (module) ########################

set( target kio_media )

kde_add_kpart( ${target} AUTOMOC
  SOURCES dummy.cpp
  EMBED kiomedia-static
  LINK mediacommon-static kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

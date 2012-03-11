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

add_subdirectory( kdedmodule )
add_subdirectory( entries )
add_subdirectory( mimetypes )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES system.protocol DESTINATION ${SERVICES_INSTALL_DIR} )


##### kiosystem (static) ########################

set( target kiosystem )

kde_add_library( ${target} STATIC_PIC AUTOMOC
  SOURCES kio_system.cpp systemimpl.cpp
)


##### kio_system (module) #######################

set( target kio_system )

kde_add_kpart( ${target} AUTOMOC
  SOURCES dummy.cpp
  EMBED kiosystem-static
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

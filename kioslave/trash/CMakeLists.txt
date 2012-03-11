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

add_subdirectory( kfile-plugin )


include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES trash.protocol DESTINATION ${SERVICES_INSTALL_DIR} )


##### trashcommon (static) ######################

set( target trashcommon )

kde_add_library( ${target} STATIC_PIC AUTOMOC
  SOURCES trashimpl.cpp
)


##### kio_trash (module) ########################

set( target kio_trash )

kde_add_kpart( ${target} AUTOMOC
  SOURCES kio_trash.cpp
  LINK trashcommon-static kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### ktrash (executable) #######################

set( target ktrash )

kde_add_executable( ${target}
  SOURCES ktrash.cpp
  LINK kio-shared
  DESTINATION ${BIN_INSTALL_DIR}
)

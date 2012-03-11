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

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES remote.protocol DESTINATION ${SERVICES_INSTALL_DIR} )


##### kioremote (static) ########################

set( target kioremote )

kde_add_library( ${target} STATIC_PIC AUTOMOC
  SOURCES kio_remote.cpp remoteimpl.cpp
)


##### kio_remote (module) #######################

set( target kio_remote )

configure_file( ${TEMPLATE_DUMMY_CPP} dummy.cpp COPYONLY )

kde_add_kpart( ${target}
  SOURCES dummy.cpp
  EMBED kioremote-static
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

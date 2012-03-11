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

install( FILES home.protocol DESTINATION ${SERVICES_INSTALL_DIR} )


##### kiohome (static) ##########################

set( target kiohome )

kde_add_library( ${target} STATIC_PIC AUTOMOC
  SOURCES kio_home.cpp homeimpl.cpp
)


##### kio_home (module) #########################

set( target kio_home )

kde_add_kpart( ${target}
  SOURCES dummy.cpp
  EMBED kiohome-static
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

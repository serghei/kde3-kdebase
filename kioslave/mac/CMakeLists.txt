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
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES mac.protocol DESTINATION ${SERVICES_INSTALL_DIR} )
kde_install_icons( )


##### kio_mac (module) ##########################

set( target kio_mac )

kde_add_kpart( ${target} AUTOMOC
  SOURCES kio_mac.cpp
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


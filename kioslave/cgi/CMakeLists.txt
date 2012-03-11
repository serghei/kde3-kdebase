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

add_subdirectory( kcmcgi )


include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES cgi.protocol DESTINATION ${SERVICES_INSTALL_DIR} )


##### kio_cgi (module) ##########################

set( target kio_cgi )

kde_add_kpart( ${target} AUTOMOC
  SOURCES cgi.cpp
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

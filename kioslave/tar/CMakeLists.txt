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
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES tar.protocol ar.protocol zip.protocol DESTINATION ${SERVICES_INSTALL_DIR} )


##### kio_tar (module) #######################

set( target kio_tar )

kde_add_kpart( ${target} AUTOMOC
  SOURCES tar.cc
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

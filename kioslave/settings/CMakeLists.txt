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

install( FILES
    settings.protocol programs.protocol applications.protocol
  DESTINATION ${SERVICES_INSTALL_DIR} )


##### kio_settings (module) #####################

set( target kio_settings )

kde_add_kpart( ${target} AUTOMOC
  SOURCES kio_settings.cc
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

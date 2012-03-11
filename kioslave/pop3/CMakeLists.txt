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

install( FILES pop3.protocol pop3s.protocol DESTINATION ${SERVICES_INSTALL_DIR} )


##### kio_pop3 (module) #########################

set( target kio_pop3 )

kde_add_kpart( ${target} AUTOMOC
  SOURCES pop3.cc
  LINK kio-shared ${SASL_LIBRARIES}
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

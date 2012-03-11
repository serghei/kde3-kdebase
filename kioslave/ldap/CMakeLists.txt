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

include( ConfigureChecks.cmake )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES ldap.protocol ldaps.protocol DESTINATION ${SERVICES_INSTALL_DIR} )


##### kio_ldap (module) #########################

kde_add_kpart( kio_ldap AUTOMOC
  SOURCES kio_ldap.cpp
  LINK kabc-shared ${LDAP_LIBRARIES}
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

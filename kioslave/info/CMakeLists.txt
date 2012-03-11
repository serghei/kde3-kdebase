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

install( FILES info.protocol DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kde-info2html.conf DESTINATION ${DATA_INSTALL_DIR}/kio_info )
install( PROGRAMS kde-info2html DESTINATION ${DATA_INSTALL_DIR}/kio_info )


##### kio_info (module) #########################

set( target kio_info )

kde_add_kpart( ${target} AUTOMOC
  SOURCES info.cc
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


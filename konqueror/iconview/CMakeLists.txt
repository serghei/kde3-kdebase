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
  ${CMAKE_SOURCE_DIR}/libkonq
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES konq_iconview.desktop konq_multicolumnview.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES konq_iconview.rc konq_multicolumnview.rc DESTINATION ${DATA_INSTALL_DIR}/konqiconview )


##### konq_iconview (module) ####################

set( target konq_iconview )

kde_add_kpart( ${target} AUTOMOC
  SOURCES konq_iconview.cc
  LINK konq-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

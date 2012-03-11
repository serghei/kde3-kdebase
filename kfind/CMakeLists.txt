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

if( NOT BUILD_LIBKONQ )
  include( "${TDE_CMAKE_DIR}/libkonq.cmake" )
endif( NOT BUILD_LIBKONQ )

add_subdirectory( pics )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/libkonq
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

kde_install_icons( kfind )
install( FILES Kfind.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES kfindpart.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES findpart.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )


##### libkfindpart (module) #####################

kde_add_kpart( libkfindpart AUTOMOC
  SOURCES kfindpart.cpp
  LINK kfind_common-static konq-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kfind (executable) ######################

kde_add_executable( kfind AUTOMOC
  SOURCES kfwin.cpp kfinddlg.cpp main.cpp
  LINK kfind_common-static
  DESTINATION ${BIN_INSTALL_DIR}
)


##### kfind_common (static) #####################

kde_add_library( kfind_common STATIC_PIC AUTOMOC
  SOURCES kftabdlg.cpp kfind.cpp kquery.cpp kdatecombo.cpp
  LINK kparts-shared
)

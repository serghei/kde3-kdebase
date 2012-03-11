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

install( FILES kcmaccess.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES kaccess.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES eventsrc DESTINATION ${DATA_INSTALL_DIR}/kaccess )


##### kcm_access (module) #######################

kde_add_kpart( kcm_access AUTOMOC
  SOURCES kcmaccess.cpp
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kaccess (kdeinit) #########################

kde_add_kdeinit_executable( kaccess AUTOMOC
  SOURCES kaccess.cpp main.cpp
  LINK kdeui-shared
)

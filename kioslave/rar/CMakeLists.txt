#################################################
#
#  (C) 2012 Serghei Amelian
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

install( FILES rar.protocol kfile_rar.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


##### kio_rar (module) ##########################

kde_add_kpart( kio_rar AUTOMOC
  SOURCES kio_rar.cpp krar.cpp
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kfile_rar (module) ########################

kde_add_kpart( kfile_rar AUTOMOC
  SOURCES kfile_rar.cpp krar.cpp
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

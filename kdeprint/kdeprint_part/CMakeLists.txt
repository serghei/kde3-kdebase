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

add_definitions( -D_KDEPRINT_COMPILE )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kdeprint_part.rc DESTINATION ${DATA_INSTALL_DIR}/kdeprint_part )
install( FILES kdeprint_part.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


##### libkdeprint_part (module) #################

kde_add_kpart( libkdeprint_part AUTOMOC
  SOURCES printpart.cpp
  LINK kdeprint_management-shared kparts-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

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

# FIXME not built: man2html, kio_man_test


##### system checks #############################

check_include_file( "unistd.h" HAVE_UNISTD_H )
check_include_file( "string.h" HAVE_STRING_H )


##### compiler/linker settings ##################

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES man.protocol kmanpart.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kio_man.css DESTINATION ${DATA_INSTALL_DIR}/kio_man )


##### kio_man (module) ##########################

set( target kio_man )

kde_add_kpart( ${target} AUTOMOC
  SOURCES man2html.cpp kio_man.cpp
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### libkmanpart (module) ######################

set( target libkmanpart )

kde_add_kpart( ${target} AUTOMOC
  SOURCES kmanpart.cpp
  LINK khtml-shared kparts-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

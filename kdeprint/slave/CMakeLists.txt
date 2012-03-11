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

add_subdirectory( mimetypes )
add_subdirectory( templates )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES print.protocol printdb.protocol DESTINATION ${SERVICES_INSTALL_DIR} )


##### kio_print (module) ########################

kde_add_kpart( kio_print AUTOMOC
  SOURCES kio_print.cpp
  LINK kdeprint-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

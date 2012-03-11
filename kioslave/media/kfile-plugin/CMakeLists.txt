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

check_include_file( "sys/statvfs.h" HAVE_STATVFS )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kioslave/media/libmediacommon
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kfile_media.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


##### kfile_media (module) ######################

set( target kfile_media )

kde_add_kpart( ${target} AUTOMOC
  SOURCES kfilemediaplugin.cpp
  LINK mediacommon-static kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

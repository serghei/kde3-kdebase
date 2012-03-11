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

add_subdirectory( themes )
add_subdirectory( kcs )
add_subdirectory( ad )
add_subdirectory( pics )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kcmdisplayrc.upd DESTINATION ${DATA_INSTALL_DIR}/kconf_update )


##### krdb (static) #############################

kde_add_library( krdb STATIC_PIC AUTOMOC
  SOURCES krdb.cpp
  LINK kdecore-shared
)

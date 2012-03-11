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

add_subdirectory( themer )
add_subdirectory( themes )
add_subdirectory( pics )
add_subdirectory( sessions )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../backend
  ${CMAKE_CURRENT_BINARY_DIR}/..
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kdmlib
  ${CMAKE_SOURCE_DIR}/kcontrol/background
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( CODE "execute_process( COMMAND ${CMAKE_CURRENT_BINARY_DIR}/genkdmconf --in \$ENV{DESTDIR}${CONFIG_INSTALL_DIR}/kdm --no-in-notice --face-src ${CMAKE_CURRENT_SOURCE_DIR}/pics ${GENKDMCONF_FLAGS} )" )


##### kdm_config (executable) ###################

kde_add_executable( kdm_config
  SOURCES kdm_config.c
  DEPENDENCIES config.ci-common
  DESTINATION ${BIN_INSTALL_DIR}
)


##### kdm_greet (executable) ####################

kde_add_executable( kdm_greet AUTOMOC
  SOURCES
    kdm_greet.c kdmconfig.cpp kdmclock.cpp kconsole.cpp
    kfdialog.cpp kgdialog.cpp kchooser.cpp kgverify.cpp
    kdmshutdown.cpp kgreeter.cpp kgapp.cpp
  LINK kdmthemer-static kdeui-shared Xtst
  DESTINATION ${BIN_INSTALL_DIR}
)


##### krootimage (executable) ###################

kde_add_executable( krootimage AUTOMOC
  SOURCES krootimage.cpp
  LINK bgnd-static kio-shared
  DESTINATION ${BIN_INSTALL_DIR}
)


##### genkdmconf (executable) ###################

kde_add_executable( genkdmconf AUTOMOC
  SOURCES genkdmconf.c
  LINK X11
  DEPENDENCIES config.ci-common
  DESTINATION ${BIN_INSTALL_DIR}
)


##### kdmctl (executable) #######################

kde_add_executable( kdmctl
  SOURCES kdmctl.c
  DESTINATION ${BIN_INSTALL_DIR}
)

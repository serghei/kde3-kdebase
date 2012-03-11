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
  ${CMAKE_SOURCE_DIR}/kicker/taskmanager
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kasbarextension.desktop DESTINATION ${DATA_INSTALL_DIR}/kicker/extensions )


##### kasbar (shared) ###########################

set( target kasbar )

set( ${target}_SRCS
  kasbar.cpp kasitem.cpp kastaskitem.cpp kastasker.cpp
  kaspopup.cpp kastaskpopup.cpp kasstartupitem.cpp
  kasaboutdlg.cpp kasprefsdlg.cpp kasclockitem.cpp
  kasgroupitem.cpp kasresources.cpp kasgrouper.cpp
  kasloaditem.cpp
)

kde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 1.0.0
  LINK taskmanager-shared
  DESTINATION ${LIB_INSTALL_DIR}
)


##### kasbar_panelextension (module) ############

kde_add_kpart( kasbar_panelextension AUTOMOC
  SOURCES kasbarextension.cpp
  LINK kasbar-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kasbar (executable) #######################

kde_add_executable( kasbar
  SOURCES kasbarapp.cpp
  LINK kasbar-shared
  DESTINATION ${BIN_INSTALL_DIR}
)

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

add_subdirectory( about )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

kde_install_icons()
install( FILES kcontrolui.rc DESTINATION ${DATA_INSTALL_DIR}/kcontrol )
install( FILES kinfocenterui.rc DESTINATION ${DATA_INSTALL_DIR}/kinfocenter )
install( FILES  KControl.desktop kinfocenter.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### kcontrol (kdeinit) ########################

set( target kcontrol )

set( ${target}_SRCS
  main.cpp toplevel.cpp indexwidget.cpp helpwidget.cpp
  dockcontainer.cpp aboutwidget.cpp moduletreeview.cpp
  moduleiconview.cpp moduleIface.cpp moduleIface.skel
  global.cpp modules.cpp proxywidget.cpp kcrootonly.cpp
  searchwidget.cpp
)

kde_add_kdeinit_executable( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK khtml-shared
)

kde_install_symlink( kcontrol ${BIN_INSTALL_DIR}/kinfocenter )

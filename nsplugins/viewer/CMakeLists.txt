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
  ${CMAKE_SOURCE_DIR}/nsplugins
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
  ${GLIB2_INCLUDE_DIRS}
)


##### nspluginviewer (executable) ###############

kde_add_executable( nspluginviewer AUTOMOC
  SOURCES
    ../NSPluginCallbackIface.stub NSPluginClassIface.skel
    nsplugin.cpp viewer.cpp kxt.cpp qxteventloop.cpp
    glibevents.cpp
  LINK kparts-shared kio-shared ${GLIB2_LIBRARIES} Xt
  DESTINATION ${BIN_INSTALL_DIR}
)

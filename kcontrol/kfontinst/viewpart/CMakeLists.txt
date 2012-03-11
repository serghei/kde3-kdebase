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
  ${CMAKE_SOURCE_DIR}/kcontrol/kfontinst/lib
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kfontviewpart.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kfontview.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES kfontviewpart.rc kfontviewui.rc DESTINATION ${DATA_INSTALL_DIR}/kfontview )


##### libkfontviewpart (module) #################

kde_add_kpart( libkfontviewpart AUTOMOC
  SOURCES FontViewPart.cpp FontViewPartFactory.cpp FontPreview.cpp
  LINK kfontinstprint-static kfontinst-shared kparts-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kfontview (executable) ####################

kde_add_executable( kfontview AUTOMOC
  SOURCES FontViewerApp.cpp
  LINK kparts-shared
  DESTINATION ${BIN_INSTALL_DIR}
)


##### kfontinstprint (static) ###################

kde_add_library( kfontinstprint STATIC_PIC
  SOURCES KfiPrint.cpp
  LINK kdeprint-shared
)

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
  ${CMAKE_SOURCE_DIR}/libkonq
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES
    desktopbehavior.desktop desktop.desktop
    desktoppath.desktop filebrowser.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR} )

install( FILES
    overlapping.png onlyone.png
  DESTINATION ${DATA_INSTALL_DIR}/kcontrol/pics )

install( FILES
    filebehavior.desktop fileappearance.desktop
    filepreviews.desktop
  DESTINATION ${APPS_INSTALL_DIR}/.hidden )


##### kcm_konq (module) #########################

kde_add_kpart( kcm_konq AUTOMOC
  SOURCES
    behaviour.cpp desktop.cpp desktopbehavior_impl.cpp
    previews.cpp browser.cpp main.cpp rootopts.cpp
    fontopts.cpp  desktopbehavior.ui
  LINK kio-shared kutils-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

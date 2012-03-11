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
  ${CMAKE_SOURCE_DIR}/konqueror/sidebar
  ${CMAKE_SOURCE_DIR}/konqueror/sidebar/trees
  ${CMAKE_SOURCE_DIR}/libkonq
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kcmhistory.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### konq_sidebartree_history (module) #########

set( target konq_sidebartree_history )

set( ${target}_SRCS
  history_module.cpp history_item.cpp
  history_settings.cpp history_settings.skel
)

kde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK konq_sidebar_tree-static
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kcm_history (module) ######################

set( target kcm_history )

set( ${target}_SRCS
  kcmhistory.cpp history_dlg.ui history_settings.cpp
  history_settings.skel
)

kde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK konq-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

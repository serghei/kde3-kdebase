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


##### konq_sidebartree_dirtree (module) #########

set( target konq_sidebartree_dirtree )

set( ${target}_SRCS
  dirtree_module.cpp dirtree_item.cpp
)

kde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK konq_sidebar_tree-static
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

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

add_subdirectory( init )
add_subdirectory( dirtree_module )
add_subdirectory( history_module )
add_subdirectory( bookmark_module )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/konqueror/sidebar
  ${CMAKE_SOURCE_DIR}/libkonq
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### konq_sidebar_tree (static lib) ############

set( target konq_sidebar_tree )

set( ${target}_SRCS
  konq_sidebartree.cpp konq_sidebartreeitem.cpp
  konq_sidebartreetoplevelitem.cpp
)

kde_add_library( ${target} STATIC_PIC AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK konqsidebarplugin-shared konq-shared
)


##### konqsidebar_tree (module) #################

set( target konqsidebar_tree )

set( ${target}_SRCS
  konqsidebar_tree.cpp
)

kde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK konq_sidebar_tree-static
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

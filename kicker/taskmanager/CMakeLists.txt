#################################################
#
#  (C) 2010-2012 Serghei Amelian
#  serghei (DOT) amelian (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#################################################

##### check for xcomposite ######################

if( WITH_XCOMPOSITE )

  pkg_search_module( XCOMPOSITE xcomposite )
  if( XCOMPOSITE_FOUND )
    set( HAVE_XCOMPOSITE 1 CACHE INTERNAL "" )
  else( XCOMPOSITE_FOUND )
    message(FATAL_ERROR "\nxcomposite are requested, but not found on your system" )
  endif( XCOMPOSITE_FOUND )

  pkg_search_module( XFIXES xfixes )
  if( XFIXES_FOUND )
    set( HAVE_XFIXES 1 CACHE INTERNAL "" )
  else( XFIXES_FOUND )
    message(FATAL_ERROR "\nxfixes are required, but not found on your system" )
  endif( XFIXES_FOUND )

endif( WITH_XCOMPOSITE )


include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kicker/libkicker
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### taskmanager (shared) ######################

set( target taskmanager )

set( ${target}_SRCS
  tasklmbmenu.cpp taskrmbmenu.cpp taskmanager.cpp
)

kde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 1.0.0
  LINK kdecore-shared kickermain-shared ${XFIXES_LIBRARIES} ${XRENDER_LIBRARIES} ${XCOMPOSITE_LIBRARIES}
  DESTINATION ${LIB_INSTALL_DIR}
)

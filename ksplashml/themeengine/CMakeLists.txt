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

add_subdirectory( default )
add_subdirectory( standard )
add_subdirectory( redmond )
add_subdirectory( simple )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES themeengine.h objkstheme.h DESTINATION ${INCLUDE_INSTALL_DIR}/ksplash )
install( FILES ksplashplugins.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )


##### ksplashthemes (shared) ####################

kde_add_library( ksplashthemes SHARED AUTOMOC
  SOURCES themeengine.cpp objkstheme.cpp
  VERSION 0.0.0
  LINK kdeui-shared
  DESTINATION ${LIB_INSTALL_DIR}
)

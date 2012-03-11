#################################################
#
#  (C) 2010-2012 Serghei Amelian
#  serghei (DOT) amelian (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#################################################

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kickerSettings.kcfg DESTINATION ${KCFG_INSTALL_DIR} )


##### kickermain (shared) #######################

set( target kickermain )

set( ${target}_SRCS
  appletinfo.cpp global.cpp kickertip.cpp menuinfo.cpp
  panelbutton.cpp panner.cpp kickerSettings.kcfgc
  kshadowsettings.cpp kshadowengine.cpp paneldrag.cpp
  simplebutton.cpp
)

kde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 1.0.0
  LINK kio-shared
  DESTINATION ${LIB_INSTALL_DIR}
)

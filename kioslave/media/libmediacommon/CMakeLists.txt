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
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES mediamanagersettings.kcfg DESTINATION ${KCFG_INSTALL_DIR} )


##### mediacommon (static) ######################

set( target mediacommon )

set( ${target}_SRCS
  medium.cpp actionlistboxitem.cpp notifieraction.cpp
  notifierserviceaction.cpp notifiernothingaction.cpp
  notifieropenaction.cpp notifiersettings.cpp
  mediamanagersettings.kcfgc
)

kde_add_library( ${target} STATIC_PIC AUTOMOC
  SOURCES ${${target}_SRCS}
)

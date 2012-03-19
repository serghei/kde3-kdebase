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
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### khotkeys_shared (shared) ##################

set( target khotkeys_shared )

set( ${target}_SRCS
  action_data.cpp khotkeysglobal.cpp triggers.cpp
  windows.cpp settings.cpp actions.cpp input.cpp
  conditions.cpp khlistview.cpp khlistbox.cpp
  gestures.cpp voices.cpp voicesignature.cpp
  sound.cpp soundrecorder.cpp
)

kde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 1.0.0
  LINK kio-shared ${XTEST_LIBRARIES}
  DESTINATION ${LIB_INSTALL_DIR}
)
